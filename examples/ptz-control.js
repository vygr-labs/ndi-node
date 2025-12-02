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
 * NDI PTZ Control Example
 * 
 * This example demonstrates how to control PTZ (Pan-Tilt-Zoom) cameras over NDI.
 */

const ndi = require('../lib');
const readline = require('readline');

// Initialize NDI
if (!ndi.initialize()) {
    console.error('Failed to initialize NDI');
    process.exit(1);
}

console.log('NDI Version:', ndi.version());
console.log('Searching for NDI sources...');

// Find sources
const finder = new ndi.Finder();

setTimeout(() => {
    const sources = finder.getSources();
    
    if (sources.length === 0) {
        console.log('No NDI sources found.');
        finder.destroy();
        ndi.destroy();
        process.exit(1);
    }
    
    console.log(`\nFound ${sources.length} source(s):`);
    sources.forEach((source, index) => {
        console.log(`${index + 1}. ${source.name}`);
    });
    
    // Connect to first source
    const selectedSource = sources[0];
    console.log(`\nConnecting to: ${selectedSource.name}`);
    
    const receiver = new ndi.Receiver({
        source: selectedSource,
        bandwidth: ndi.Bandwidth.LOWEST, // We only need control, not video
        name: 'NDI PTZ Controller'
    });
    
    // Check PTZ support
    setTimeout(() => {
        const ptzSupported = receiver.ptzIsSupported();
        console.log(`\nPTZ Supported: ${ptzSupported}`);
        
        if (!ptzSupported) {
            console.log('This source does not support PTZ control.');
            console.log('(Note: PTZ support requires a PTZ-capable NDI source)');
        }
        
        console.log('\n=== PTZ Control Commands ===');
        console.log('w/s - Tilt up/down');
        console.log('a/d - Pan left/right');
        console.log('+/- - Zoom in/out');
        console.log('f   - Auto focus');
        console.log('1-5 - Recall preset 1-5');
        console.log('q   - Quit\n');
        
        // Set up keyboard input
        const rl = readline.createInterface({
            input: process.stdin,
            output: process.stdout
        });
        
        readline.emitKeypressEvents(process.stdin);
        if (process.stdin.isTTY) {
            process.stdin.setRawMode(true);
        }
        
        let zoom = 0.5;
        let pan = 0;
        let tilt = 0;
        
        process.stdin.on('keypress', (str, key) => {
            if (key.ctrl && key.name === 'c') {
                cleanup();
            }
            
            switch (str) {
                case 'w':
                    tilt = Math.min(1, tilt + 0.1);
                    receiver.ptzPanTilt(pan, tilt);
                    console.log(`Pan: ${pan.toFixed(2)}, Tilt: ${tilt.toFixed(2)}`);
                    break;
                    
                case 's':
                    tilt = Math.max(-1, tilt - 0.1);
                    receiver.ptzPanTilt(pan, tilt);
                    console.log(`Pan: ${pan.toFixed(2)}, Tilt: ${tilt.toFixed(2)}`);
                    break;
                    
                case 'a':
                    pan = Math.max(-1, pan - 0.1);
                    receiver.ptzPanTilt(pan, tilt);
                    console.log(`Pan: ${pan.toFixed(2)}, Tilt: ${tilt.toFixed(2)}`);
                    break;
                    
                case 'd':
                    pan = Math.min(1, pan + 0.1);
                    receiver.ptzPanTilt(pan, tilt);
                    console.log(`Pan: ${pan.toFixed(2)}, Tilt: ${tilt.toFixed(2)}`);
                    break;
                    
                case '+':
                case '=':
                    zoom = Math.min(1, zoom + 0.1);
                    receiver.ptzZoom(zoom);
                    console.log(`Zoom: ${zoom.toFixed(2)}`);
                    break;
                    
                case '-':
                    zoom = Math.max(0, zoom - 0.1);
                    receiver.ptzZoom(zoom);
                    console.log(`Zoom: ${zoom.toFixed(2)}`);
                    break;
                    
                case 'f':
                    receiver.ptzAutoFocus();
                    console.log('Auto focus activated');
                    break;
                    
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                    const preset = parseInt(str);
                    receiver.ptzRecallPreset(preset - 1, 1.0);
                    console.log(`Recalled preset ${preset}`);
                    break;
                    
                case 'q':
                    cleanup();
                    break;
            }
        });
        
        function cleanup() {
            console.log('\nShutting down...');
            rl.close();
            receiver.destroy();
            finder.destroy();
            ndi.destroy();
            process.exit(0);
        }
        
    }, 1000);
    
}, 3000);
