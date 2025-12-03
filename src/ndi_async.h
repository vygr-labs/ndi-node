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
 * NDI Async Workers - Async worker classes for non-blocking NDI operations
 */

#ifndef NDI_ASYNC_H
#define NDI_ASYNC_H

#include <napi.h>
#include "../deps/ndi/include/Processing.NDI.Lib.h"
#include <vector>
#include <string>

// ============================================================================
// Finder Async Workers
// ============================================================================

/**
 * Async worker for waiting for NDI sources
 */
class WaitForSourcesWorker : public Napi::AsyncWorker {
public:
    WaitForSourcesWorker(
        Napi::Env env,
        NDIlib_find_instance_t finder,
        uint32_t timeout
    );

    void Execute() override;
    void OnOK() override;
    
    Napi::Promise::Deferred m_deferred;

private:
    NDIlib_find_instance_t m_finder;
    uint32_t m_timeout;
    bool m_changed;
    std::vector<std::pair<std::string, std::string>> m_sources;
    uint32_t m_numSources;
};

/**
 * Async worker for getting current sources without blocking
 */
class GetSourcesWorker : public Napi::AsyncWorker {
public:
    GetSourcesWorker(
        Napi::Env env,
        NDIlib_find_instance_t finder
    );

    void Execute() override;
    void OnOK() override;
    
    Napi::Promise::Deferred m_deferred;

private:
    NDIlib_find_instance_t m_finder;
    std::vector<std::pair<std::string, std::string>> m_sources;
};

// ============================================================================
// Receiver Async Workers
// ============================================================================

/**
 * Captured frame data that can be passed between threads
 */
struct CapturedVideoFrame {
    int xres;
    int yres;
    std::string fourCC;
    int frameRateN;
    int frameRateD;
    float pictureAspectRatio;
    std::string frameFormat;
    int64_t timecode;
    int lineStride;
    std::vector<uint8_t> data;
    std::string metadata;
    int64_t timestamp;
    bool valid;
};

struct CapturedAudioFrame {
    int sampleRate;
    int noChannels;
    int noSamples;
    int64_t timecode;
    int channelStride;
    std::vector<float> data;
    std::string metadata;
    int64_t timestamp;
    bool valid;
};

struct CapturedMetadataFrame {
    std::string data;
    int64_t timecode;
    bool valid;
};

/**
 * Async worker for capturing video frames
 */
class CaptureVideoWorker : public Napi::AsyncWorker {
public:
    CaptureVideoWorker(
        Napi::Env env,
        NDIlib_recv_instance_t receiver,
        uint32_t timeout
    );

    void Execute() override;
    void OnOK() override;
    
    Napi::Promise::Deferred m_deferred;

private:
    NDIlib_recv_instance_t m_receiver;
    uint32_t m_timeout;
    CapturedVideoFrame m_frame;
};

/**
 * Async worker for capturing audio frames
 */
class CaptureAudioWorker : public Napi::AsyncWorker {
public:
    CaptureAudioWorker(
        Napi::Env env,
        NDIlib_recv_instance_t receiver,
        uint32_t timeout
    );

    void Execute() override;
    void OnOK() override;
    
    Napi::Promise::Deferred m_deferred;

private:
    NDIlib_recv_instance_t m_receiver;
    uint32_t m_timeout;
    CapturedAudioFrame m_frame;
};

/**
 * Async worker for capturing any frame type
 */
class CaptureWorker : public Napi::AsyncWorker {
public:
    CaptureWorker(
        Napi::Env env,
        NDIlib_recv_instance_t receiver,
        uint32_t timeout
    );

    void Execute() override;
    void OnOK() override;
    
    Napi::Promise::Deferred m_deferred;

private:
    NDIlib_recv_instance_t m_receiver;
    uint32_t m_timeout;
    NDIlib_frame_type_e m_frameType;
    CapturedVideoFrame m_videoFrame;
    CapturedAudioFrame m_audioFrame;
    CapturedMetadataFrame m_metadataFrame;
};

// ============================================================================
// Sender Async Workers
// ============================================================================

/**
 * Async worker for sending video frames
 */
class SendVideoWorker : public Napi::AsyncWorker {
public:
    SendVideoWorker(
        Napi::Env env,
        NDIlib_send_instance_t sender,
        NDIlib_video_frame_v2_t frame,
        uint8_t* dataBuffer
    );
    
    ~SendVideoWorker();

    void Execute() override;
    void OnOK() override;
    void OnError(const Napi::Error& error) override;
    
    Napi::Promise::Deferred m_deferred;

private:
    NDIlib_send_instance_t m_sender;
    NDIlib_video_frame_v2_t m_frame;
    uint8_t* m_dataBuffer;
};

/**
 * Async worker for sending audio frames
 */
class SendAudioWorker : public Napi::AsyncWorker {
public:
    SendAudioWorker(
        Napi::Env env,
        NDIlib_send_instance_t sender,
        NDIlib_audio_frame_v2_t frame,
        float* dataBuffer
    );
    
    ~SendAudioWorker();

    void Execute() override;
    void OnOK() override;
    void OnError(const Napi::Error& error) override;
    
    Napi::Promise::Deferred m_deferred;

private:
    NDIlib_send_instance_t m_sender;
    NDIlib_audio_frame_v2_t m_frame;
    float* m_dataBuffer;
};

/**
 * Async worker for getting tally with timeout
 */
class GetTallyWorker : public Napi::AsyncWorker {
public:
    GetTallyWorker(
        Napi::Env env,
        NDIlib_send_instance_t sender,
        uint32_t timeout
    );

    void Execute() override;
    void OnOK() override;
    
    Napi::Promise::Deferred m_deferred;

private:
    NDIlib_send_instance_t m_sender;
    uint32_t m_timeout;
    NDIlib_tally_t m_tally;
    bool m_success;
};

/**
 * Async worker for getting connection count with timeout
 */
class GetConnectionsWorker : public Napi::AsyncWorker {
public:
    GetConnectionsWorker(
        Napi::Env env,
        NDIlib_send_instance_t sender,
        uint32_t timeout
    );

    void Execute() override;
    void OnOK() override;
    
    Napi::Promise::Deferred m_deferred;

private:
    NDIlib_send_instance_t m_sender;
    uint32_t m_timeout;
    int m_numConnections;
};

#endif // NDI_ASYNC_H
