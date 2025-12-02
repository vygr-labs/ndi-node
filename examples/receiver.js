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
 * NDI Receiver Example
 * 
 * This example demonstrates how to receive video from an NDI source.
 * It finds available sources and connects to the first one.
 */

const ndi = require('../lib');

// Initialize NDI
if (!ndi.initialize()) {
    console.error('Failed to initialize NDI');
    process.exit(1);
}

console.log('NDI Version:', ndi.version());
console.log('Searching for NDI sources...');

// Find sources first
const finder = new ndi.Finder();

// Wait for sources to appear
setTimeout(() => {
    const sources = finder.getSources();
    
    if (sources.length === 0) {
        console.log('No NDI sources found. Make sure an NDI source is running.');
        finder.destroy();
        ndi.destroy();
        process.exit(1);
    }
    
    console.log(`Found ${sources.length} source(s):`);
    sources.forEach((source, index) => {
        console.log(`${index + 1}. ${source.name}`);
    });
    
    // Connect to the first source
    const selectedSource = sources[0];
    console.log(`\nConnecting to: ${selectedSource.name}`);
    
    // Create receiver
    const receiver = new ndi.Receiver({
        source: selectedSource,
        colorFormat: ndi.ColorFormat.BGRX_BGRA,
        bandwidth: ndi.Bandwidth.HIGHEST,
        allowVideoFields: true,
        name: 'NDI Node.js Receiver'
    });
    
    // Set tally to indicate we're receiving
    receiver.setTally({ onProgram: true, onPreview: false });
    
    // Frame counters
    let videoFrames = 0;
    let audioFrames = 0;
    let lastReport = Date.now();
    
    // Handle video frames
    receiver.on('video', (frame) => {
        videoFrames++;
        
        // Report stats every second
        const now = Date.now();
        if (now - lastReport >= 1000) {
            console.log(`Video: ${videoFrames} fps, Audio: ${audioFrames} fps, ` +
                        `Resolution: ${frame.xres}x${frame.yres}, Format: ${frame.fourCC}`);
            videoFrames = 0;
            audioFrames = 0;
            lastReport = now;
        }
    });
    
    // Handle audio frames
    receiver.on('audio', (frame) => {
        audioFrames++;
    });
    
    // Handle metadata
    receiver.on('metadata', (frame) => {
        console.log('Metadata received:', frame.data);
    });
    
    // Handle status changes
    receiver.on('status_change', () => {
        console.log('Connection status changed');
    });
    
    // Handle errors
    receiver.on('error', (error) => {
        console.error('Receive error:', error);
    });
    
    // Start capturing
    receiver.startCapture(100);
    
    console.log('Receiving... Press Ctrl+C to exit\n');
    
    // Cleanup on exit
    process.on('SIGINT', () => {
        console.log('\nShutting down...');
        receiver.destroy();
        finder.destroy();
        ndi.destroy();
        process.exit(0);
    });
    
}, 3000);
