# ndi-node

Node.js bindings for NDI (Network Device Interface) SDK.

NDI is a royalty-free software specification developed by NewTek that enables video-compatible products to communicate, deliver, and receive high-quality video over IP networks.

## Features

- **NDI Source Discovery** - Find NDI sources on your network
- **NDI Sender** - Broadcast video and audio as an NDI source
- **NDI Receiver** - Receive video and audio from NDI sources
- **PTZ Control** - Control PTZ cameras over NDI
- **Tally Support** - Send and receive tally information
- **Metadata** - Exchange XML metadata with NDI sources
- **Async/Await Support** - Non-blocking async methods for all operations
- **TypeScript Support** - Full TypeScript type definitions included

## Prerequisites

### NDI SDK

You need to download and install the NDI SDK from NewTek:

1. Visit https://ndi.video/download-ndi-sdk/
2. Download the NDI SDK for your platform
3. Copy the SDK files to the `deps/ndi` directory:

```
deps/ndi/
├── include/
│   └── Processing.NDI.Lib.h
└── lib/
    └── x64/  (Windows)
        ├── Processing.NDI.Lib.x64.lib
        └── Processing.NDI.Lib.x64.dll
```

### Build Tools

- **Windows**: Visual Studio Build Tools with C++ workload
- **macOS**: Xcode Command Line Tools
- **Linux**: GCC/G++ and build-essential

## Installation

```bash
npm install @vygr-labs/ndi-node
```

Or with yarn:

```bash
yarn add @vygr-labs/ndi-node
```

Or with pnpm:

```bash
pnpm add @vygr-labs/ndi-node
```

This will compile the native addon using node-gyp. After installation, a post-install script will check for the NDI SDK and provide setup instructions if needed.

## Quick Start

### Finding NDI Sources

```javascript
const ndi = require('@vygr-labs/ndi-node');

async function main() {
    // Initialize NDI
    ndi.initialize();

    // Create a finder
    const finder = new ndi.Finder({ showLocalSources: true });

    // Find sources asynchronously (non-blocking)
    const sources = await finder.getSourcesAsync();
    console.log('Found sources:', sources);

    // Or wait for sources to appear
    const result = await finder.waitForSourcesAsync(5000);
    console.log('Sources changed:', result.changed);
    console.log('Sources:', result.sources);

    // Cleanup
    finder.destroy();
    ndi.destroy();
}

main();
```

### Sending Video

```javascript
const ndi = require('@vygr-labs/ndi-node');

async function main() {
    ndi.initialize();

    const sender = new ndi.Sender({
        name: 'My NDI Source',
        clockVideo: true
    });

    // Send a frame asynchronously (non-blocking, runs on background thread)
    await sender.sendVideoPromise({
        xres: 1920,
        yres: 1080,
        fourCC: ndi.FourCC.BGRA,
        frameRateN: 30000,
        frameRateD: 1001,
        data: frameBuffer // Buffer containing BGRA pixel data
    });

    // Check tally and connections asynchronously
    const [tally, connections] = await Promise.all([
        sender.getTallyAsync(),
        sender.getConnectionsAsync()
    ]);
    console.log('Tally:', tally);
    console.log('Connections:', connections);

    // Cleanup
    sender.destroy();
    ndi.destroy();
}

main();
```

### Receiving Video

```javascript
const ndi = require('@vygr-labs/ndi-node');

async function main() {
    ndi.initialize();

    // Find sources first
    const finder = new ndi.Finder();
    const result = await finder.waitForSourcesAsync(5000);
    
    if (result.sources.length === 0) {
        console.log('No sources found');
        return;
    }

    // Create receiver and connect to first source
    const receiver = new ndi.Receiver({
        source: result.sources[0],
        colorFormat: ndi.ColorFormat.BGRX_BGRA,
        bandwidth: ndi.Bandwidth.HIGHEST
    });

    // Capture frames asynchronously (non-blocking)
    while (true) {
        const frame = await receiver.captureAsync(1000);
        
        if (frame.type === 'video') {
            console.log(`Video: ${frame.video.xres}x${frame.video.yres}`);
            // frame.video.data contains the pixel buffer
        } else if (frame.type === 'audio') {
            console.log(`Audio: ${frame.audio.noSamples} samples`);
        }
    }

    // Cleanup
    receiver.destroy();
    finder.destroy();
    ndi.destroy();
}

main();
```

## API Reference

### Core Functions

#### `ndi.initialize(): boolean`
Initialize the NDI library. Must be called before using any other functions.

#### `ndi.destroy(): void`
Cleanup the NDI library. Should be called when done using NDI.

#### `ndi.version(): string | null`
Get the NDI library version string.

#### `ndi.find(timeout?, options?): Promise<Source[]>`
Find NDI sources on the network.

### Finder Class

```javascript
new ndi.Finder(options?)
```

Options:
- `showLocalSources: boolean` - Include local sources (default: true)
- `groups: string` - Comma-separated list of groups to search
- `extraIps: string` - Extra IPs to search for sources

Methods:
- `getSources(): Source[]` - Get currently discovered sources (sync)
- `getSourcesAsync(): Promise<Source[]>` - Get sources asynchronously (non-blocking)
- `waitForSources(timeout?): boolean` - Wait for sources to change (sync)
- `waitForSourcesAsync(timeout?): Promise<{changed, sources}>` - Wait for sources asynchronously (non-blocking)
- `startPolling(interval?)` - Start polling for sources
- `stopPolling()` - Stop polling
- `destroy()` - Release resources

Events:
- `'sources'` - Emitted when sources change (when using polling)

### Sender Class

```javascript
new ndi.Sender(options)
```

Options:
- `name: string` - Name of the NDI source (required)
- `groups: string` - Comma-separated list of groups
- `clockVideo: boolean` - Clock video to frame rate (default: true)
- `clockAudio: boolean` - Clock audio to sample rate (default: true)

Methods:
- `sendVideo(frame)` - Send a video frame (sync)
- `sendVideoAsync(frame)` - Send a video frame using NDI async API
- `sendVideoPromise(frame): Promise<void>` - Send a video frame on background thread (non-blocking)
- `sendAudio(frame)` - Send an audio frame (sync)
- `sendAudioPromise(frame): Promise<void>` - Send an audio frame on background thread (non-blocking)
- `sendMetadata(frame)` - Send metadata
- `getTally(timeout?): Tally | null` - Get tally state (sync)
- `getTallyAsync(timeout?): Promise<Tally | null>` - Get tally state asynchronously (non-blocking)
- `setTally(tally)` - Set tally state
- `getConnections(timeout?): number` - Get number of connections (sync)
- `getConnectionsAsync(timeout?): Promise<number>` - Get connections asynchronously (non-blocking)
- `getSourceName(): string | null` - Get full source name
- `startTallyPolling(interval?)` - Start polling for tally changes
- `stopTallyPolling()` - Stop tally polling
- `destroy()` - Release resources

Events:
- `'tally'` - Emitted when tally state changes (when using polling)

### Receiver Class

```javascript
new ndi.Receiver(options?)
```

Options:
- `source: Source` - Source to connect to
- `colorFormat: string` - Color format (default: 'BGRX_BGRA')
- `bandwidth: string` - Bandwidth mode (default: 'highest')
- `allowVideoFields: boolean` - Allow video fields (default: true)
- `name: string` - Receiver name

Methods:
- `connect(source)` - Connect to a source
- `capture(timeout?): CaptureResult` - Capture any frame type (sync)
- `captureAsync(timeout?): Promise<CaptureResult>` - Capture any frame type asynchronously (non-blocking)
- `captureVideo(timeout?): VideoFrame | null` - Capture video only (sync)
- `captureVideoAsync(timeout?): Promise<VideoFrame | null>` - Capture video asynchronously (non-blocking)
- `captureAudio(timeout?): AudioFrame | null` - Capture audio only (sync)
- `captureAudioAsync(timeout?): Promise<AudioFrame | null>` - Capture audio asynchronously (non-blocking)
- `setTally(tally): boolean` - Set tally information
- `sendMetadata(frame)` - Send metadata to source
- `destroy()` - Release resources

PTZ Methods:
- `ptzIsSupported(): boolean`
- `ptzZoom(zoom): boolean`
- `ptzPanTilt(pan, tilt): boolean`
- `ptzPanTiltSpeed(panSpeed, tiltSpeed): boolean`
- `ptzStorePreset(presetNo): boolean`
- `ptzRecallPreset(presetNo, speed?): boolean`
- `ptzAutoFocus(): boolean`
- `ptzFocus(focus): boolean`
- `ptzWhiteBalanceAuto(): boolean`
- `ptzExposureAuto(): boolean`
- And more...

Events:
- `'video'` - Emitted when video frame is received
- `'audio'` - Emitted when audio frame is received
- `'metadata'` - Emitted when metadata is received
- `'status_change'` - Emitted when connection status changes
- `'error'` - Emitted on receive error

### Constants

```javascript
// Video pixel formats
ndi.FourCC.BGRA
ndi.FourCC.RGBA
ndi.FourCC.UYVY
ndi.FourCC.I420
ndi.FourCC.NV12
// ... and more

// Frame format types
ndi.FrameFormat.PROGRESSIVE
ndi.FrameFormat.INTERLEAVED
ndi.FrameFormat.FIELD_0
ndi.FrameFormat.FIELD_1

// Bandwidth modes
ndi.Bandwidth.HIGHEST
ndi.Bandwidth.LOWEST
ndi.Bandwidth.AUDIO_ONLY
ndi.Bandwidth.METADATA_ONLY

// Color formats
ndi.ColorFormat.BGRX_BGRA
ndi.ColorFormat.UYVY_BGRA
ndi.ColorFormat.RGBX_RGBA
ndi.ColorFormat.FASTEST
ndi.ColorFormat.BEST

// Frame types
ndi.FrameType.VIDEO
ndi.FrameType.AUDIO
ndi.FrameType.METADATA
ndi.FrameType.ERROR
ndi.FrameType.STATUS_CHANGE
```

## Examples

See the `examples/` directory for complete examples using async/await:

- `finder.js` - Discover NDI sources asynchronously
- `sender.js` - Send video test pattern with async frame sending
- `receiver.js` - Receive video/audio with async capture loop

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Credits

NDI® is a registered trademark of NewTek, Inc.
