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
 * NDI Finder - Implementation
 */

#include "ndi_finder.h"
#include "ndi_utils.h"
#include "ndi_async.h"

Napi::FunctionReference NdiFinder::constructor;

Napi::Object NdiFinder::Init(Napi::Env env, Napi::Object exports) {
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(env, "NdiFinder", {
        InstanceMethod("getSources", &NdiFinder::GetSources),
        InstanceMethod("waitForSources", &NdiFinder::WaitForSources),
        InstanceMethod("getSourcesAsync", &NdiFinder::GetSourcesAsync),
        InstanceMethod("waitForSourcesAsync", &NdiFinder::WaitForSourcesAsync),
        InstanceMethod("destroy", &NdiFinder::Destroy),
        InstanceMethod("isValid", &NdiFinder::IsValid)
    });

    constructor = Napi::Persistent(func);
    constructor.SuppressDestruct();

    exports.Set("NdiFinder", func);
    return exports;
}

NdiFinder::NdiFinder(const Napi::CallbackInfo& info) 
    : Napi::ObjectWrap<NdiFinder>(info), m_finder(nullptr), m_destroyed(false) {
    
    Napi::Env env = info.Env();
    
    NDIlib_find_create_t find_create = {};
    find_create.show_local_sources = true;
    find_create.p_groups = nullptr;
    find_create.p_extra_ips = nullptr;
    
    // Parse options if provided
    if (info.Length() > 0 && info[0].IsObject()) {
        Napi::Object options = info[0].As<Napi::Object>();
        
        if (options.Has("showLocalSources") && options.Get("showLocalSources").IsBoolean()) {
            find_create.show_local_sources = options.Get("showLocalSources").As<Napi::Boolean>().Value();
        }
        
        if (options.Has("groups") && options.Get("groups").IsString()) {
            std::string groups = options.Get("groups").As<Napi::String>().Utf8Value();
            char* groupsCopy = new char[groups.length() + 1];
            strcpy(groupsCopy, groups.c_str());
            find_create.p_groups = groupsCopy;
        }
        
        if (options.Has("extraIps") && options.Get("extraIps").IsString()) {
            std::string extraIps = options.Get("extraIps").As<Napi::String>().Utf8Value();
            char* extraIpsCopy = new char[extraIps.length() + 1];
            strcpy(extraIpsCopy, extraIps.c_str());
            find_create.p_extra_ips = extraIpsCopy;
        }
    }
    
    m_finder = NDIlib_find_create_v2(&find_create);
    
    // Clean up allocated strings
    if (find_create.p_groups) delete[] find_create.p_groups;
    if (find_create.p_extra_ips) delete[] find_create.p_extra_ips;
    
    if (!m_finder) {
        Napi::Error::New(env, "Failed to create NDI finder instance").ThrowAsJavaScriptException();
        return;
    }
}

NdiFinder::~NdiFinder() {
    if (m_finder && !m_destroyed) {
        NDIlib_find_destroy(m_finder);
        m_finder = nullptr;
    }
}

Napi::Value NdiFinder::GetSources(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_finder || m_destroyed) {
        Napi::Error::New(env, "Finder has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t numSources = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(m_finder, &numSources);
    
    Napi::Array result = Napi::Array::New(env, numSources);
    
    for (uint32_t i = 0; i < numSources; i++) {
        result.Set(i, NdiUtils::SourceToObject(env, sources[i]));
    }
    
    return result;
}

Napi::Value NdiFinder::WaitForSources(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_finder || m_destroyed) {
        Napi::Error::New(env, "Finder has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 1000; // Default 1 second
    
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    bool changed = NDIlib_find_wait_for_sources(m_finder, timeout);
    
    return Napi::Boolean::New(env, changed);
}

Napi::Value NdiFinder::Destroy(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (m_finder && !m_destroyed) {
        NDIlib_find_destroy(m_finder);
        m_finder = nullptr;
        m_destroyed = true;
    }
    
    return env.Undefined();
}

Napi::Value NdiFinder::IsValid(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, m_finder != nullptr && !m_destroyed);
}

Napi::Value NdiFinder::GetSourcesAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_finder || m_destroyed) {
        Napi::Error::New(env, "Finder has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    GetSourcesWorker* worker = new GetSourcesWorker(env, m_finder);
    Napi::Promise promise = worker->m_deferred.Promise();
    worker->Queue();
    
    return promise;
}

Napi::Value NdiFinder::WaitForSourcesAsync(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (!m_finder || m_destroyed) {
        Napi::Error::New(env, "Finder has been destroyed").ThrowAsJavaScriptException();
        return env.Null();
    }
    
    uint32_t timeout = 1000;
    if (info.Length() > 0 && info[0].IsNumber()) {
        timeout = info[0].As<Napi::Number>().Uint32Value();
    }
    
    WaitForSourcesWorker* worker = new WaitForSourcesWorker(env, m_finder, timeout);
    Napi::Promise promise = worker->m_deferred.Promise();
    worker->Queue();
    
    return promise;
}
