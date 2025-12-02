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
 * NDI Audio Sender Example
 * 
 * This example demonstrates how to send audio over NDI.
 * It generates a sine wave tone and broadcasts it as an NDI source.
 */

const ndi = require('../lib');

// Initialize NDI
if (!ndi.initialize()) {
    console.error('Failed to initialize NDI');
    process.exit(1);
}

console.log('NDI Version:', ndi.version());

// Audio settings
const SAMPLE_RATE = 48000;
const NUM_CHANNELS = 2;
const SAMPLES_PER_FRAME = 1600; // ~33ms of audio at 48kHz
const FREQUENCY = 440; // A4 note

// Create sender
const sender = new ndi.Sender({
    name: 'NDI Audio Test',
    clockVideo: false,
    clockAudio: true
});

console.log('Source name:', sender.getSourceName());
console.log('Broadcasting NDI audio...');

// Phase for sine wave generation
let phase = 0;

// Generate audio samples (planar float format)
function generateAudio() {
    // Create buffer for planar audio (channel 1 samples, then channel 2 samples)
    const data = Buffer.alloc(NUM_CHANNELS * SAMPLES_PER_FRAME * 4); // float32 = 4 bytes
    
    for (let i = 0; i < SAMPLES_PER_FRAME; i++) {
        // Generate sine wave sample
        const sample = Math.sin(phase) * 0.5; // 50% amplitude
        
        // Write to both channels (planar format)
        data.writeFloatLE(sample, i * 4);                                    // Channel 1
        data.writeFloatLE(sample, (SAMPLES_PER_FRAME + i) * 4);             // Channel 2
        
        // Advance phase
        phase += (2 * Math.PI * FREQUENCY) / SAMPLE_RATE;
        if (phase > 2 * Math.PI) {
            phase -= 2 * Math.PI;
        }
    }
    
    return data;
}

// Frame counter
let frameNumber = 0;

// Send audio at regular intervals
const audioInterval = (SAMPLES_PER_FRAME / SAMPLE_RATE) * 1000;

const sendAudio = () => {
    const frame = {
        sampleRate: SAMPLE_RATE,
        noChannels: NUM_CHANNELS,
        noSamples: SAMPLES_PER_FRAME,
        channelStrideInBytes: SAMPLES_PER_FRAME * 4, // float32 = 4 bytes
        data: generateAudio()
    };
    
    sender.sendAudio(frame);
    frameNumber++;
    
    // Log every ~1 second (30 frames at ~33ms each)
    if (frameNumber % 30 === 0) {
        const connections = sender.getConnections();
        console.log(`Audio frames sent: ${frameNumber}, Connections: ${connections}`);
    }
};

// Start sending audio
const intervalId = setInterval(sendAudio, audioInterval);

// Cleanup on exit
process.on('SIGINT', () => {
    console.log('\nShutting down...');
    clearInterval(intervalId);
    sender.destroy();
    ndi.destroy();
    process.exit(0);
});

console.log(`Generating ${FREQUENCY}Hz sine wave`);
console.log('Press Ctrl+C to exit\n');
