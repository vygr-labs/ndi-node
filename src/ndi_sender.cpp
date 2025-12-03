/*
 * ndi-node - Node.js bindings for NDI (Network Device Interface)
 * Copyright (C) 2025 Eyetu Kingsley Oghenekome - Technical Director, Voyager Technologies
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * NDI Sender - Implementation
 */

#include "ndi_sender.h"
#include "ndi_utils.h"
#include "ndi_async.h"
#include <cstring>

Napi::FunctionReference NdiSender::constructor;

Napi::Object NdiSender::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "NdiSender", {
        InstanceMethod("sendVideo", &NdiSender::SendVideo),
        InstanceMethod("sendVideoAsync", &NdiSender::SendVideoAsync),
        InstanceMethod("sendVideoPromise", &NdiSender::SendVideoPromise),
        InstanceMethod("sendAudio", &NdiSender::SendAudio),
        InstanceMethod("sendAudioPromise", &NdiSender::SendAudioPromise),
        InstanceMethod("sendMetadata", &NdiSender::SendMetadata),
        InstanceMethod("getTally", &NdiSender::GetTally),
        InstanceMethod("getTallyAsync", &NdiSender::GetTallyAsync),
        InstanceMethod("setTally", &NdiSender::SetTally),
        InstanceMethod("getConnections", &NdiSender::GetConnections),
        InstanceMethod("getConnectionsAsync", &NdiSender::GetConnectionsAsync),
        InstanceMethod("getSourceName", &NdiSender::GetSourceName),
        InstanceMethod("clearConnectionMetadata", &NdiSender::ClearConnectionMetadata),
        InstanceMethod("addConnectionMetadata", &NdiSender::AddConnectionMetadata),
        InstanceMethod("destroy", &NdiSender::Destroy),
        InstanceMethod("isValid", &NdiSender::IsValid)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("NdiSender", func);
    return exports;
}

NdiSender::NdiSender(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<NdiSender>(info), m_sender(nullptr), m_destroyed(false), m_asyncVideoBuffer(nullptr) {
    
    Napi::Env env = info.Env();
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected options object as first argument").ThrowAsJavaScriptException();
        return;
    }
    
    Napi::Object options = info[0].As<Napi::Object>();
    
    NDIlib_send_create_t send_create = {};
    send_create.clock_video = true;
    send_create.clock_audio = true;
    
    std::string ndiName;
    std::string groups;
    
    if (options.Has("name") && options.Get("name").IsString()) {
        ndiName = options.Get("name").As<Napi::String>().Utf8Value();
        send_create.p_ndi_name = ndiName.c_str();
    } else {
        Napi::TypeError::New(env, "name is required in options").ThrowAsJavaScriptException();
        return;
    }
    
    if (options.Has("groups") && options.Get("groups").IsString()) {
        groups = options.Get("groups").As<Napi::String>().Utf8Value();
        send_create.p_groups = groups.c_str();
    }
    
    if (options.Has("clockVideo") && options.Get("clockVideo").IsBoolean()) {
        send_create.clock_video = options.Get("clockVideo").As<Napi::Boolean>().Value();
    }
    
    if (options.Has("clockAudio") && options.Get("clockAudio").IsBoolean()) {
        send_create.clock_audio = options.Get("clockAudio").As<Napi::Boolean>().Value();
    }
    
    m_sender = NDIlib_send_create(&send_create);
    
    if (!m_sender) {
        Napi::Error::New(env, "Failed to create NDI sender instance").ThrowAsJavaScriptException();
        return;
    }
}

NdiSender::~NdiSender() {
    if (m_asyncVideoBuffer) {
        delete[] m_asyncVideoBuffer;
        m_asyncVideoBuffer = nullptr;
    }
    
    if (m_sender && !m_destroyed) {
        NDIlib_send_destroy(m_sender);
        m_sender = nullptr;
    }
}

Napi::Value NdiSender::SendVideo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected video frame object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object frameObj = info[0].As<Napi::Object>();
    uint8_t* dataBuffer = nullptr;
    NDIlib_video_frame_v2_t frame = NdiUtils::ObjectToVideoFrame(env, frameObj, &dataBuffer);
    
    NDIlib_send_send_video_v2(m_sender, &frame);
    
    if (dataBuffer) {
        delete[] dataBuffer;
    }
    
    return env.Undefined();
}

Napi::Value NdiSender::SendVideoAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected video frame object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    // Clean up previous async buffer
    if (m_asyncVideoBuffer) {
        // Wait for previous async send to complete by sending nullptr
        NDIlib_send_send_video_async_v2(m_sender, nullptr);
        delete[] m_asyncVideoBuffer;
        m_asyncVideoBuffer = nullptr;
    }
    
    Napi::Object frameObj = info[0].As<Napi::Object>();
    NDIlib_video_frame_v2_t frame = NdiUtils::ObjectToVideoFrame(env, frameObj, &m_asyncVideoBuffer);
    
    NDIlib_send_send_video_async_v2(m_sender, &frame);
    
    return env.Undefined();
}

Napi::Value NdiSender::SendAudio(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected audio frame object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object frameObj = info[0].As<Napi::Object>();
    float* dataBuffer = nullptr;
    NDIlib_audio_frame_v2_t frame = NdiUtils::ObjectToAudioFrame(env, frameObj, &dataBuffer);
    
    NDIlib_send_send_audio_v2(m_sender, &frame);
    
    if (dataBuffer) {
        delete[] dataBuffer;
    }
    
    return env.Undefined();
}

Napi::Value NdiSender::SendMetadata(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected metadata frame object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object frameObj = info[0].As<Napi::Object>();
    char* dataBuffer = nullptr;
    NDIlib_metadata_frame_t frame = NdiUtils::ObjectToMetadataFrame(env, frameObj, &dataBuffer);
    
    NDIlib_send_send_metadata(m_sender, &frame);
    
    if (dataBuffer) {
        delete[] dataBuffer;
    }
    
    return env.Undefined();
}

Napi::Value NdiSender::GetTally(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 0;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    NDIlib_tally_t tally = {};
    bool success = NDIlib_send_get_tally(m_sender, &tally, timeout);
    
    if (!success) {
        return env.Null();
    }
    
    return NdiUtils::TallyToObject(env, tally);
}

Napi::Value NdiSender::SetTally(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    // Note: NDI senders receive tally from receivers, they don't set it.
    // This function is kept for API compatibility but does nothing.
    // Use getTally() to read the tally state set by receivers.
    
    return env.Undefined();
}

Napi::Value NdiSender::GetConnections(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 0;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    int numConnections = NDIlib_send_get_no_connections(m_sender, timeout);
    
    return Napi::Number::New(env, numConnections);
}

Napi::Value NdiSender::GetSourceName(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    const NDIlib_source_t* source = NDIlib_send_get_source_name(m_sender);
    
    if (source && source->p_ndi_name) {
        return Napi::String::New(env, source->p_ndi_name);
    }
    
    return env.Null();
}

Napi::Value NdiSender::ClearConnectionMetadata(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    NDIlib_send_clear_connection_metadata(m_sender);
    
    return env.Undefined();
}

Napi::Value NdiSender::AddConnectionMetadata(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected metadata frame object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object frameObj = info[0].As<Napi::Object>();
    char* dataBuffer = nullptr;
    NDIlib_metadata_frame_t frame = NdiUtils::ObjectToMetadataFrame(env, frameObj, &dataBuffer);
    
    NDIlib_send_add_connection_metadata(m_sender, &frame);
    
    if (dataBuffer) {
        delete[] dataBuffer;
    }
    
    return env.Undefined();
}

Napi::Value NdiSender::Destroy(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (m_asyncVideoBuffer) {
        // Wait for async send to complete
        if (m_sender) {
            NDIlib_send_send_video_async_v2(m_sender, nullptr);
        }
        delete[] m_asyncVideoBuffer;
        m_asyncVideoBuffer = nullptr;
    }
    
    if (m_sender && !m_destroyed) {
        NDIlib_send_destroy(m_sender);
        m_sender = nullptr;
        m_destroyed = true;
    }
    
    return env.Undefined();
}

Napi::Value NdiSender::IsValid(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, m_sender != nullptr && !m_destroyed);
}

Napi::Value NdiSender::SendVideoPromise(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected video frame object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object frameObj = info[0].As<Napi::Object>();
    uint8_t* dataBuffer = nullptr;
    NDIlib_video_frame_v2_t frame = NdiUtils::ObjectToVideoFrame(env, frameObj, &dataBuffer);
    
    SendVideoWorker* worker = new SendVideoWorker(env, m_sender, frame, dataBuffer);
    Napi::Promise promise = worker->m_deferred.Promise();
    worker->Queue();
    
    return promise;
}

Napi::Value NdiSender::SendAudioPromise(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected audio frame object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object frameObj = info[0].As<Napi::Object>();
    float* dataBuffer = nullptr;
    NDIlib_audio_frame_v2_t frame = NdiUtils::ObjectToAudioFrame(env, frameObj, &dataBuffer);
    
    SendAudioWorker* worker = new SendAudioWorker(env, m_sender, frame, dataBuffer);
    Napi::Promise promise = worker->m_deferred.Promise();
    worker->Queue();
    
    return promise;
}

Napi::Value NdiSender::GetTallyAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 0;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    GetTallyWorker* worker = new GetTallyWorker(env, m_sender, timeout);
    Napi::Promise promise = worker->m_deferred.Promise();
    worker->Queue();
    
    return promise;
}

Napi::Value NdiSender::GetConnectionsAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_sender || m_destroyed) {
        Napi::Error::New(env, "Sender has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 0;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    GetConnectionsWorker* worker = new GetConnectionsWorker(env, m_sender, timeout);
    Napi::Promise promise = worker->m_deferred.Promise();
    worker->Queue();
    
    return promise;
}
