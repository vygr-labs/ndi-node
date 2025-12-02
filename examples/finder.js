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
 * NDI Finder Example
 * 
 * This example demonstrates how to discover NDI sources on the network.
 */

const ndi = require('../lib');

// Initialize NDI
if (!ndi.initialize()) {
    console.error('Failed to initialize NDI');
    process.exit(1);
}

console.log('NDI Version:', ndi.version());
console.log('Searching for NDI sources...\n');

// Create a finder
const finder = new ndi.Finder({
    showLocalSources: true
});

// Poll for sources every second
finder.on('sources', (sources) => {
    console.log('\n--- Sources Updated ---');
    sources.forEach((source, index) => {
        console.log(`${index + 1}. ${source.name}`);
        if (source.urlAddress) {
            console.log(`   URL: ${source.urlAddress}`);
        }
    });
});

finder.startPolling(1000);

// Initial search
setTimeout(() => {
    const sources = finder.getSources();
    console.log(`Found ${sources.length} NDI source(s):`);
    sources.forEach((source, index) => {
        console.log(`${index + 1}. ${source.name}`);
    });
}, 2000);

// Cleanup on exit
process.on('SIGINT', () => {
    console.log('\nShutting down...');
    finder.destroy();
    ndi.destroy();
    process.exit(0);
});

console.log('Press Ctrl+C to exit\n');
