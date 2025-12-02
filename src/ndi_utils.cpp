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
 * NDI Utils - Implementation of utility functions
 */

#include "ndi_utils.h"
#include <cstring>

namespace NdiUtils {

Napi::Object SourceToObject(Napi::Env env, const NDIlib_source_t& source) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("name", source.p_ndi_name ? Napi::String::New(env, source.p_ndi_name) : env.Null());
    obj.Set("urlAddress", source.p_url_address ? Napi::String::New(env, source.p_url_address) : env.Null());
    return obj;
}

NDIlib_source_t ObjectToSource(Napi::Env env, const Napi::Object& obj) {
    NDIlib_source_t source = {};
    
    if (obj.Has("name") && obj.Get("name").IsString()) {
        std::string name = obj.Get("name").As<Napi::String>().Utf8Value();
        char* nameCopy = new char[name.length() + 1];
        strcpy(nameCopy, name.c_str());
        source.p_ndi_name = nameCopy;
    }
    
    if (obj.Has("urlAddress") && obj.Get("urlAddress").IsString()) {
        std::string url = obj.Get("urlAddress").As<Napi::String>().Utf8Value();
        char* urlCopy = new char[url.length() + 1];
        strcpy(urlCopy, url.c_str());
        source.p_url_address = urlCopy;
    }
    
    return source;
}

Napi::Object VideoFrameToObject(Napi::Env env, const NDIlib_video_frame_v2_t& frame) {
    Napi::Object obj = Napi::Object::New(env);
    
    obj.Set("xres", Napi::Number::New(env, frame.xres));
    obj.Set("yres", Napi::Number::New(env, frame.yres));
    obj.Set("fourCC", Napi::String::New(env, FourCCToString(frame.FourCC)));
    obj.Set("frameRateN", Napi::Number::New(env, frame.frame_rate_N));
    obj.Set("frameRateD", Napi::Number::New(env, frame.frame_rate_D));
    obj.Set("pictureAspectRatio", Napi::Number::New(env, frame.picture_aspect_ratio));
    obj.Set("frameFormatType", Napi::String::New(env, FrameFormatToString(frame.frame_format_type)));
    obj.Set("timecode", Napi::Number::New(env, static_cast<double>(frame.timecode)));
    obj.Set("lineStrideInBytes", Napi::Number::New(env, frame.line_stride_in_bytes));
    obj.Set("timestamp", Napi::Number::New(env, static_cast<double>(frame.timestamp)));
    
    if (frame.p_metadata) {
        obj.Set("metadata", Napi::String::New(env, frame.p_metadata));
    }
    
    // Copy video data to a buffer
    if (frame.p_data && frame.yres > 0 && frame.line_stride_in_bytes > 0) {
        size_t dataSize = static_cast<size_t>(frame.yres) * static_cast<size_t>(frame.line_stride_in_bytes);
        Napi::Buffer<uint8_t> buffer = Napi::Buffer<uint8_t>::Copy(env, frame.p_data, dataSize);
        obj.Set("data", buffer);
    }
    
    return obj;
}

NDIlib_video_frame_v2_t ObjectToVideoFrame(Napi::Env env, const Napi::Object& obj, uint8_t** dataBuffer) {
    NDIlib_video_frame_v2_t frame = {};
    
    if (obj.Has("xres")) frame.xres = obj.Get("xres").As<Napi::Number>().Int32Value();
    if (obj.Has("yres")) frame.yres = obj.Get("yres").As<Napi::Number>().Int32Value();
    
    if (obj.Has("fourCC")) {
        std::string fourcc = obj.Get("fourCC").As<Napi::String>().Utf8Value();
        frame.FourCC = StringToFourCC(fourcc);
    } else {
        frame.FourCC = NDIlib_FourCC_video_type_BGRA;
    }
    
    if (obj.Has("frameRateN")) frame.frame_rate_N = obj.Get("frameRateN").As<Napi::Number>().Int32Value();
    else frame.frame_rate_N = 30000;
    
    if (obj.Has("frameRateD")) frame.frame_rate_D = obj.Get("frameRateD").As<Napi::Number>().Int32Value();
    else frame.frame_rate_D = 1001;
    
    if (obj.Has("pictureAspectRatio")) {
        frame.picture_aspect_ratio = obj.Get("pictureAspectRatio").As<Napi::Number>().FloatValue();
    } else {
        frame.picture_aspect_ratio = static_cast<float>(frame.xres) / static_cast<float>(frame.yres);
    }
    
    if (obj.Has("frameFormatType")) {
        std::string format = obj.Get("frameFormatType").As<Napi::String>().Utf8Value();
        frame.frame_format_type = StringToFrameFormat(format);
    } else {
        frame.frame_format_type = NDIlib_frame_format_type_progressive;
    }
    
    if (obj.Has("timecode")) {
        frame.timecode = static_cast<int64_t>(obj.Get("timecode").As<Napi::Number>().DoubleValue());
    }
    
    if (obj.Has("lineStrideInBytes")) {
        frame.line_stride_in_bytes = obj.Get("lineStrideInBytes").As<Napi::Number>().Int32Value();
    } else {
        // Calculate default stride based on format
        int bytesPerPixel = 4; // Default for BGRA/RGBA
        if (frame.FourCC == NDIlib_FourCC_video_type_UYVY) {
            bytesPerPixel = 2;
        }
        frame.line_stride_in_bytes = frame.xres * bytesPerPixel;
    }
    
    // Handle video data buffer
    if (obj.Has("data") && obj.Get("data").IsBuffer()) {
        Napi::Buffer<uint8_t> buffer = obj.Get("data").As<Napi::Buffer<uint8_t>>();
        size_t dataSize = buffer.Length();
        *dataBuffer = new uint8_t[dataSize];
        memcpy(*dataBuffer, buffer.Data(), dataSize);
        frame.p_data = *dataBuffer;
    }
    
    return frame;
}

Napi::Object AudioFrameToObject(Napi::Env env, const NDIlib_audio_frame_v2_t& frame) {
    Napi::Object obj = Napi::Object::New(env);
    
    obj.Set("sampleRate", Napi::Number::New(env, frame.sample_rate));
    obj.Set("noChannels", Napi::Number::New(env, frame.no_channels));
    obj.Set("noSamples", Napi::Number::New(env, frame.no_samples));
    obj.Set("timecode", Napi::Number::New(env, static_cast<double>(frame.timecode)));
    obj.Set("channelStrideInBytes", Napi::Number::New(env, frame.channel_stride_in_bytes));
    obj.Set("timestamp", Napi::Number::New(env, static_cast<double>(frame.timestamp)));
    
    if (frame.p_metadata) {
        obj.Set("metadata", Napi::String::New(env, frame.p_metadata));
    }
    
    // Copy audio data to a buffer (planar float format)
    if (frame.p_data && frame.no_channels > 0 && frame.no_samples > 0) {
        size_t dataSize = static_cast<size_t>(frame.no_channels) * static_cast<size_t>(frame.channel_stride_in_bytes);
        Napi::Buffer<float> buffer = Napi::Buffer<float>::Copy(env, frame.p_data, dataSize / sizeof(float));
        obj.Set("data", buffer);
    }
    
    return obj;
}

NDIlib_audio_frame_v2_t ObjectToAudioFrame(Napi::Env env, const Napi::Object& obj, float** dataBuffer) {
    NDIlib_audio_frame_v2_t frame = {};
    
    if (obj.Has("sampleRate")) frame.sample_rate = obj.Get("sampleRate").As<Napi::Number>().Int32Value();
    else frame.sample_rate = 48000;
    
    if (obj.Has("noChannels")) frame.no_channels = obj.Get("noChannels").As<Napi::Number>().Int32Value();
    else frame.no_channels = 2;
    
    if (obj.Has("noSamples")) frame.no_samples = obj.Get("noSamples").As<Napi::Number>().Int32Value();
    
    if (obj.Has("timecode")) {
        frame.timecode = static_cast<int64_t>(obj.Get("timecode").As<Napi::Number>().DoubleValue());
    }
    
    if (obj.Has("channelStrideInBytes")) {
        frame.channel_stride_in_bytes = obj.Get("channelStrideInBytes").As<Napi::Number>().Int32Value();
    } else {
        frame.channel_stride_in_bytes = frame.no_samples * sizeof(float);
    }
    
    // Handle audio data buffer
    if (obj.Has("data") && obj.Get("data").IsBuffer()) {
        Napi::Buffer<float> buffer = obj.Get("data").As<Napi::Buffer<float>>();
        size_t dataSize = buffer.Length();
        *dataBuffer = new float[dataSize];
        memcpy(*dataBuffer, buffer.Data(), dataSize * sizeof(float));
        frame.p_data = *dataBuffer;
    }
    
    return frame;
}

Napi::Object MetadataFrameToObject(Napi::Env env, const NDIlib_metadata_frame_t& frame) {
    Napi::Object obj = Napi::Object::New(env);
    
    obj.Set("length", Napi::Number::New(env, frame.length));
    obj.Set("timecode", Napi::Number::New(env, static_cast<double>(frame.timecode)));
    
    if (frame.p_data) {
        obj.Set("data", Napi::String::New(env, frame.p_data));
    }
    
    return obj;
}

NDIlib_metadata_frame_t ObjectToMetadataFrame(Napi::Env env, const Napi::Object& obj, char** dataBuffer) {
    NDIlib_metadata_frame_t frame = {};
    
    if (obj.Has("timecode")) {
        frame.timecode = static_cast<int64_t>(obj.Get("timecode").As<Napi::Number>().DoubleValue());
    }
    
    if (obj.Has("data") && obj.Get("data").IsString()) {
        std::string data = obj.Get("data").As<Napi::String>().Utf8Value();
        *dataBuffer = new char[data.length() + 1];
        strcpy(*dataBuffer, data.c_str());
        frame.p_data = *dataBuffer;
        frame.length = static_cast<int>(data.length());
    }
    
    return frame;
}

Napi::Object TallyToObject(Napi::Env env, const NDIlib_tally_t& tally) {
    Napi::Object obj = Napi::Object::New(env);
    obj.Set("onProgram", Napi::Boolean::New(env, tally.on_program));
    obj.Set("onPreview", Napi::Boolean::New(env, tally.on_preview));
    return obj;
}

NDIlib_tally_t ObjectToTally(Napi::Env env, const Napi::Object& obj) {
    NDIlib_tally_t tally = {};
    
    if (obj.Has("onProgram")) {
        tally.on_program = obj.Get("onProgram").As<Napi::Boolean>().Value();
    }
    
    if (obj.Has("onPreview")) {
        tally.on_preview = obj.Get("onPreview").As<Napi::Boolean>().Value();
    }
    
    return tally;
}

NDIlib_FourCC_video_type_e StringToFourCC(const std::string& str) {
    if (str == "UYVY") return NDIlib_FourCC_video_type_UYVY;
    if (str == "BGRA") return NDIlib_FourCC_video_type_BGRA;
    if (str == "BGRX") return NDIlib_FourCC_video_type_BGRX;
    if (str == "RGBA") return NDIlib_FourCC_video_type_RGBA;
    if (str == "RGBX") return NDIlib_FourCC_video_type_RGBX;
    if (str == "I420") return NDIlib_FourCC_video_type_I420;
    if (str == "NV12") return NDIlib_FourCC_video_type_NV12;
    if (str == "P216") return NDIlib_FourCC_video_type_P216;
    if (str == "PA16") return NDIlib_FourCC_video_type_PA16;
    return NDIlib_FourCC_video_type_BGRA; // Default
}

std::string FourCCToString(NDIlib_FourCC_video_type_e fourcc) {
    switch (fourcc) {
        case NDIlib_FourCC_video_type_UYVY: return "UYVY";
        case NDIlib_FourCC_video_type_BGRA: return "BGRA";
        case NDIlib_FourCC_video_type_BGRX: return "BGRX";
        case NDIlib_FourCC_video_type_RGBA: return "RGBA";
        case NDIlib_FourCC_video_type_RGBX: return "RGBX";
        case NDIlib_FourCC_video_type_I420: return "I420";
        case NDIlib_FourCC_video_type_NV12: return "NV12";
        case NDIlib_FourCC_video_type_P216: return "P216";
        case NDIlib_FourCC_video_type_PA16: return "PA16";
        default: return "UNKNOWN";
    }
}

NDIlib_frame_format_type_e StringToFrameFormat(const std::string& str) {
    if (str == "progressive") return NDIlib_frame_format_type_progressive;
    if (str == "interleaved") return NDIlib_frame_format_type_interleaved;
    if (str == "field0") return NDIlib_frame_format_type_field_0;
    if (str == "field1") return NDIlib_frame_format_type_field_1;
    return NDIlib_frame_format_type_progressive; // Default
}

std::string FrameFormatToString(NDIlib_frame_format_type_e format) {
    switch (format) {
        case NDIlib_frame_format_type_progressive: return "progressive";
        case NDIlib_frame_format_type_interleaved: return "interleaved";
        case NDIlib_frame_format_type_field_0: return "field0";
        case NDIlib_frame_format_type_field_1: return "field1";
        default: return "unknown";
    }
}

NDIlib_recv_bandwidth_e StringToBandwidth(const std::string& str) {
    if (str == "metadata_only") return NDIlib_recv_bandwidth_metadata_only;
    if (str == "audio_only") return NDIlib_recv_bandwidth_audio_only;
    if (str == "lowest") return NDIlib_recv_bandwidth_lowest;
    if (str == "highest") return NDIlib_recv_bandwidth_highest;
    return NDIlib_recv_bandwidth_highest; // Default
}

std::string BandwidthToString(NDIlib_recv_bandwidth_e bandwidth) {
    switch (bandwidth) {
        case NDIlib_recv_bandwidth_metadata_only: return "metadata_only";
        case NDIlib_recv_bandwidth_audio_only: return "audio_only";
        case NDIlib_recv_bandwidth_lowest: return "lowest";
        case NDIlib_recv_bandwidth_highest: return "highest";
        default: return "unknown";
    }
}

NDIlib_recv_color_format_e StringToColorFormat(const std::string& str) {
    if (str == "BGRX_BGRA") return NDIlib_recv_color_format_BGRX_BGRA;
    if (str == "UYVY_BGRA") return NDIlib_recv_color_format_UYVY_BGRA;
    if (str == "RGBX_RGBA") return NDIlib_recv_color_format_RGBX_RGBA;
    if (str == "UYVY_RGBA") return NDIlib_recv_color_format_UYVY_RGBA;
    if (str == "fastest") return NDIlib_recv_color_format_fastest;
    if (str == "best") return NDIlib_recv_color_format_best;
    return NDIlib_recv_color_format_BGRX_BGRA; // Default
}

std::string ColorFormatToString(NDIlib_recv_color_format_e format) {
    switch (format) {
        case NDIlib_recv_color_format_BGRX_BGRA: return "BGRX_BGRA";
        case NDIlib_recv_color_format_UYVY_BGRA: return "UYVY_BGRA";
        case NDIlib_recv_color_format_RGBX_RGBA: return "RGBX_RGBA";
        case NDIlib_recv_color_format_UYVY_RGBA: return "UYVY_RGBA";
        case NDIlib_recv_color_format_fastest: return "fastest";
        case NDIlib_recv_color_format_best: return "best";
        default: return "unknown";
    }
}

std::string FrameTypeToString(NDIlib_frame_type_e type) {
    switch (type) {
        case NDIlib_frame_type_none: return "none";
        case NDIlib_frame_type_video: return "video";
        case NDIlib_frame_type_audio: return "audio";
        case NDIlib_frame_type_metadata: return "metadata";
        case NDIlib_frame_type_error: return "error";
        case NDIlib_frame_type_status_change: return "status_change";
        default: return "unknown";
    }
}

} // namespace NdiUtils
