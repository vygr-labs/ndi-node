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
 * NDI Sender - Video/Audio transmission functionality
 */

#ifndef NDI_SENDER_H
#define NDI_SENDER_H

#include <napi.h>
#include "../deps/ndi/include/Processing.NDI.Lib.h"

class NdiSender : public Napi::ObjectWrap<NdiSender> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    NdiSender(const Napi::CallbackInfo& info);
    ~NdiSender();

private:
    static Napi::FunctionReference constructor;
    
    // Instance methods
    Napi::Value SendVideo(const Napi::CallbackInfo& info);
    Napi::Value SendVideoAsync(const Napi::CallbackInfo& info);
    Napi::Value SendAudio(const Napi::CallbackInfo& info);
    Napi::Value SendMetadata(const Napi::CallbackInfo& info);
    Napi::Value GetTally(const Napi::CallbackInfo& info);
    Napi::Value SetTally(const Napi::CallbackInfo& info);
    Napi::Value GetConnections(const Napi::CallbackInfo& info);
    Napi::Value GetSourceName(const Napi::CallbackInfo& info);
    Napi::Value ClearConnectionMetadata(const Napi::CallbackInfo& info);
    Napi::Value AddConnectionMetadata(const Napi::CallbackInfo& info);
    Napi::Value Destroy(const Napi::CallbackInfo& info);
    Napi::Value IsValid(const Napi::CallbackInfo& info);
    
    // Internal state
    NDIlib_send_instance_t m_sender;
    bool m_destroyed;
    uint8_t* m_asyncVideoBuffer;
};

#endif // NDI_SENDER_H
