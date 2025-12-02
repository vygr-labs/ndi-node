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
 * Basic test for ndi-node
 * 
 * This test verifies that the module can be loaded and basic functions work.
 * Note: Full testing requires the NDI SDK to be installed.
 */

'use strict';

console.log('=== NDI Node.js Bindings Test ===\n');

// Test 1: Module can be loaded
let ndi;
try {
    ndi = require('../lib');
    console.log('✓ Module loaded successfully');
} catch (e) {
    console.log('✗ Failed to load module:', e.message);
    console.log('\nNote: This is expected if the NDI SDK is not installed.');
    console.log('Please install the NDI SDK from https://ndi.video/download-ndi-sdk/');
    process.exit(1);
}

// Test 2: Constants are defined
console.log('\n--- Testing Constants ---');

const constantTests = [
    { name: 'FourCC.BGRA', value: ndi.FourCC?.BGRA, expected: 'BGRA' },
    { name: 'FourCC.RGBA', value: ndi.FourCC?.RGBA, expected: 'RGBA' },
    { name: 'FrameFormat.PROGRESSIVE', value: ndi.FrameFormat?.PROGRESSIVE, expected: 'progressive' },
    { name: 'Bandwidth.HIGHEST', value: ndi.Bandwidth?.HIGHEST, expected: 'highest' },
    { name: 'ColorFormat.BEST', value: ndi.ColorFormat?.BEST, expected: 'best' },
    { name: 'FrameType.VIDEO', value: ndi.FrameType?.VIDEO, expected: 'video' },
];

constantTests.forEach(test => {
    if (test.value === test.expected) {
        console.log(`✓ ${test.name} = '${test.value}'`);
    } else {
        console.log(`✗ ${test.name} expected '${test.expected}', got '${test.value}'`);
    }
});

// Test 3: Classes are defined
console.log('\n--- Testing Classes ---');

const classTests = ['Finder', 'Sender', 'Receiver'];
classTests.forEach(className => {
    if (typeof ndi[className] === 'function') {
        console.log(`✓ ${className} class exists`);
    } else {
        console.log(`✗ ${className} class missing`);
    }
});

// Test 4: Core functions are defined
console.log('\n--- Testing Core Functions ---');

const functionTests = ['initialize', 'destroy', 'isInitialized', 'version', 'find'];
functionTests.forEach(funcName => {
    if (typeof ndi[funcName] === 'function') {
        console.log(`✓ ${funcName}() function exists`);
    } else {
        console.log(`✗ ${funcName}() function missing`);
    }
});

// Test 5: Initialize and version (requires NDI SDK)
console.log('\n--- Testing NDI Initialization ---');

try {
    const initialized = ndi.initialize();
    console.log(`✓ initialize() returned: ${initialized}`);
    
    if (initialized) {
        const version = ndi.version();
        console.log(`✓ NDI Version: ${version}`);
        
        const isInit = ndi.isInitialized();
        console.log(`✓ isInitialized() returned: ${isInit}`);
        
        ndi.destroy();
        console.log('✓ destroy() completed');
        
        const isInitAfter = ndi.isInitialized();
        console.log(`✓ isInitialized() after destroy: ${isInitAfter}`);
    }
} catch (e) {
    console.log(`✗ NDI initialization failed: ${e.message}`);
    console.log('  This may be because the NDI runtime is not installed.');
}

console.log('\n=== Test Complete ===');
