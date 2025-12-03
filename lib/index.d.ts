/**
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

/**
 * TypeScript definitions for ndi-node
 * NDI (Network Device Interface) bindings for Node.js
 */

import { EventEmitter } from 'events';

// ============================================================================
// Constants
// ============================================================================

export declare const FourCC: {
    readonly UYVY: 'UYVY';
    readonly BGRA: 'BGRA';
    readonly BGRX: 'BGRX';
    readonly RGBA: 'RGBA';
    readonly RGBX: 'RGBX';
    readonly I420: 'I420';
    readonly NV12: 'NV12';
    readonly P216: 'P216';
    readonly PA16: 'PA16';
};

export declare const FrameFormat: {
    readonly PROGRESSIVE: 'progressive';
    readonly INTERLEAVED: 'interleaved';
    readonly FIELD_0: 'field0';
    readonly FIELD_1: 'field1';
};

export declare const Bandwidth: {
    readonly METADATA_ONLY: 'metadata_only';
    readonly AUDIO_ONLY: 'audio_only';
    readonly LOWEST: 'lowest';
    readonly HIGHEST: 'highest';
};

export declare const ColorFormat: {
    readonly BGRX_BGRA: 'BGRX_BGRA';
    readonly UYVY_BGRA: 'UYVY_BGRA';
    readonly RGBX_RGBA: 'RGBX_RGBA';
    readonly UYVY_RGBA: 'UYVY_RGBA';
    readonly FASTEST: 'fastest';
    readonly BEST: 'best';
};

export declare const FrameType: {
    readonly NONE: 'none';
    readonly VIDEO: 'video';
    readonly AUDIO: 'audio';
    readonly METADATA: 'metadata';
    readonly ERROR: 'error';
    readonly STATUS_CHANGE: 'status_change';
};

// ============================================================================
// Types
// ============================================================================

export type FourCCType = typeof FourCC[keyof typeof FourCC];
export type FrameFormatType = typeof FrameFormat[keyof typeof FrameFormat];
export type BandwidthType = typeof Bandwidth[keyof typeof Bandwidth];
export type ColorFormatType = typeof ColorFormat[keyof typeof ColorFormat];
export type FrameTypeValue = typeof FrameType[keyof typeof FrameType];

export interface NdiSource {
    name: string;
    urlAddress?: string;
}

export interface VideoFrame {
    xres: number;
    yres: number;
    fourCC?: FourCCType;
    frameRateN?: number;
    frameRateD?: number;
    pictureAspectRatio?: number;
    frameFormatType?: FrameFormatType;
    timecode?: number;
    data: Buffer;
    lineStrideInBytes?: number;
    metadata?: string;
    timestamp?: number;
}

export interface AudioFrame {
    sampleRate?: number;
    noChannels?: number;
    noSamples: number;
    timecode?: number;
    data: Buffer;
    channelStrideInBytes?: number;
    metadata?: string;
    timestamp?: number;
}

export interface MetadataFrame {
    length?: number;
    timecode?: number;
    data: string;
}

export interface Tally {
    onProgram: boolean;
    onPreview: boolean;
}

export interface CaptureResult {
    type: FrameTypeValue;
    video?: VideoFrame;
    audio?: AudioFrame;
    metadata?: MetadataFrame;
}

// ============================================================================
// Core Functions
// ============================================================================

/**
 * Initialize the NDI library. Must be called before using any other functions.
 * @returns True if initialization was successful
 */
export declare function initialize(): boolean;

/**
 * Destroy/cleanup the NDI library. Should be called when done using NDI.
 */
export declare function destroy(): void;

/**
 * Check if NDI has been initialized
 * @returns True if NDI is initialized
 */
export declare function isInitialized(): boolean;

/**
 * Get the NDI library version string
 * @returns Version string or null if not available
 */
export declare function version(): string | null;

/**
 * Find NDI sources on the network (convenience function)
 * @param timeout How long to search for sources (default: 5000ms)
 * @param options Finder options
 * @returns Promise that resolves to array of found sources
 */
export declare function find(timeout?: number, options?: FinderOptions): Promise<NdiSource[]>;

// ============================================================================
// Finder
// ============================================================================

export interface FinderOptions {
    /** Include local sources (default: true) */
    showLocalSources?: boolean;
    /** Comma-separated list of groups to search */
    groups?: string;
    /** Extra IPs to search for sources */
    extraIps?: string;
}

export interface FinderEvents {
    sources: (sources: NdiSource[]) => void;
}

export declare class Finder extends EventEmitter {
    constructor(options?: FinderOptions);

    /**
     * Get currently discovered sources
     */
    getSources(): NdiSource[];

    /**
     * Wait for sources to change
     * @param timeout Timeout in milliseconds (default: 1000)
     * @returns True if sources changed during the wait
     */
    waitForSources(timeout?: number): boolean;

    /**
     * Get currently discovered sources (async, non-blocking)
     * @returns Promise resolving to array of source objects
     */
    getSourcesAsync(): Promise<NdiSource[]>;

    /**
     * Wait for sources to change (async, non-blocking)
     * @param timeout Timeout in milliseconds (default: 1000)
     * @returns Promise resolving to object with changed flag and sources
     */
    waitForSourcesAsync(timeout?: number): Promise<{ changed: boolean; sources: NdiSource[] }>;

    /**
     * Start polling for sources. Emits 'sources' event when sources change.
     * @param interval Poll interval in milliseconds (default: 1000)
     */
    startPolling(interval?: number): void;

    /**
     * Stop polling for sources
     */
    stopPolling(): void;

    /**
     * Check if finder is valid
     */
    isValid(): boolean;

    /**
     * Destroy the finder and release resources
     */
    destroy(): void;

    on<K extends keyof FinderEvents>(event: K, listener: FinderEvents[K]): this;
    emit<K extends keyof FinderEvents>(event: K, ...args: Parameters<FinderEvents[K]>): boolean;
}

// ============================================================================
// Sender
// ============================================================================

export interface SenderOptions {
    /** Name of the NDI source */
    name: string;
    /** Comma-separated list of groups */
    groups?: string;
    /** Clock video to frame rate (default: true) */
    clockVideo?: boolean;
    /** Clock audio to sample rate (default: true) */
    clockAudio?: boolean;
}

export interface SenderEvents {
    tally: (tally: Tally) => void;
}

export declare class Sender extends EventEmitter {
    constructor(options: SenderOptions);

    /**
     * Send a video frame
     */
    sendVideo(frame: VideoFrame): void;

    /**
     * Send a video frame asynchronously (non-blocking, uses NDI async API)
     */
    sendVideoAsync(frame: VideoFrame): void;

    /**
     * Send a video frame (Promise-based async, runs on background thread)
     * @returns Promise that resolves when the frame is sent
     */
    sendVideoPromise(frame: VideoFrame): Promise<void>;

    /**
     * Send an audio frame
     */
    sendAudio(frame: AudioFrame): void;

    /**
     * Send an audio frame (Promise-based async, runs on background thread)
     * @returns Promise that resolves when the frame is sent
     */
    sendAudioPromise(frame: AudioFrame): Promise<void>;

    /**
     * Send metadata
     */
    sendMetadata(frame: MetadataFrame): void;

    /**
     * Get the current tally state
     * @param timeout Timeout in milliseconds (0 = non-blocking)
     */
    getTally(timeout?: number): Tally | null;

    /**
     * Get the current tally state (async, non-blocking)
     * @param timeout Timeout in milliseconds
     * @returns Promise resolving to tally state or null
     */
    getTallyAsync(timeout?: number): Promise<Tally | null>;

    /**
     * Set the tally state
     */
    setTally(tally: Tally): void;

    /**
     * Get the number of current connections
     * @param timeout Timeout in milliseconds
     */
    getConnections(timeout?: number): number;

    /**
     * Get the number of current connections (async, non-blocking)
     * @param timeout Timeout in milliseconds
     * @returns Promise resolving to connection count
     */
    getConnectionsAsync(timeout?: number): Promise<number>;

    /**
     * Get the full source name (includes computer name)
     */
    getSourceName(): string | null;

    /**
     * Clear all connection metadata
     */
    clearConnectionMetadata(): void;

    /**
     * Add connection metadata
     */
    addConnectionMetadata(frame: MetadataFrame): void;

    /**
     * Start polling for tally changes. Emits 'tally' event when tally changes.
     * @param interval Poll interval in milliseconds (default: 100)
     */
    startTallyPolling(interval?: number): void;

    /**
     * Stop polling for tally changes
     */
    stopTallyPolling(): void;

    /**
     * Check if sender is valid
     */
    isValid(): boolean;

    /**
     * Destroy the sender and release resources
     */
    destroy(): void;

    on<K extends keyof SenderEvents>(event: K, listener: SenderEvents[K]): this;
    emit<K extends keyof SenderEvents>(event: K, ...args: Parameters<SenderEvents[K]>): boolean;
}

// ============================================================================
// Receiver
// ============================================================================

export interface ReceiverOptions {
    /** Source to connect to */
    source?: NdiSource;
    /** Color format (default: 'BGRX_BGRA') */
    colorFormat?: ColorFormatType;
    /** Bandwidth mode (default: 'highest') */
    bandwidth?: BandwidthType;
    /** Allow video fields (default: true) */
    allowVideoFields?: boolean;
    /** Receiver name */
    name?: string;
}

export interface ReceiverEvents {
    video: (frame: VideoFrame) => void;
    audio: (frame: AudioFrame) => void;
    metadata: (frame: MetadataFrame) => void;
    status_change: () => void;
    error: (error: Error) => void;
}

export declare class Receiver extends EventEmitter {
    constructor(options?: ReceiverOptions);

    /**
     * Connect to an NDI source
     */
    connect(source: NdiSource): void;

    /**
     * Capture a frame (video, audio, or metadata)
     * @param timeout Timeout in milliseconds (default: 1000)
     */
    capture(timeout?: number): CaptureResult;

    /**
     * Capture only video frames
     * @param timeout Timeout in milliseconds (default: 1000)
     */
    captureVideo(timeout?: number): VideoFrame | null;

    /**
     * Capture only audio frames
     * @param timeout Timeout in milliseconds (default: 1000)
     */
    captureAudio(timeout?: number): AudioFrame | null;

    /**
     * Capture a frame asynchronously (video, audio, or metadata) - non-blocking
     * @param timeout Timeout in milliseconds (default: 1000)
     * @returns Promise resolving to capture result
     */
    captureAsync(timeout?: number): Promise<CaptureResult>;

    /**
     * Capture only video frames asynchronously - non-blocking
     * @param timeout Timeout in milliseconds (default: 1000)
     * @returns Promise resolving to video frame or null
     */
    captureVideoAsync(timeout?: number): Promise<VideoFrame | null>;

    /**
     * Capture only audio frames asynchronously - non-blocking
     * @param timeout Timeout in milliseconds (default: 1000)
     * @returns Promise resolving to audio frame or null
     */
    captureAudioAsync(timeout?: number): Promise<AudioFrame | null>;

    /**
     * Set tally information
     */
    setTally(tally: Tally): boolean;

    /**
     * Send metadata to the source
     */
    sendMetadata(frame: MetadataFrame): void;

    // PTZ Controls

    /**
     * Check if PTZ is supported
     */
    ptzIsSupported(): boolean;

    /**
     * Set PTZ zoom level
     * @param zoom Zoom value (0.0 = wide, 1.0 = telephoto)
     */
    ptzZoom(zoom: number): boolean;

    /**
     * Set PTZ pan/tilt position
     * @param pan Pan value (-1.0 to 1.0)
     * @param tilt Tilt value (-1.0 to 1.0)
     */
    ptzPanTilt(pan: number, tilt: number): boolean;

    /**
     * Set PTZ pan/tilt speed
     * @param panSpeed Pan speed (-1.0 to 1.0)
     * @param tiltSpeed Tilt speed (-1.0 to 1.0)
     */
    ptzPanTiltSpeed(panSpeed: number, tiltSpeed: number): boolean;

    /**
     * Store a PTZ preset
     * @param presetNo Preset number (0-255)
     */
    ptzStorePreset(presetNo: number): boolean;

    /**
     * Recall a PTZ preset
     * @param presetNo Preset number (0-255)
     * @param speed Speed to move to preset (default: 1.0)
     */
    ptzRecallPreset(presetNo: number, speed?: number): boolean;

    /**
     * Enable auto focus
     */
    ptzAutoFocus(): boolean;

    /**
     * Set manual focus
     * @param focus Focus value (0.0 = near, 1.0 = far)
     */
    ptzFocus(focus: number): boolean;

    /**
     * Set focus speed
     * @param speed Focus speed (-1.0 to 1.0)
     */
    ptzFocusSpeed(speed: number): boolean;

    /**
     * Enable auto white balance
     */
    ptzWhiteBalanceAuto(): boolean;

    /**
     * Set indoor white balance preset
     */
    ptzWhiteBalanceIndoor(): boolean;

    /**
     * Set outdoor white balance preset
     */
    ptzWhiteBalanceOutdoor(): boolean;

    /**
     * Perform one-shot white balance
     */
    ptzWhiteBalanceOneshot(): boolean;

    /**
     * Set manual white balance
     * @param red Red value
     * @param blue Blue value
     */
    ptzWhiteBalanceManual(red: number, blue: number): boolean;

    /**
     * Enable auto exposure
     */
    ptzExposureAuto(): boolean;

    /**
     * Set manual exposure
     * @param exposure Exposure level
     */
    ptzExposureManual(exposure: number): boolean;

    /**
     * Start continuous capture. Emits 'video', 'audio', and 'metadata' events.
     * @param timeout Capture timeout per frame (default: 100)
     * @param useAsync Use async capture for non-blocking operation (default: true)
     */
    startCapture(timeout?: number, useAsync?: boolean): void;

    /**
     * Stop continuous capture
     */
    stopCapture(): void;

    /**
     * Check if receiver is valid
     */
    isValid(): boolean;

    /**
     * Destroy the receiver and release resources
     */
    destroy(): void;

    on<K extends keyof ReceiverEvents>(event: K, listener: ReceiverEvents[K]): this;
    emit<K extends keyof ReceiverEvents>(event: K, ...args: Parameters<ReceiverEvents[K]>): boolean;
}

// ============================================================================
// Native addon (advanced use)
// ============================================================================

export declare const native: any;
