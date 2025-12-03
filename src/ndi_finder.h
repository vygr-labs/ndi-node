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
 * NDI Finder - Source discovery functionality
 */

#ifndef NDI_FINDER_H
#define NDI_FINDER_H

#include <napi.h>
#include "../deps/ndi/include/Processing.NDI.Lib.h"

class NdiFinder : public Napi::ObjectWrap<NdiFinder> {
public:
    static Napi::Object Init(Napi::Env env, Napi::Object exports);
    NdiFinder(const Napi::CallbackInfo& info);
    ~NdiFinder();
    
    // Allow async workers to access the finder instance
    NDIlib_find_instance_t GetFinder() const { return m_finder; }
    bool IsDestroyed() const { return m_destroyed; }

private:
    static Napi::FunctionReference constructor;
    
    // Synchronous instance methods
    Napi::Value GetSources(const Napi::CallbackInfo& info);
    Napi::Value WaitForSources(const Napi::CallbackInfo& info);
    Napi::Value Destroy(const Napi::CallbackInfo& info);
    Napi::Value IsValid(const Napi::CallbackInfo& info);
    
    // Async instance methods
    Napi::Value GetSourcesAsync(const Napi::CallbackInfo& info);
    Napi::Value WaitForSourcesAsync(const Napi::CallbackInfo& info);
    
    // Internal state
    NDIlib_find_instance_t m_finder;
    bool m_destroyed;
};

#endif // NDI_FINDER_H
