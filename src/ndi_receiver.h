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
 * NDI Receiver - Video/Audio reception functionality
 */

#ifndef NDI_RECEIVER_H
#define NDI_RECEIVER_H

#include <napi.h>
#include "../deps/ndi/include/Processing.NDI.Lib.h"

class NdiReceiver : public Napi::ObjectWrap<NdiReceiver> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    NdiReceiver(const Napi::CallbackInfo& info);
    ~NdiReceiver();

private:
    static Napi::FunctionReference constructor;
    
    // Instance methods
    Napi::Value Connect(const Napi::CallbackInfo& info);
    Napi::Value Capture(const Napi::CallbackInfo& info);
    Napi::Value CaptureVideo(const Napi::CallbackInfo& info);
    Napi::Value CaptureAudio(const Napi::CallbackInfo& info);
    Napi::Value SetTally(const Napi::CallbackInfo& info);
    Napi::Value SendMetadata(const Napi::CallbackInfo& info);
    Napi::Value PtzIsSupported(const Napi::CallbackInfo& info);
    Napi::Value PtzZoom(const Napi::CallbackInfo& info);
    Napi::Value PtzPanTilt(const Napi::CallbackInfo& info);
    Napi::Value PtzPanTiltSpeed(const Napi::CallbackInfo& info);
    Napi::Value PtzStorePreset(const Napi::CallbackInfo& info);
    Napi::Value PtzRecallPreset(const Napi::CallbackInfo& info);
    Napi::Value PtzAutoFocus(const Napi::CallbackInfo& info);
    Napi::Value PtzFocus(const Napi::CallbackInfo& info);
    Napi::Value PtzFocusSpeed(const Napi::CallbackInfo& info);
    Napi::Value PtzWhiteBalanceAuto(const Napi::CallbackInfo& info);
    Napi::Value PtzWhiteBalanceIndoor(const Napi::CallbackInfo& info);
    Napi::Value PtzWhiteBalanceOutdoor(const Napi::CallbackInfo& info);
    Napi::Value PtzWhiteBalanceOneshot(const Napi::CallbackInfo& info);
    Napi::Value PtzWhiteBalanceManual(const Napi::CallbackInfo& info);
    Napi::Value PtzExposureAuto(const Napi::CallbackInfo& info);
    Napi::Value PtzExposureManual(const Napi::CallbackInfo& info);
    Napi::Value Destroy(const Napi::CallbackInfo& info);
    Napi::Value IsValid(const Napi::CallbackInfo& info);
    
    // Internal state
    NDIlib_recv_instance_t m_receiver;
    bool m_destroyed;
};

#endif // NDI_RECEIVER_H
