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
 * NDI Receiver - Implementation
 */

#include "ndi_receiver.h"
#include "ndi_utils.h"
#include "ndi_async.h"
#include <cstring>

Napi::FunctionReference NdiReceiver::constructor;

Napi::Object NdiReceiver::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "NdiReceiver", {
        InstanceMethod("connect", &NdiReceiver::Connect),
        InstanceMethod("capture", &NdiReceiver::Capture),
        InstanceMethod("captureVideo", &NdiReceiver::CaptureVideo),
        InstanceMethod("captureAudio", &NdiReceiver::CaptureAudio),
        InstanceMethod("captureAsync", &NdiReceiver::CaptureAsync),
        InstanceMethod("captureVideoAsync", &NdiReceiver::CaptureVideoAsync),
        InstanceMethod("captureAudioAsync", &NdiReceiver::CaptureAudioAsync),
        InstanceMethod("setTally", &NdiReceiver::SetTally),
        InstanceMethod("sendMetadata", &NdiReceiver::SendMetadata),
        InstanceMethod("ptzIsSupported", &NdiReceiver::PtzIsSupported),
        InstanceMethod("ptzZoom", &NdiReceiver::PtzZoom),
        InstanceMethod("ptzPanTilt", &NdiReceiver::PtzPanTilt),
        InstanceMethod("ptzPanTiltSpeed", &NdiReceiver::PtzPanTiltSpeed),
        InstanceMethod("ptzStorePreset", &NdiReceiver::PtzStorePreset),
        InstanceMethod("ptzRecallPreset", &NdiReceiver::PtzRecallPreset),
        InstanceMethod("ptzAutoFocus", &NdiReceiver::PtzAutoFocus),
        InstanceMethod("ptzFocus", &NdiReceiver::PtzFocus),
        InstanceMethod("ptzFocusSpeed", &NdiReceiver::PtzFocusSpeed),
        InstanceMethod("ptzWhiteBalanceAuto", &NdiReceiver::PtzWhiteBalanceAuto),
        InstanceMethod("ptzWhiteBalanceIndoor", &NdiReceiver::PtzWhiteBalanceIndoor),
        InstanceMethod("ptzWhiteBalanceOutdoor", &NdiReceiver::PtzWhiteBalanceOutdoor),
        InstanceMethod("ptzWhiteBalanceOneshot", &NdiReceiver::PtzWhiteBalanceOneshot),
        InstanceMethod("ptzWhiteBalanceManual", &NdiReceiver::PtzWhiteBalanceManual),
        InstanceMethod("ptzExposureAuto", &NdiReceiver::PtzExposureAuto),
        InstanceMethod("ptzExposureManual", &NdiReceiver::PtzExposureManual),
        InstanceMethod("destroy", &NdiReceiver::Destroy),
        InstanceMethod("isValid", &NdiReceiver::IsValid)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("NdiReceiver", func);
    return exports;
}

NdiReceiver::NdiReceiver(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<NdiReceiver>(info), m_receiver(nullptr), m_destroyed(false) {
    
    Napi::Env env = info.Env();
    
    NDIlib_recv_create_v3_t recv_create = {};
    recv_create.source_to_connect_to.p_ndi_name = nullptr;
    recv_create.source_to_connect_to.p_url_address = nullptr;
    recv_create.color_format = NDIlib_recv_color_format_BGRX_BGRA;
    recv_create.bandwidth = NDIlib_recv_bandwidth_highest;
    recv_create.allow_video_fields = true;
    recv_create.p_ndi_recv_name = nullptr;
    
    std::string sourceName;
    std::string sourceUrl;
    std::string recvName;
    
    // Parse options if provided
    if (info.Length() > 0 && info[0].IsObject()) {
        Napi::Object options = info[0].As<Napi::Object>();
        
        if (options.Has("source") && options.Get("source").IsObject()) {
            Napi::Object sourceObj = options.Get("source").As<Napi::Object>();
            
            if (sourceObj.Has("name") && sourceObj.Get("name").IsString()) {
                sourceName = sourceObj.Get("name").As<Napi::String>().Utf8Value();
                recv_create.source_to_connect_to.p_ndi_name = sourceName.c_str();
            }
            
            if (sourceObj.Has("urlAddress") && sourceObj.Get("urlAddress").IsString()) {
                sourceUrl = sourceObj.Get("urlAddress").As<Napi::String>().Utf8Value();
                recv_create.source_to_connect_to.p_url_address = sourceUrl.c_str();
            }
        }
        
        if (options.Has("colorFormat") && options.Get("colorFormat").IsString()) {
            std::string format = options.Get("colorFormat").As<Napi::String>().Utf8Value();
            recv_create.color_format = NdiUtils::StringToColorFormat(format);
        }
        
        if (options.Has("bandwidth") && options.Get("bandwidth").IsString()) {
            std::string bandwidth = options.Get("bandwidth").As<Napi::String>().Utf8Value();
            recv_create.bandwidth = NdiUtils::StringToBandwidth(bandwidth);
        }
        
        if (options.Has("allowVideoFields") && options.Get("allowVideoFields").IsBoolean()) {
            recv_create.allow_video_fields = options.Get("allowVideoFields").As<Napi::Boolean>().Value();
        }
        
        if (options.Has("name") && options.Get("name").IsString()) {
            recvName = options.Get("name").As<Napi::String>().Utf8Value();
            recv_create.p_ndi_recv_name = recvName.c_str();
        }
    }
    
    m_receiver = NDIlib_recv_create_v3(&recv_create);
    
    if (!m_receiver) {
        Napi::Error::New(env, "Failed to create NDI receiver instance").ThrowAsJavaScriptException();
        return;
    }
}

NdiReceiver::~NdiReceiver() {
    if (m_receiver && !m_destroyed) {
        NDIlib_recv_destroy(m_receiver);
        m_receiver = nullptr;
    }
}

Napi::Value NdiReceiver::Connect(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected source object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object sourceObj = info[0].As<Napi::Object>();
    NDIlib_source_t source = NdiUtils::ObjectToSource(env, sourceObj);
    
    NDIlib_recv_connect(m_receiver, &source);
    
    // Clean up allocated strings
    if (source.p_ndi_name) delete[] source.p_ndi_name;
    if (source.p_url_address) delete[] source.p_url_address;
    
    return env.Undefined();
}

Napi::Value NdiReceiver::Capture(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 1000;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    NDIlib_video_frame_v2_t videoFrame = {};
    NDIlib_audio_frame_v2_t audioFrame = {};
    NDIlib_metadata_frame_t metadataFrame = {};
    
    NDIlib_frame_type_e frameType = NDIlib_recv_capture_v2(
        m_receiver,
        &videoFrame,
        &audioFrame,
        &metadataFrame,
        timeout
    );
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("type", Napi::String::New(env, NdiUtils::FrameTypeToString(frameType)));
    
    switch (frameType) {
        case NDIlib_frame_type_video:
            result.Set("video", NdiUtils::VideoFrameToObject(env, videoFrame));
            NDIlib_recv_free_video_v2(m_receiver, &videoFrame);
            break;
            
        case NDIlib_frame_type_audio:
            result.Set("audio", NdiUtils::AudioFrameToObject(env, audioFrame));
            NDIlib_recv_free_audio_v2(m_receiver, &audioFrame);
            break;
            
        case NDIlib_frame_type_metadata:
            result.Set("metadata", NdiUtils::MetadataFrameToObject(env, metadataFrame));
            NDIlib_recv_free_metadata(m_receiver, &metadataFrame);
            break;
            
        default:
            break;
    }
    
    return result;
}

Napi::Value NdiReceiver::CaptureVideo(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 1000;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    NDIlib_video_frame_v2_t videoFrame = {};
    
    NDIlib_frame_type_e frameType = NDIlib_recv_capture_v2(
        m_receiver,
        &videoFrame,
        nullptr,
        nullptr,
        timeout
    );
    
    if (frameType == NDIlib_frame_type_video) {
        Napi::Object result = NdiUtils::VideoFrameToObject(env, videoFrame);
        NDIlib_recv_free_video_v2(m_receiver, &videoFrame);
        return result;
    }
    
    return env.Null();
}

Napi::Value NdiReceiver::CaptureAudio(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 1000;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    NDIlib_audio_frame_v2_t audioFrame = {};
    
    NDIlib_frame_type_e frameType = NDIlib_recv_capture_v2(
        m_receiver,
        nullptr,
        &audioFrame,
        nullptr,
        timeout
    );
    
    if (frameType == NDIlib_frame_type_audio) {
        Napi::Object result = NdiUtils::AudioFrameToObject(env, audioFrame);
        NDIlib_recv_free_audio_v2(m_receiver, &audioFrame);
        return result;
    }
    
    return env.Null();
}

Napi::Value NdiReceiver::SetTally(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected tally object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object tallyObj = info[0].As<Napi::Object>();
    NDIlib_tally_t tally = NdiUtils::ObjectToTally(env, tallyObj);
    
    bool success = NDIlib_recv_set_tally(m_receiver, &tally);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::SendMetadata(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsObject()) {
        Napi::TypeError::New(env, "Expected metadata frame object").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    Napi::Object frameObj = info[0].As<Napi::Object>();
    char* dataBuffer = nullptr;
    NDIlib_metadata_frame_t frame = NdiUtils::ObjectToMetadataFrame(env, frameObj, &dataBuffer);
    
    NDIlib_recv_send_metadata(m_receiver, &frame);
    
    if (dataBuffer) {
        delete[] dataBuffer;
    }
    
    return env.Undefined();
}

Napi::Value NdiReceiver::PtzIsSupported(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    bool supported = NDIlib_recv_ptz_is_supported(m_receiver);
    return Napi::Boolean::New(env, supported);
}

Napi::Value NdiReceiver::PtzZoom(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected zoom value as number").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    float zoom = info[0].As<Napi::Number>().FloatValue();
    bool success = NDIlib_recv_ptz_zoom(m_receiver, zoom);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzPanTilt(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected pan and tilt values as numbers").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    float pan = info[0].As<Napi::Number>().FloatValue();
    float tilt = info[1].As<Napi::Number>().FloatValue();
    bool success = NDIlib_recv_ptz_pan_tilt(m_receiver, pan, tilt);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzPanTiltSpeed(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected pan and tilt speed values as numbers").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    float panSpeed = info[0].As<Napi::Number>().FloatValue();
    float tiltSpeed = info[1].As<Napi::Number>().FloatValue();
    bool success = NDIlib_recv_ptz_pan_tilt_speed(m_receiver, panSpeed, tiltSpeed);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzStorePreset(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected preset number").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    int presetNo = info[0].As<Napi::Number>().Int32Value();
    bool success = NDIlib_recv_ptz_store_preset(m_receiver, presetNo);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzRecallPreset(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected preset number").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    int presetNo = info[0].As<Napi::Number>().Int32Value();
    float speed = 1.0f;
    
    if (info.Length() > 1 && info[1].IsNumber()) {
        speed = info[1].As<Napi::Number>().FloatValue();
    }
    
    bool success = NDIlib_recv_ptz_recall_preset(m_receiver, presetNo, speed);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzAutoFocus(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    bool success = NDIlib_recv_ptz_auto_focus(m_receiver);
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzFocus(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected focus value as number").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    float focus = info[0].As<Napi::Number>().FloatValue();
    bool success = NDIlib_recv_ptz_focus(m_receiver, focus);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzFocusSpeed(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected focus speed value as number").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    float focusSpeed = info[0].As<Napi::Number>().FloatValue();
    bool success = NDIlib_recv_ptz_focus_speed(m_receiver, focusSpeed);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzWhiteBalanceAuto(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    bool success = NDIlib_recv_ptz_white_balance_auto(m_receiver);
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzWhiteBalanceIndoor(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    bool success = NDIlib_recv_ptz_white_balance_indoor(m_receiver);
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzWhiteBalanceOutdoor(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    bool success = NDIlib_recv_ptz_white_balance_outdoor(m_receiver);
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzWhiteBalanceOneshot(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    bool success = NDIlib_recv_ptz_white_balance_oneshot(m_receiver);
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzWhiteBalanceManual(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 2 || !info[0].IsNumber() || !info[1].IsNumber()) {
        Napi::TypeError::New(env, "Expected red and blue values as numbers").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    float red = info[0].As<Napi::Number>().FloatValue();
    float blue = info[1].As<Napi::Number>().FloatValue();
    bool success = NDIlib_recv_ptz_white_balance_manual(m_receiver, red, blue);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzExposureAuto(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    bool success = NDIlib_recv_ptz_exposure_auto(m_receiver);
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::PtzExposureManual(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    if (info.Length() < 1 || !info[0].IsNumber()) {
        Napi::TypeError::New(env, "Expected exposure level as number").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    float exposure = info[0].As<Napi::Number>().FloatValue();
    bool success = NDIlib_recv_ptz_exposure_manual(m_receiver, exposure);
    
    return Napi::Boolean::New(env, success);
}

Napi::Value NdiReceiver::Destroy(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (m_receiver && !m_destroyed) {
        NDIlib_recv_destroy(m_receiver);
        m_receiver = nullptr;
        m_destroyed = true;
    }
    
    return env.Undefined();
}

Napi::Value NdiReceiver::IsValid(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, m_receiver != nullptr && !m_destroyed);
}

Napi::Value NdiReceiver::CaptureAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 1000;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    CaptureWorker* worker = new CaptureWorker(env, m_receiver, timeout);
    Napi::Promise promise = worker->m_deferred.Promise();
    worker->Queue();
    
    return promise;
}

Napi::Value NdiReceiver::CaptureVideoAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 1000;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    CaptureVideoWorker* worker = new CaptureVideoWorker(env, m_receiver, timeout);
    Napi::Promise promise = worker->m_deferred.Promise();
    worker->Queue();
    
    return promise;
}

Napi::Value NdiReceiver::CaptureAudioAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_receiver || m_destroyed) {
        Napi::Error::New(env, "Receiver has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 1000;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    CaptureAudioWorker* worker = new CaptureAudioWorker(env, m_receiver, timeout);
    Napi::Promise promise = worker->m_deferred.Promise();
    worker->Queue();
    
    return promise;
}
