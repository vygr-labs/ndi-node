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
 * NDI Node.js Bindings
 * High-level JavaScript API for NDI (Network Device Interface)
 */

'use strict';

const path = require('path');
const EventEmitter = require('events');

// Try to load the native addon
let ndiAddon;
try {
    ndiAddon = require('../build/Release/ndi_addon.node');
} catch (e) {
    try {
        ndiAddon = require('../build/Debug/ndi_addon.node');
    } catch (e2) {
        throw new Error(
            'Failed to load NDI native addon. Please ensure you have:\n' +
            '1. Installed the NDI SDK from https://ndi.video/download-ndi-sdk/\n' +
            '2. Placed the SDK files in the deps/ndi directory\n' +
            '3. Run npm install to build the native addon\n' +
            `Original error: ${e.message}`
        );
    }
}

// Export constants from native addon
const FourCC = ndiAddon.FourCC;
const FrameFormat = ndiAddon.FrameFormat;
const Bandwidth = ndiAddon.Bandwidth;
const ColorFormat = ndiAddon.ColorFormat;
const FrameType = ndiAddon.FrameType;

/**
 * Initialize the NDI library. Must be called before using any other functions.
 * @returns {boolean} True if initialization was successful
 */
function initialize() {
    return ndiAddon.initialize();
}

/**
 * Destroy/cleanup the NDI library. Should be called when done using NDI.
 */
function destroy() {
    ndiAddon.destroy();
}

/**
 * Check if NDI has been initialized
 * @returns {boolean} True if NDI is initialized
 */
function isInitialized() {
    return ndiAddon.isInitialized();
}

/**
 * Get the NDI library version string
 * @returns {string|null} Version string or null if not available
 */
function version() {
    return ndiAddon.version();
}

/**
 * NDI Finder - Discovers NDI sources on the network
 */
class Finder extends EventEmitter {
    /**
     * Create a new NDI Finder
     * @param {Object} options - Finder options
     * @param {boolean} [options.showLocalSources=true] - Include local sources
     * @param {string} [options.groups] - Comma-separated list of groups to search
     * @param {string} [options.extraIps] - Extra IPs to search for sources
     */
    constructor(options = {}) {
        super();
        this._finder = new ndiAddon.NdiFinder(options);
        this._polling = false;
        this._pollInterval = null;
    }

    /**
     * Get currently discovered sources
     * @returns {Array<{name: string, urlAddress: string}>} Array of source objects
     */
    getSources() {
        return this._finder.getSources();
    }

    /**
     * Wait for sources to change
     * @param {number} [timeout=1000] - Timeout in milliseconds
     * @returns {boolean} True if sources changed during the wait
     */
    waitForSources(timeout = 1000) {
        return this._finder.waitForSources(timeout);
    }

    /**
     * Get currently discovered sources (async, non-blocking)
     * @returns {Promise<Array<{name: string, urlAddress: string}>>} Array of source objects
     */
    getSourcesAsync() {
        return this._finder.getSourcesAsync();
    }

    /**
     * Wait for sources to change (async, non-blocking)
     * @param {number} [timeout=1000] - Timeout in milliseconds
     * @returns {Promise<{changed: boolean, sources: Array}>}
     */
    waitForSourcesAsync(timeout = 1000) {
        return this._finder.waitForSourcesAsync(timeout);
    }

    /**
     * Start polling for sources. Emits 'sources' event when sources change.
     * @param {number} [interval=1000] - Poll interval in milliseconds
     */
    startPolling(interval = 1000) {
        if (this._polling) return;
        
        this._polling = true;
        let lastSources = JSON.stringify(this.getSources());
        
        this._pollInterval = setInterval(() => {
            if (this.waitForSources(100)) {
                const sources = this.getSources();
                const sourcesJson = JSON.stringify(sources);
                
                if (sourcesJson !== lastSources) {
                    lastSources = sourcesJson;
                    this.emit('sources', sources);
                }
            }
        }, interval);
    }

    /**
     * Stop polling for sources
     */
    stopPolling() {
        if (this._pollInterval) {
            clearInterval(this._pollInterval);
            this._pollInterval = null;
        }
        this._polling = false;
    }

    /**
     * Check if finder is valid
     * @returns {boolean}
     */
    isValid() {
        return this._finder.isValid();
    }

    /**
     * Destroy the finder and release resources
     */
    destroy() {
        this.stopPolling();
        this._finder.destroy();
    }
}

/**
 * NDI Sender - Sends video and audio over NDI
 */
class Sender extends EventEmitter {
    /**
     * Create a new NDI Sender
     * @param {Object} options - Sender options
     * @param {string} options.name - Name of the NDI source
     * @param {string} [options.groups] - Comma-separated list of groups
     * @param {boolean} [options.clockVideo=true] - Clock video to frame rate
     * @param {boolean} [options.clockAudio=true] - Clock audio to sample rate
     */
    constructor(options) {
        super();
        
        if (!options || !options.name) {
            throw new Error('Sender name is required');
        }
        
        this._sender = new ndiAddon.NdiSender(options);
        this._tallyPolling = false;
        this._tallyInterval = null;
    }

    /**
     * Send a video frame
     * @param {Object} frame - Video frame object
     * @param {number} frame.xres - Width in pixels
     * @param {number} frame.yres - Height in pixels
     * @param {string} [frame.fourCC='BGRA'] - Pixel format (BGRA, RGBA, UYVY, etc.)
     * @param {number} [frame.frameRateN=30000] - Frame rate numerator
     * @param {number} [frame.frameRateD=1001] - Frame rate denominator
     * @param {string} [frame.frameFormatType='progressive'] - Frame format type
     * @param {Buffer} frame.data - Raw pixel data
     * @param {number} [frame.lineStrideInBytes] - Bytes per line (auto-calculated if not provided)
     */
    sendVideo(frame) {
        this._sender.sendVideo(frame);
    }

    /**
     * Send a video frame asynchronously (non-blocking, uses NDI async API)
     * @param {Object} frame - Video frame object (same as sendVideo)
     */
    sendVideoAsync(frame) {
        this._sender.sendVideoAsync(frame);
    }

    /**
     * Send a video frame (Promise-based async, runs on background thread)
     * @param {Object} frame - Video frame object (same as sendVideo)
     * @returns {Promise<void>}
     */
    sendVideoPromise(frame) {
        return this._sender.sendVideoPromise(frame);
    }

    /**
     * Send an audio frame
     * @param {Object} frame - Audio frame object
     * @param {number} [frame.sampleRate=48000] - Sample rate in Hz
     * @param {number} [frame.noChannels=2] - Number of audio channels
     * @param {number} frame.noSamples - Number of samples per channel
     * @param {Buffer} frame.data - Float32 audio data (planar format)
     */
    sendAudio(frame) {
        this._sender.sendAudio(frame);
    }

    /**
     * Send an audio frame (Promise-based async, runs on background thread)
     * @param {Object} frame - Audio frame object (same as sendAudio)
     * @returns {Promise<void>}
     */
    sendAudioPromise(frame) {
        return this._sender.sendAudioPromise(frame);
    }

    /**
     * Send metadata
     * @param {Object} frame - Metadata frame
     * @param {string} frame.data - XML metadata string
     */
    sendMetadata(frame) {
        this._sender.sendMetadata(frame);
    }

    /**
     * Get the current tally state
     * @param {number} [timeout=0] - Timeout in milliseconds (0 = non-blocking)
     * @returns {{onProgram: boolean, onPreview: boolean}|null}
     */
    getTally(timeout = 0) {
        return this._sender.getTally(timeout);
    }

    /**
     * Get the current tally state (async, non-blocking)
     * @param {number} [timeout=0] - Timeout in milliseconds
     * @returns {Promise<{onProgram: boolean, onPreview: boolean}|null>}
     */
    getTallyAsync(timeout = 0) {
        return this._sender.getTallyAsync(timeout);
    }

    /**
     * Set the tally state
     * @param {{onProgram: boolean, onPreview: boolean}} tally
     */
    setTally(tally) {
        this._sender.setTally(tally);
    }

    /**
     * Get the number of current connections
     * @param {number} [timeout=0] - Timeout in milliseconds
     * @returns {number}
     */
    getConnections(timeout = 0) {
        return this._sender.getConnections(timeout);
    }

    /**
     * Get the number of current connections (async, non-blocking)
     * @param {number} [timeout=0] - Timeout in milliseconds
     * @returns {Promise<number>}
     */
    getConnectionsAsync(timeout = 0) {
        return this._sender.getConnectionsAsync(timeout);
    }

    /**
     * Get the full source name (includes computer name)
     * @returns {string|null}
     */
    getSourceName() {
        return this._sender.getSourceName();
    }

    /**
     * Clear all connection metadata
     */
    clearConnectionMetadata() {
        this._sender.clearConnectionMetadata();
    }

    /**
     * Add connection metadata
     * @param {Object} frame - Metadata frame
     * @param {string} frame.data - XML metadata string
     */
    addConnectionMetadata(frame) {
        this._sender.addConnectionMetadata(frame);
    }

    /**
     * Start polling for tally changes. Emits 'tally' event when tally changes.
     * @param {number} [interval=100] - Poll interval in milliseconds
     */
    startTallyPolling(interval = 100) {
        if (this._tallyPolling) return;
        
        this._tallyPolling = true;
        let lastTally = JSON.stringify(this.getTally());
        
        this._tallyInterval = setInterval(() => {
            const tally = this.getTally();
            const tallyJson = JSON.stringify(tally);
            
            if (tallyJson !== lastTally) {
                lastTally = tallyJson;
                this.emit('tally', tally);
            }
        }, interval);
    }

    /**
     * Stop polling for tally changes
     */
    stopTallyPolling() {
        if (this._tallyInterval) {
            clearInterval(this._tallyInterval);
            this._tallyInterval = null;
        }
        this._tallyPolling = false;
    }

    /**
     * Check if sender is valid
     * @returns {boolean}
     */
    isValid() {
        return this._sender.isValid();
    }

    /**
     * Destroy the sender and release resources
     */
    destroy() {
        this.stopTallyPolling();
        this._sender.destroy();
    }
}

/**
 * NDI Receiver - Receives video and audio over NDI
 */
class Receiver extends EventEmitter {
    /**
     * Create a new NDI Receiver
     * @param {Object} [options] - Receiver options
     * @param {Object} [options.source] - Source to connect to
     * @param {string} [options.source.name] - Source name
     * @param {string} [options.source.urlAddress] - Source URL
     * @param {string} [options.colorFormat='BGRX_BGRA'] - Color format
     * @param {string} [options.bandwidth='highest'] - Bandwidth mode
     * @param {boolean} [options.allowVideoFields=true] - Allow video fields
     * @param {string} [options.name] - Receiver name
     */
    constructor(options = {}) {
        super();
        this._receiver = new ndiAddon.NdiReceiver(options);
        this._capturing = false;
        this._captureLoop = null;
    }

    /**
     * Connect to an NDI source
     * @param {Object} source - Source to connect to
     * @param {string} source.name - Source name
     * @param {string} [source.urlAddress] - Source URL
     */
    connect(source) {
        this._receiver.connect(source);
    }

    /**
     * Capture a frame (video, audio, or metadata)
     * @param {number} [timeout=1000] - Timeout in milliseconds
     * @returns {{type: string, video?: Object, audio?: Object, metadata?: Object}}
     */
    capture(timeout = 1000) {
        return this._receiver.capture(timeout);
    }

    /**
     * Capture only video frames
     * @param {number} [timeout=1000] - Timeout in milliseconds
     * @returns {Object|null} Video frame or null if timeout
     */
    captureVideo(timeout = 1000) {
        return this._receiver.captureVideo(timeout);
    }

    /**
     * Capture only audio frames
     * @param {number} [timeout=1000] - Timeout in milliseconds
     * @returns {Object|null} Audio frame or null if timeout
     */
    captureAudio(timeout = 1000) {
        return this._receiver.captureAudio(timeout);
    }

    /**
     * Capture a frame asynchronously (video, audio, or metadata) - non-blocking
     * @param {number} [timeout=1000] - Timeout in milliseconds
     * @returns {Promise<{type: string, video?: Object, audio?: Object, metadata?: Object}>}
     */
    captureAsync(timeout = 1000) {
        return this._receiver.captureAsync(timeout);
    }

    /**
     * Capture only video frames asynchronously - non-blocking
     * @param {number} [timeout=1000] - Timeout in milliseconds
     * @returns {Promise<Object|null>} Video frame or null if timeout
     */
    captureVideoAsync(timeout = 1000) {
        return this._receiver.captureVideoAsync(timeout);
    }

    /**
     * Capture only audio frames asynchronously - non-blocking
     * @param {number} [timeout=1000] - Timeout in milliseconds
     * @returns {Promise<Object|null>} Audio frame or null if timeout
     */
    captureAudioAsync(timeout = 1000) {
        return this._receiver.captureAudioAsync(timeout);
    }

    /**
     * Set tally information
     * @param {{onProgram: boolean, onPreview: boolean}} tally
     * @returns {boolean}
     */
    setTally(tally) {
        return this._receiver.setTally(tally);
    }

    /**
     * Send metadata to the source
     * @param {Object} frame - Metadata frame
     * @param {string} frame.data - XML metadata string
     */
    sendMetadata(frame) {
        this._receiver.sendMetadata(frame);
    }

    /**
     * Check if PTZ is supported
     * @returns {boolean}
     */
    ptzIsSupported() {
        return this._receiver.ptzIsSupported();
    }

    /**
     * Set PTZ zoom level
     * @param {number} zoom - Zoom value (0.0 = wide, 1.0 = telephoto)
     * @returns {boolean}
     */
    ptzZoom(zoom) {
        return this._receiver.ptzZoom(zoom);
    }

    /**
     * Set PTZ pan/tilt position
     * @param {number} pan - Pan value (-1.0 to 1.0)
     * @param {number} tilt - Tilt value (-1.0 to 1.0)
     * @returns {boolean}
     */
    ptzPanTilt(pan, tilt) {
        return this._receiver.ptzPanTilt(pan, tilt);
    }

    /**
     * Set PTZ pan/tilt speed
     * @param {number} panSpeed - Pan speed (-1.0 to 1.0)
     * @param {number} tiltSpeed - Tilt speed (-1.0 to 1.0)
     * @returns {boolean}
     */
    ptzPanTiltSpeed(panSpeed, tiltSpeed) {
        return this._receiver.ptzPanTiltSpeed(panSpeed, tiltSpeed);
    }

    /**
     * Store a PTZ preset
     * @param {number} presetNo - Preset number (0-255)
     * @returns {boolean}
     */
    ptzStorePreset(presetNo) {
        return this._receiver.ptzStorePreset(presetNo);
    }

    /**
     * Recall a PTZ preset
     * @param {number} presetNo - Preset number (0-255)
     * @param {number} [speed=1.0] - Speed to move to preset
     * @returns {boolean}
     */
    ptzRecallPreset(presetNo, speed = 1.0) {
        return this._receiver.ptzRecallPreset(presetNo, speed);
    }

    /**
     * Enable auto focus
     * @returns {boolean}
     */
    ptzAutoFocus() {
        return this._receiver.ptzAutoFocus();
    }

    /**
     * Set manual focus
     * @param {number} focus - Focus value (0.0 = near, 1.0 = far)
     * @returns {boolean}
     */
    ptzFocus(focus) {
        return this._receiver.ptzFocus(focus);
    }

    /**
     * Set focus speed
     * @param {number} speed - Focus speed (-1.0 to 1.0)
     * @returns {boolean}
     */
    ptzFocusSpeed(speed) {
        return this._receiver.ptzFocusSpeed(speed);
    }

    /**
     * Enable auto white balance
     * @returns {boolean}
     */
    ptzWhiteBalanceAuto() {
        return this._receiver.ptzWhiteBalanceAuto();
    }

    /**
     * Set indoor white balance preset
     * @returns {boolean}
     */
    ptzWhiteBalanceIndoor() {
        return this._receiver.ptzWhiteBalanceIndoor();
    }

    /**
     * Set outdoor white balance preset
     * @returns {boolean}
     */
    ptzWhiteBalanceOutdoor() {
        return this._receiver.ptzWhiteBalanceOutdoor();
    }

    /**
     * Perform one-shot white balance
     * @returns {boolean}
     */
    ptzWhiteBalanceOneshot() {
        return this._receiver.ptzWhiteBalanceOneshot();
    }

    /**
     * Set manual white balance
     * @param {number} red - Red value
     * @param {number} blue - Blue value
     * @returns {boolean}
     */
    ptzWhiteBalanceManual(red, blue) {
        return this._receiver.ptzWhiteBalanceManual(red, blue);
    }

    /**
     * Enable auto exposure
     * @returns {boolean}
     */
    ptzExposureAuto() {
        return this._receiver.ptzExposureAuto();
    }

    /**
     * Set manual exposure
     * @param {number} exposure - Exposure level
     * @returns {boolean}
     */
    ptzExposureManual(exposure) {
        return this._receiver.ptzExposureManual(exposure);
    }

    /**
     * Start continuous capture. Emits 'video', 'audio', and 'metadata' events.
     * @param {number} [timeout=100] - Capture timeout per frame
     * @param {boolean} [useAsync=true] - Use async capture (non-blocking)
     */
    startCapture(timeout = 100, useAsync = true) {
        if (this._capturing) return;
        
        this._capturing = true;
        
        if (useAsync) {
            // Use async capture for non-blocking operation
            const captureFrameAsync = async () => {
                while (this._capturing) {
                    try {
                        const result = await this.captureAsync(timeout);
                        
                        if (result && this._capturing) {
                            switch (result.type) {
                                case 'video':
                                    this.emit('video', result.video);
                                    break;
                                case 'audio':
                                    this.emit('audio', result.audio);
                                    break;
                                case 'metadata':
                                    this.emit('metadata', result.metadata);
                                    break;
                                case 'status_change':
                                    this.emit('status_change');
                                    break;
                                case 'error':
                                    this.emit('error', new Error('NDI receive error'));
                                    break;
                            }
                        }
                    } catch (err) {
                        if (this._capturing) {
                            this.emit('error', err);
                        }
                    }
                }
            };
            
            captureFrameAsync();
        } else {
            // Use sync capture with setImmediate for backwards compatibility
            const captureFrame = () => {
                if (!this._capturing) return;
                
                const result = this.capture(timeout);
                
                if (result) {
                    switch (result.type) {
                        case 'video':
                            this.emit('video', result.video);
                            break;
                        case 'audio':
                            this.emit('audio', result.audio);
                            break;
                        case 'metadata':
                            this.emit('metadata', result.metadata);
                            break;
                        case 'status_change':
                            this.emit('status_change');
                            break;
                        case 'error':
                            this.emit('error', new Error('NDI receive error'));
                            break;
                    }
                }
                
                // Use setImmediate for non-blocking capture loop
                if (this._capturing) {
                    this._captureLoop = setImmediate(captureFrame);
                }
            };
            
            this._captureLoop = setImmediate(captureFrame);
        }
    }

    /**
     * Stop continuous capture
     */
    stopCapture() {
        this._capturing = false;
        if (this._captureLoop) {
            clearImmediate(this._captureLoop);
            this._captureLoop = null;
        }
    }

    /**
     * Check if receiver is valid
     * @returns {boolean}
     */
    isValid() {
        return this._receiver.isValid();
    }

    /**
     * Destroy the receiver and release resources
     */
    destroy() {
        this.stopCapture();
        this._receiver.destroy();
    }
}

/**
 * Find NDI sources on the network (convenience function)
 * Uses async operations for non-blocking discovery
 * @param {number} [timeout=5000] - How long to search for sources
 * @param {Object} [options] - Finder options
 * @returns {Promise<Array>} Array of found sources
 */
async function find(timeout = 5000, options = {}) {
    const finder = new Finder(options);
    
    try {
        // Use async wait for sources
        const result = await finder.waitForSourcesAsync(timeout);
        return result.sources;
    } finally {
        finder.destroy();
    }
}

// Export everything
module.exports = {
    // Core functions
    initialize,
    destroy,
    isInitialized,
    version,
    find,
    
    // Classes
    Finder,
    Sender,
    Receiver,
    
    // Constants
    FourCC,
    FrameFormat,
    Bandwidth,
    ColorFormat,
    FrameType,
    
    // Native addon (for advanced use)
    native: ndiAddon
};
