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
 * NDI Sender Example (Async)
 * 
 * This example demonstrates how to send video frames over NDI
 * using async/await for non-blocking operation.
 */

const ndi = require('../lib');

// Video settings
const WIDTH = 1920;
const HEIGHT = 1080;
const FRAME_RATE_N = 30000;
const FRAME_RATE_D = 1001;

let sender = null;
let running = true;

// Generate a test pattern frame (BGRA format)
function generateFrame(frameNumber) {
    const data = Buffer.alloc(WIDTH * HEIGHT * 4);
    
    // Create a simple gradient with moving bar
    const barPosition = (frameNumber * 10) % WIDTH;
    
    for (let y = 0; y < HEIGHT; y++) {
        for (let x = 0; x < WIDTH; x++) {
            const offset = (y * WIDTH + x) * 4;
            
            // Blue channel - horizontal gradient
            data[offset] = Math.floor((x / WIDTH) * 255);
            
            // Green channel - vertical gradient
            data[offset + 1] = Math.floor((y / HEIGHT) * 255);
            
            // Red channel - moving bar
            if (Math.abs(x - barPosition) < 20) {
                data[offset + 2] = 255;
            } else {
                data[offset + 2] = 0;
            }
            
            // Alpha channel
            data[offset + 3] = 255;
        }
    }
    
    return data;
}

// Sleep utility
function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

async function main() {
    // Initialize NDI
    if (!ndi.initialize()) {
        console.error('Failed to initialize NDI');
        process.exit(1);
    }

    console.log('NDI Version:', ndi.version());

    // Create sender
    sender = new ndi.Sender({
        name: 'NDI Test Source',
        clockVideo: true,
        clockAudio: false
    });

    console.log('Source name:', sender.getSourceName());
    console.log('Broadcasting NDI source...');
    console.log('Press Ctrl+C to exit\n');

    // Frame counter
    let frameNumber = 0;
    const frameInterval = (FRAME_RATE_D / FRAME_RATE_N) * 1000;

    // Main async loop
    while (running) {
        const frame = {
            xres: WIDTH,
            yres: HEIGHT,
            fourCC: ndi.FourCC.BGRA,
            frameRateN: FRAME_RATE_N,
            frameRateD: FRAME_RATE_D,
            frameFormatType: ndi.FrameFormat.PROGRESSIVE,
            data: generateFrame(frameNumber)
        };
        
        // Use async send - runs on background thread
        await sender.sendVideoPromise(frame);
        frameNumber++;
        
        // Log and check tally/connections every second (approximately 30 frames)
        if (frameNumber % 30 === 0) {
            // Use async methods for tally and connections
            const [tally, connections] = await Promise.all([
                sender.getTallyAsync(0),
                sender.getConnectionsAsync(0)
            ]);
            
            console.log(`Frame ${frameNumber}, Connections: ${connections}, Tally: ${JSON.stringify(tally)}`);
        }
        
        // Sleep to maintain frame rate
        await sleep(frameInterval);
    }
}

// Cleanup on exit
process.on('SIGINT', () => {
    console.log('\nShutting down...');
    running = false;
    if (sender) sender.destroy();
    ndi.destroy();
    process.exit(0);
});

// Run the async main function
main().catch(err => {
    console.error('Fatal error:', err);
    if (sender) sender.destroy();
    ndi.destroy();
    process.exit(1);
});
