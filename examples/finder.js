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
 * NDI Finder Example (Async)
 * 
 * This example demonstrates how to discover NDI sources on the network
 * using async/await for non-blocking operation.
 */

const ndi = require('../lib');

let finder = null;
let running = true;

async function main() {
    // Initialize NDI
    if (!ndi.initialize()) {
        console.error('Failed to initialize NDI');
        process.exit(1);
    }

    console.log('NDI Version:', ndi.version());
    console.log('Searching for NDI sources...\n');

    // Create a finder
    finder = new ndi.Finder({
        showLocalSources: true
    });

    // Initial search with async method
    console.log('Performing initial source discovery...');
    const initialSources = await finder.getSourcesAsync();
    console.log(`Found ${initialSources.length} NDI source(s):`);
    initialSources.forEach((source, index) => {
        console.log(`  ${index + 1}. ${source.name}`);
        if (source.urlAddress) {
            console.log(`     URL: ${source.urlAddress}`);
        }
    });

    console.log('\nContinuously monitoring for source changes...');
    console.log('Press Ctrl+C to exit\n');

    // Continuous async polling loop
    let lastSourcesJson = JSON.stringify(initialSources);
    
    while (running) {
        try {
            // Wait for sources to change (async, non-blocking)
            const result = await finder.waitForSourcesAsync(1000);
            
            if (result.changed) {
                const sources = result.sources;
                const sourcesJson = JSON.stringify(sources);
                
                if (sourcesJson !== lastSourcesJson) {
                    lastSourcesJson = sourcesJson;
                    console.log('\n--- Sources Updated ---');
                    console.log(`Found ${sources.length} source(s):`);
                    sources.forEach((source, index) => {
                        console.log(`  ${index + 1}. ${source.name}`);
                        if (source.urlAddress) {
                            console.log(`     URL: ${source.urlAddress}`);
                        }
                    });
                }
            }
        } catch (err) {
            console.error('Error during source discovery:', err.message);
        }
    }
}

// Cleanup on exit
process.on('SIGINT', () => {
    console.log('\nShutting down...');
    running = false;
    if (finder) finder.destroy();
    ndi.destroy();
    process.exit(0);
});

// Run the async main function
main().catch(err => {
    console.error('Fatal error:', err);
    if (finder) finder.destroy();
    ndi.destroy();
    process.exit(1);
});
