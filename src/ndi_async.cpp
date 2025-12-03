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
 * NDI Async Workers - Implementation
 */

#include "ndi_async.h"
#include "ndi_utils.h"
#include <cstring>

// ============================================================================
// Finder Async Workers
// ============================================================================

WaitForSourcesWorker::WaitForSourcesWorker(
    Napi::Env env,
    NDIlib_find_instance_t finder,
    uint32_t timeout
) : Napi::AsyncWorker(env),
    m_finder(finder),
    m_timeout(timeout),
    m_changed(false),
    m_numSources(0),
    m_deferred(Napi::Promise::Deferred::New(env))
{
}

void WaitForSourcesWorker::Execute() {
    // Wait for sources on background thread
    m_changed = NDIlib_find_wait_for_sources(m_finder, m_timeout);
    
    // Get current sources
    uint32_t numSources = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(m_finder, &numSources);
    m_numSources = numSources;
    
    // Copy source data to thread-safe storage
    for (uint32_t i = 0; i < numSources; i++) {
        std::string name = sources[i].p_ndi_name ? sources[i].p_ndi_name : "";
        std::string url = sources[i].p_url_address ? sources[i].p_url_address : "";
        m_sources.push_back(std::make_pair(name, url));
    }
}

void WaitForSourcesWorker::OnOK() {
    Napi::Env env = Env();
    Napi::HandleScope scope(env);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("changed", Napi::Boolean::New(env, m_changed));
    
    Napi::Array sourcesArray = Napi::Array::New(env, m_sources.size());
    for (size_t i = 0; i < m_sources.size(); i++) {
        Napi::Object sourceObj = Napi::Object::New(env);
        sourceObj.Set("name", Napi::String::New(env, m_sources[i].first));
        sourceObj.Set("urlAddress", Napi::String::New(env, m_sources[i].second));
        sourcesArray.Set(i, sourceObj);
    }
    result.Set("sources", sourcesArray);
    
    m_deferred.Resolve(result);
}

GetSourcesWorker::GetSourcesWorker(
    Napi::Env env,
    NDIlib_find_instance_t finder
) : Napi::AsyncWorker(env),
    m_finder(finder),
    m_deferred(Napi::Promise::Deferred::New(env))
{
}

void GetSourcesWorker::Execute() {
    uint32_t numSources = 0;
    const NDIlib_source_t* sources = NDIlib_find_get_current_sources(m_finder, &numSources);
    
    for (uint32_t i = 0; i < numSources; i++) {
        std::string name = sources[i].p_ndi_name ? sources[i].p_ndi_name : "";
        std::string url = sources[i].p_url_address ? sources[i].p_url_address : "";
        m_sources.push_back(std::make_pair(name, url));
    }
}

void GetSourcesWorker::OnOK() {
    Napi::Env env = Env();
    Napi::HandleScope scope(env);
    
    Napi::Array sourcesArray = Napi::Array::New(env, m_sources.size());
    for (size_t i = 0; i < m_sources.size(); i++) {
        Napi::Object sourceObj = Napi::Object::New(env);
        sourceObj.Set("name", Napi::String::New(env, m_sources[i].first));
        sourceObj.Set("urlAddress", Napi::String::New(env, m_sources[i].second));
        sourcesArray.Set(i, sourceObj);
    }
    
    m_deferred.Resolve(sourcesArray);
}

// ============================================================================
// Receiver Async Workers
// ============================================================================

CaptureVideoWorker::CaptureVideoWorker(
    Napi::Env env,
    NDIlib_recv_instance_t receiver,
    uint32_t timeout
) : Napi::AsyncWorker(env),
    m_receiver(receiver),
    m_timeout(timeout),
    m_deferred(Napi::Promise::Deferred::New(env))
{
    m_frame.valid = false;
}

void CaptureVideoWorker::Execute() {
    NDIlib_video_frame_v2_t videoFrame = {};
    
    NDIlib_frame_type_e frameType = NDIlib_recv_capture_v2(
        m_receiver,
        &videoFrame,
        nullptr,
        nullptr,
        m_timeout
    );
    
    if (frameType == NDIlib_frame_type_video) {
        m_frame.valid = true;
        m_frame.xres = videoFrame.xres;
        m_frame.yres = videoFrame.yres;
        m_frame.fourCC = NdiUtils::FourCCToString(videoFrame.FourCC);
        m_frame.frameRateN = videoFrame.frame_rate_N;
        m_frame.frameRateD = videoFrame.frame_rate_D;
        m_frame.pictureAspectRatio = videoFrame.picture_aspect_ratio;
        m_frame.frameFormat = NdiUtils::FrameFormatToString(videoFrame.frame_format_type);
        m_frame.timecode = videoFrame.timecode;
        m_frame.lineStride = videoFrame.line_stride_in_bytes;
        m_frame.timestamp = videoFrame.timestamp;
        
        if (videoFrame.p_metadata) {
            m_frame.metadata = videoFrame.p_metadata;
        }
        
        // Copy frame data
        if (videoFrame.p_data && videoFrame.line_stride_in_bytes > 0) {
            size_t dataSize = static_cast<size_t>(videoFrame.line_stride_in_bytes) * videoFrame.yres;
            m_frame.data.resize(dataSize);
            memcpy(m_frame.data.data(), videoFrame.p_data, dataSize);
        }
        
        NDIlib_recv_free_video_v2(m_receiver, &videoFrame);
    }
}

void CaptureVideoWorker::OnOK() {
    Napi::Env env = Env();
    Napi::HandleScope scope(env);
    
    if (!m_frame.valid) {
        m_deferred.Resolve(env.Null());
        return;
    }
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("xres", Napi::Number::New(env, m_frame.xres));
    result.Set("yres", Napi::Number::New(env, m_frame.yres));
    result.Set("fourCC", Napi::String::New(env, m_frame.fourCC));
    result.Set("frameRateN", Napi::Number::New(env, m_frame.frameRateN));
    result.Set("frameRateD", Napi::Number::New(env, m_frame.frameRateD));
    result.Set("pictureAspectRatio", Napi::Number::New(env, m_frame.pictureAspectRatio));
    result.Set("frameFormat", Napi::String::New(env, m_frame.frameFormat));
    result.Set("timecode", Napi::Number::New(env, static_cast<double>(m_frame.timecode)));
    result.Set("lineStride", Napi::Number::New(env, m_frame.lineStride));
    result.Set("timestamp", Napi::Number::New(env, static_cast<double>(m_frame.timestamp)));
    
    if (!m_frame.metadata.empty()) {
        result.Set("metadata", Napi::String::New(env, m_frame.metadata));
    }
    
    if (!m_frame.data.empty()) {
        Napi::Buffer<uint8_t> dataBuffer = Napi::Buffer<uint8_t>::Copy(
            env, m_frame.data.data(), m_frame.data.size()
        );
        result.Set("data", dataBuffer);
    }
    
    m_deferred.Resolve(result);
}

CaptureAudioWorker::CaptureAudioWorker(
    Napi::Env env,
    NDIlib_recv_instance_t receiver,
    uint32_t timeout
) : Napi::AsyncWorker(env),
    m_receiver(receiver),
    m_timeout(timeout),
    m_deferred(Napi::Promise::Deferred::New(env))
{
    m_frame.valid = false;
}

void CaptureAudioWorker::Execute() {
    NDIlib_audio_frame_v2_t audioFrame = {};
    
    NDIlib_frame_type_e frameType = NDIlib_recv_capture_v2(
        m_receiver,
        nullptr,
        &audioFrame,
        nullptr,
        m_timeout
    );
    
    if (frameType == NDIlib_frame_type_audio) {
        m_frame.valid = true;
        m_frame.sampleRate = audioFrame.sample_rate;
        m_frame.noChannels = audioFrame.no_channels;
        m_frame.noSamples = audioFrame.no_samples;
        m_frame.timecode = audioFrame.timecode;
        m_frame.channelStride = audioFrame.channel_stride_in_bytes;
        m_frame.timestamp = audioFrame.timestamp;
        
        if (audioFrame.p_metadata) {
            m_frame.metadata = audioFrame.p_metadata;
        }
        
        // Copy audio data
        if (audioFrame.p_data && audioFrame.no_samples > 0 && audioFrame.no_channels > 0) {
            size_t dataSize = static_cast<size_t>(audioFrame.no_samples) * audioFrame.no_channels;
            m_frame.data.resize(dataSize);
            memcpy(m_frame.data.data(), audioFrame.p_data, dataSize * sizeof(float));
        }
        
        NDIlib_recv_free_audio_v2(m_receiver, &audioFrame);
    }
}

void CaptureAudioWorker::OnOK() {
    Napi::Env env = Env();
    Napi::HandleScope scope(env);
    
    if (!m_frame.valid) {
        m_deferred.Resolve(env.Null());
        return;
    }
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("sampleRate", Napi::Number::New(env, m_frame.sampleRate));
    result.Set("noChannels", Napi::Number::New(env, m_frame.noChannels));
    result.Set("noSamples", Napi::Number::New(env, m_frame.noSamples));
    result.Set("timecode", Napi::Number::New(env, static_cast<double>(m_frame.timecode)));
    result.Set("channelStride", Napi::Number::New(env, m_frame.channelStride));
    result.Set("timestamp", Napi::Number::New(env, static_cast<double>(m_frame.timestamp)));
    
    if (!m_frame.metadata.empty()) {
        result.Set("metadata", Napi::String::New(env, m_frame.metadata));
    }
    
    if (!m_frame.data.empty()) {
        Napi::Float32Array dataArray = Napi::Float32Array::New(env, m_frame.data.size());
        memcpy(dataArray.Data(), m_frame.data.data(), m_frame.data.size() * sizeof(float));
        result.Set("data", dataArray);
    }
    
    m_deferred.Resolve(result);
}

CaptureWorker::CaptureWorker(
    Napi::Env env,
    NDIlib_recv_instance_t receiver,
    uint32_t timeout
) : Napi::AsyncWorker(env),
    m_receiver(receiver),
    m_timeout(timeout),
    m_frameType(NDIlib_frame_type_none),
    m_deferred(Napi::Promise::Deferred::New(env))
{
    m_videoFrame.valid = false;
    m_audioFrame.valid = false;
    m_metadataFrame.valid = false;
}

void CaptureWorker::Execute() {
    NDIlib_video_frame_v2_t videoFrame = {};
    NDIlib_audio_frame_v2_t audioFrame = {};
    NDIlib_metadata_frame_t metadataFrame = {};
    
    m_frameType = NDIlib_recv_capture_v2(
        m_receiver,
        &videoFrame,
        &audioFrame,
        &metadataFrame,
        m_timeout
    );
    
    switch (m_frameType) {
        case NDIlib_frame_type_video:
            m_videoFrame.valid = true;
            m_videoFrame.xres = videoFrame.xres;
            m_videoFrame.yres = videoFrame.yres;
            m_videoFrame.fourCC = NdiUtils::FourCCToString(videoFrame.FourCC);
            m_videoFrame.frameRateN = videoFrame.frame_rate_N;
            m_videoFrame.frameRateD = videoFrame.frame_rate_D;
            m_videoFrame.pictureAspectRatio = videoFrame.picture_aspect_ratio;
            m_videoFrame.frameFormat = NdiUtils::FrameFormatToString(videoFrame.frame_format_type);
            m_videoFrame.timecode = videoFrame.timecode;
            m_videoFrame.lineStride = videoFrame.line_stride_in_bytes;
            m_videoFrame.timestamp = videoFrame.timestamp;
            
            if (videoFrame.p_metadata) {
                m_videoFrame.metadata = videoFrame.p_metadata;
            }
            
            if (videoFrame.p_data && videoFrame.line_stride_in_bytes > 0) {
                size_t dataSize = static_cast<size_t>(videoFrame.line_stride_in_bytes) * videoFrame.yres;
                m_videoFrame.data.resize(dataSize);
                memcpy(m_videoFrame.data.data(), videoFrame.p_data, dataSize);
            }
            
            NDIlib_recv_free_video_v2(m_receiver, &videoFrame);
            break;
            
        case NDIlib_frame_type_audio:
            m_audioFrame.valid = true;
            m_audioFrame.sampleRate = audioFrame.sample_rate;
            m_audioFrame.noChannels = audioFrame.no_channels;
            m_audioFrame.noSamples = audioFrame.no_samples;
            m_audioFrame.timecode = audioFrame.timecode;
            m_audioFrame.channelStride = audioFrame.channel_stride_in_bytes;
            m_audioFrame.timestamp = audioFrame.timestamp;
            
            if (audioFrame.p_metadata) {
                m_audioFrame.metadata = audioFrame.p_metadata;
            }
            
            if (audioFrame.p_data && audioFrame.no_samples > 0 && audioFrame.no_channels > 0) {
                size_t dataSize = static_cast<size_t>(audioFrame.no_samples) * audioFrame.no_channels;
                m_audioFrame.data.resize(dataSize);
                memcpy(m_audioFrame.data.data(), audioFrame.p_data, dataSize * sizeof(float));
            }
            
            NDIlib_recv_free_audio_v2(m_receiver, &audioFrame);
            break;
            
        case NDIlib_frame_type_metadata:
            m_metadataFrame.valid = true;
            m_metadataFrame.timecode = metadataFrame.timecode;
            if (metadataFrame.p_data) {
                m_metadataFrame.data = metadataFrame.p_data;
            }
            NDIlib_recv_free_metadata(m_receiver, &metadataFrame);
            break;
            
        default:
            break;
    }
}

void CaptureWorker::OnOK() {
    Napi::Env env = Env();
    Napi::HandleScope scope(env);
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("type", Napi::String::New(env, NdiUtils::FrameTypeToString(m_frameType)));
    
    if (m_videoFrame.valid) {
        Napi::Object videoObj = Napi::Object::New(env);
        videoObj.Set("xres", Napi::Number::New(env, m_videoFrame.xres));
        videoObj.Set("yres", Napi::Number::New(env, m_videoFrame.yres));
        videoObj.Set("fourCC", Napi::String::New(env, m_videoFrame.fourCC));
        videoObj.Set("frameRateN", Napi::Number::New(env, m_videoFrame.frameRateN));
        videoObj.Set("frameRateD", Napi::Number::New(env, m_videoFrame.frameRateD));
        videoObj.Set("pictureAspectRatio", Napi::Number::New(env, m_videoFrame.pictureAspectRatio));
        videoObj.Set("frameFormat", Napi::String::New(env, m_videoFrame.frameFormat));
        videoObj.Set("timecode", Napi::Number::New(env, static_cast<double>(m_videoFrame.timecode)));
        videoObj.Set("lineStride", Napi::Number::New(env, m_videoFrame.lineStride));
        videoObj.Set("timestamp", Napi::Number::New(env, static_cast<double>(m_videoFrame.timestamp)));
        
        if (!m_videoFrame.metadata.empty()) {
            videoObj.Set("metadata", Napi::String::New(env, m_videoFrame.metadata));
        }
        
        if (!m_videoFrame.data.empty()) {
            Napi::Buffer<uint8_t> dataBuffer = Napi::Buffer<uint8_t>::Copy(
                env, m_videoFrame.data.data(), m_videoFrame.data.size()
            );
            videoObj.Set("data", dataBuffer);
        }
        
        result.Set("video", videoObj);
    }
    
    if (m_audioFrame.valid) {
        Napi::Object audioObj = Napi::Object::New(env);
        audioObj.Set("sampleRate", Napi::Number::New(env, m_audioFrame.sampleRate));
        audioObj.Set("noChannels", Napi::Number::New(env, m_audioFrame.noChannels));
        audioObj.Set("noSamples", Napi::Number::New(env, m_audioFrame.noSamples));
        audioObj.Set("timecode", Napi::Number::New(env, static_cast<double>(m_audioFrame.timecode)));
        audioObj.Set("channelStride", Napi::Number::New(env, m_audioFrame.channelStride));
        audioObj.Set("timestamp", Napi::Number::New(env, static_cast<double>(m_audioFrame.timestamp)));
        
        if (!m_audioFrame.metadata.empty()) {
            audioObj.Set("metadata", Napi::String::New(env, m_audioFrame.metadata));
        }
        
        if (!m_audioFrame.data.empty()) {
            Napi::Float32Array dataArray = Napi::Float32Array::New(env, m_audioFrame.data.size());
            memcpy(dataArray.Data(), m_audioFrame.data.data(), m_audioFrame.data.size() * sizeof(float));
            audioObj.Set("data", dataArray);
        }
        
        result.Set("audio", audioObj);
    }
    
    if (m_metadataFrame.valid) {
        Napi::Object metadataObj = Napi::Object::New(env);
        metadataObj.Set("data", Napi::String::New(env, m_metadataFrame.data));
        metadataObj.Set("timecode", Napi::Number::New(env, static_cast<double>(m_metadataFrame.timecode)));
        result.Set("metadata", metadataObj);
    }
    
    m_deferred.Resolve(result);
}

// ============================================================================
// Sender Async Workers
// ============================================================================

SendVideoWorker::SendVideoWorker(
    Napi::Env env,
    NDIlib_send_instance_t sender,
    NDIlib_video_frame_v2_t frame,
    uint8_t* dataBuffer
) : Napi::AsyncWorker(env),
    m_sender(sender),
    m_frame(frame),
    m_dataBuffer(dataBuffer),
    m_deferred(Napi::Promise::Deferred::New(env))
{
}

SendVideoWorker::~SendVideoWorker() {
    if (m_dataBuffer) {
        delete[] m_dataBuffer;
        m_dataBuffer = nullptr;
    }
}

void SendVideoWorker::Execute() {
    NDIlib_send_send_video_v2(m_sender, &m_frame);
}

void SendVideoWorker::OnOK() {
    m_deferred.Resolve(Env().Undefined());
}

void SendVideoWorker::OnError(const Napi::Error& error) {
    m_deferred.Reject(error.Value());
}

SendAudioWorker::SendAudioWorker(
    Napi::Env env,
    NDIlib_send_instance_t sender,
    NDIlib_audio_frame_v2_t frame,
    float* dataBuffer
) : Napi::AsyncWorker(env),
    m_sender(sender),
    m_frame(frame),
    m_dataBuffer(dataBuffer),
    m_deferred(Napi::Promise::Deferred::New(env))
{
}

SendAudioWorker::~SendAudioWorker() {
    if (m_dataBuffer) {
        delete[] m_dataBuffer;
        m_dataBuffer = nullptr;
    }
}

void SendAudioWorker::Execute() {
    NDIlib_send_send_audio_v2(m_sender, &m_frame);
}

void SendAudioWorker::OnOK() {
    m_deferred.Resolve(Env().Undefined());
}

void SendAudioWorker::OnError(const Napi::Error& error) {
    m_deferred.Reject(error.Value());
}

GetTallyWorker::GetTallyWorker(
    Napi::Env env,
    NDIlib_send_instance_t sender,
    uint32_t timeout
) : Napi::AsyncWorker(env),
    m_sender(sender),
    m_timeout(timeout),
    m_success(false),
    m_deferred(Napi::Promise::Deferred::New(env))
{
    m_tally = {};
}

void GetTallyWorker::Execute() {
    m_success = NDIlib_send_get_tally(m_sender, &m_tally, m_timeout);
}

void GetTallyWorker::OnOK() {
    Napi::Env env = Env();
    Napi::HandleScope scope(env);
    
    if (!m_success) {
        m_deferred.Resolve(env.Null());
        return;
    }
    
    Napi::Object result = Napi::Object::New(env);
    result.Set("onProgram", Napi::Boolean::New(env, m_tally.on_program));
    result.Set("onPreview", Napi::Boolean::New(env, m_tally.on_preview));
    
    m_deferred.Resolve(result);
}

GetConnectionsWorker::GetConnectionsWorker(
    Napi::Env env,
    NDIlib_send_instance_t sender,
    uint32_t timeout
) : Napi::AsyncWorker(env),
    m_sender(sender),
    m_timeout(timeout),
    m_numConnections(0),
    m_deferred(Napi::Promise::Deferred::New(env))
{
}

void GetConnectionsWorker::Execute() {
    m_numConnections = NDIlib_send_get_no_connections(m_sender, m_timeout);
}

void GetConnectionsWorker::OnOK() {
    m_deferred.Resolve(Napi::Number::New(Env(), m_numConnections));
}
