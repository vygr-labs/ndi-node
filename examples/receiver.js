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
 * NDI Receiver Example (Async)
 * 
 * This example demonstrates how to receive video from an NDI source
 * using async/await for non-blocking operation.
 */

const ndi = require('../lib');

let finder = null;
let receiver = null;
let running = true;

async function main() {
    // Initialize NDI
    if (!ndi.initialize()) {
        console.error('Failed to initialize NDI');
        process.exit(1);
    }

    console.log('NDI Version:', ndi.version());
    console.log('Searching for NDI sources...');

    // Find sources first
    finder = new ndi.Finder();

    // Wait for sources using async method
    console.log('Waiting for sources to appear...');
    let sources = [];
    
    // Try to find sources for up to 5 seconds
    for (let i = 0; i < 5 && sources.length === 0; i++) {
        const result = await finder.waitForSourcesAsync(1000);
        sources = result.sources;
        if (sources.length === 0) {
            console.log(`  Searching... (${i + 1}/5)`);
        }
    }
    
    if (sources.length === 0) {
        console.log('No NDI sources found. Make sure an NDI source is running.');
        finder.destroy();
        ndi.destroy();
        process.exit(1);
    }
    
    console.log(`\nFound ${sources.length} source(s):`);
    sources.forEach((source, index) => {
        console.log(`  ${index + 1}. ${source.name}`);
    });
    
    // Connect to the first source
    const selectedSource = sources[0];
    console.log(`\nConnecting to: ${selectedSource.name}`);
    
    // Create receiver
    receiver = new ndi.Receiver({
        source: selectedSource,
        colorFormat: ndi.ColorFormat.BGRX_BGRA,
        bandwidth: ndi.Bandwidth.HIGHEST,
        allowVideoFields: true,
        name: 'NDI Node.js Receiver'
    });
    
    // Set tally to indicate we're receiving
    receiver.setTally({ onProgram: true, onPreview: false });
    
    console.log('Receiving... Press Ctrl+C to exit\n');
    
    // Frame counters
    let videoFrames = 0;
    let audioFrames = 0;
    let lastReport = Date.now();
    
    // Main async capture loop
    while (running) {
        try {
            // Capture frame asynchronously (non-blocking)
            const result = await receiver.captureAsync(100);
            
            switch (result.type) {
                case 'video':
                    videoFrames++;
                    break;
                    
                case 'audio':
                    audioFrames++;
                    break;
                    
                case 'metadata':
                    if (result.metadata) {
                        console.log('Metadata received:', result.metadata.data);
                    }
                    break;
                    
                case 'status_change':
                    console.log('Connection status changed');
                    break;
                    
                case 'error':
                    console.error('Receive error:', result.error);
                    break;
            }
            
            // Report stats every second
            const now = Date.now();
            if (now - lastReport >= 1000) {
                if (videoFrames > 0 || audioFrames > 0) {
                    const videoInfo = result.video 
                        ? `Resolution: ${result.video.xres}x${result.video.yres}, Format: ${result.video.fourCC}`
                        : '';
                    console.log(`Video: ${videoFrames} fps, Audio: ${audioFrames} fps ${videoInfo}`);
                }
                videoFrames = 0;
                audioFrames = 0;
                lastReport = now;
            }
        } catch (err) {
            console.error('Capture error:', err.message);
        }
    }
}

// Cleanup on exit
process.on('SIGINT', () => {
    console.log('\nShutting down...');
    running = false;
    if (receiver) receiver.destroy();
    if (finder) finder.destroy();
    ndi.destroy();
    process.exit(0);
});

// Run the async main function
main().catch(err => {
    console.error('Fatal error:', err);
    if (receiver) receiver.destroy();
    if (finder) finder.destroy();
    ndi.destroy();
    process.exit(1);
});
