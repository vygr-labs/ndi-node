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
 * NDI Sender Example
 * 
 * This example demonstrates how to send video frames over NDI.
 * It generates a simple test pattern and broadcasts it as an NDI source.
 */

const ndi = require('../lib');

// Initialize NDI
if (!ndi.initialize()) {
    console.error('Failed to initialize NDI');
    process.exit(1);
}

console.log('NDI Version:', ndi.version());

// Video settings
const WIDTH = 1920;
const HEIGHT = 1080;
const FRAME_RATE_N = 30000;
const FRAME_RATE_D = 1001;

// Create sender
const sender = new ndi.Sender({
    name: 'NDI Test Source',
    clockVideo: true,
    clockAudio: false
});

console.log('Source name:', sender.getSourceName());
console.log('Broadcasting NDI source...');

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

// Frame counter
let frameNumber = 0;

// Send frames at the specified frame rate
const frameInterval = (FRAME_RATE_D / FRAME_RATE_N) * 1000;

const sendFrame = () => {
    const frame = {
        xres: WIDTH,
        yres: HEIGHT,
        fourCC: ndi.FourCC.BGRA,
        frameRateN: FRAME_RATE_N,
        frameRateD: FRAME_RATE_D,
        frameFormatType: ndi.FrameFormat.PROGRESSIVE,
        data: generateFrame(frameNumber)
    };
    
    sender.sendVideo(frame);
    frameNumber++;
    
    // Log every 30 frames (approximately 1 second)
    if (frameNumber % 30 === 0) {
        const connections = sender.getConnections();
        console.log(`Frame ${frameNumber}, Connections: ${connections}`);
    }
};

// Start sending frames
const intervalId = setInterval(sendFrame, frameInterval);

// Monitor tally state
sender.on('tally', (tally) => {
    console.log('Tally changed:', tally);
});

sender.startTallyPolling(100);

// Cleanup on exit
process.on('SIGINT', () => {
    console.log('\nShutting down...');
    clearInterval(intervalId);
    sender.destroy();
    ndi.destroy();
    process.exit(0);
});

console.log('Press Ctrl+C to exit\n');
