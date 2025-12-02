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
 * NDI Node.js Addon - Main Entry Point
 */

#include <napi.h>
#include "../deps/ndi/include/Processing.NDI.Lib.h"
#include "ndi_finder.h"
#include "ndi_sender.h"
#include "ndi_receiver.h"

// Global initialization state
static bool g_ndi_initialized = false;

// Initialize NDI library
Napi::Value Initialize(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_ndi_initialized) {
        return Napi::Boolean::New(env, true);
    }
    
    bool success = NDIlib_initialize();
    g_ndi_initialized = success;
    
    return Napi::Boolean::New(env, success);
}

// Destroy/cleanup NDI library
Napi::Value Destroy(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    if (g_ndi_initialized) {
        NDIlib_destroy();
        g_ndi_initialized = false;
    }
    
    return env.Undefined();
}

// Check if NDI is initialized
Napi::Value IsInitialized(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    return Napi::Boolean::New(env, g_ndi_initialized);
}

// Get NDI library version
Napi::Value Version(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();
    
    const char* version = NDIlib_version();
    if (version) {
        return Napi::String::New(env, version);
    }
    
    return env.Null();
}

// Module initialization
Napi::Object Init(Napi::Env env, Napi::Object exports) {
    // Core functions
    exports.Set("initialize", Napi::Function::New(env, Initialize));
    exports.Set("destroy", Napi::Function::New(env, Destroy));
    exports.Set("isInitialized", Napi::Function::New(env, IsInitialized));
    exports.Set("version", Napi::Function::New(env, Version));
    
    // Initialize class wrappers
    NdiFinder::Init(env, exports);
    NdiSender::Init(env, exports);
    NdiReceiver::Init(env, exports);
    
    // Export constants
    Napi::Object fourCC = Napi::Object::New(env);
    fourCC.Set("UYVY", Napi::String::New(env, "UYVY"));
    fourCC.Set("BGRA", Napi::String::New(env, "BGRA"));
    fourCC.Set("BGRX", Napi::String::New(env, "BGRX"));
    fourCC.Set("RGBA", Napi::String::New(env, "RGBA"));
    fourCC.Set("RGBX", Napi::String::New(env, "RGBX"));
    fourCC.Set("I420", Napi::String::New(env, "I420"));
    fourCC.Set("NV12", Napi::String::New(env, "NV12"));
    fourCC.Set("P216", Napi::String::New(env, "P216"));
    fourCC.Set("PA16", Napi::String::New(env, "PA16"));
    exports.Set("FourCC", fourCC);
    
    Napi::Object frameFormat = Napi::Object::New(env);
    frameFormat.Set("PROGRESSIVE", Napi::String::New(env, "progressive"));
    frameFormat.Set("INTERLEAVED", Napi::String::New(env, "interleaved"));
    frameFormat.Set("FIELD_0", Napi::String::New(env, "field0"));
    frameFormat.Set("FIELD_1", Napi::String::New(env, "field1"));
    exports.Set("FrameFormat", frameFormat);
    
    Napi::Object bandwidth = Napi::Object::New(env);
    bandwidth.Set("METADATA_ONLY", Napi::String::New(env, "metadata_only"));
    bandwidth.Set("AUDIO_ONLY", Napi::String::New(env, "audio_only"));
    bandwidth.Set("LOWEST", Napi::String::New(env, "lowest"));
    bandwidth.Set("HIGHEST", Napi::String::New(env, "highest"));
    exports.Set("Bandwidth", bandwidth);
    
    Napi::Object colorFormat = Napi::Object::New(env);
    colorFormat.Set("BGRX_BGRA", Napi::String::New(env, "BGRX_BGRA"));
    colorFormat.Set("UYVY_BGRA", Napi::String::New(env, "UYVY_BGRA"));
    colorFormat.Set("RGBX_RGBA", Napi::String::New(env, "RGBX_RGBA"));
    colorFormat.Set("UYVY_RGBA", Napi::String::New(env, "UYVY_RGBA"));
    colorFormat.Set("FASTEST", Napi::String::New(env, "fastest"));
    colorFormat.Set("BEST", Napi::String::New(env, "best"));
    exports.Set("ColorFormat", colorFormat);
    
    Napi::Object frameType = Napi::Object::New(env);
    frameType.Set("NONE", Napi::String::New(env, "none"));
    frameType.Set("VIDEO", Napi::String::New(env, "video"));
    frameType.Set("AUDIO", Napi::String::New(env, "audio"));
    frameType.Set("METADATA", Napi::String::New(env, "metadata"));
    frameType.Set("ERROR", Napi::String::New(env, "error"));
    frameType.Set("STATUS_CHANGE", Napi::String::New(env, "status_change"));
    exports.Set("FrameType", frameType);
    
    return exports;
}

NODE_API_MODULE(ndi_addon, Init)
