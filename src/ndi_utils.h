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
 * NDI Utils - Utility functions for the NDI Node.js addon
 */

#ifndef NDI_UTILS_H
#define NDI_UTILS_H

#include <napi.h>
#include "../deps/ndi/include/Processing.NDI.Lib.h"

namespace NdiUtils {

// Convert NDI source to JavaScript object
Napi::Object SourceToObject(Napi::Env env, const NDIlib_source_t& source);

// Convert JavaScript object to NDI source
NDIlib_source_t ObjectToSource(Napi::Env env, const Napi::Object& obj);

// Convert NDI video frame to JavaScript object
Napi::Object VideoFrameToObject(Napi::Env env, const NDIlib_video_frame_v2_t& frame);

// Convert JavaScript object to NDI video frame
NDIlib_video_frame_v2_t ObjectToVideoFrame(Napi::Env env, const Napi::Object& obj, uint8_t** dataBuffer);

// Convert NDI audio frame to JavaScript object
Napi::Object AudioFrameToObject(Napi::Env env, const NDIlib_audio_frame_v2_t& frame);

// Convert JavaScript object to NDI audio frame
NDIlib_audio_frame_v2_t ObjectToAudioFrame(Napi::Env env, const Napi::Object& obj, float** dataBuffer);

// Convert NDI metadata frame to JavaScript object
Napi::Object MetadataFrameToObject(Napi::Env env, const NDIlib_metadata_frame_t& frame);

// Convert JavaScript object to NDI metadata frame
NDIlib_metadata_frame_t ObjectToMetadataFrame(Napi::Env env, const Napi::Object& obj, char** dataBuffer);

// Convert NDI tally to JavaScript object
Napi::Object TallyToObject(Napi::Env env, const NDIlib_tally_t& tally);

// Convert JavaScript object to NDI tally
NDIlib_tally_t ObjectToTally(Napi::Env env, const Napi::Object& obj);

// FourCC video type conversion helpers
NDIlib_FourCC_video_type_e StringToFourCC(const std::string& str);
std::string FourCCToString(NDIlib_FourCC_video_type_e fourcc);

// Frame format type conversion helpers
NDIlib_frame_format_type_e StringToFrameFormat(const std::string& str);
std::string FrameFormatToString(NDIlib_frame_format_type_e format);

// Bandwidth conversion helpers
NDIlib_recv_bandwidth_e StringToBandwidth(const std::string& str);
std::string BandwidthToString(NDIlib_recv_bandwidth_e bandwidth);

// Color format conversion helpers
NDIlib_recv_color_format_e StringToColorFormat(const std::string& str);
std::string ColorFormatToString(NDIlib_recv_color_format_e format);

// Frame type conversion helpers
std::string FrameTypeToString(NDIlib_frame_type_e type);

} // namespace NdiUtils

#endif // NDI_UTILS_H
