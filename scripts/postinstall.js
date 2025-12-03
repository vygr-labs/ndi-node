#!/usr/bin/env node

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
 * Post-install script for ndi-node
 * 
 * This script checks for NDI SDK installation and provides guidance to users.
 */

'use strict';

const fs = require('fs');
const path = require('path');
const os = require('os');

// ANSI color codes
const colors = {
  reset: '\x1b[0m',
  bright: '\x1b[1m',
  red: '\x1b[31m',
  green: '\x1b[32m',
  yellow: '\x1b[33m',
  blue: '\x1b[34m',
  cyan: '\x1b[36m'
};

function log(message, color = '') {
  console.log(`${color}${message}${colors.reset}`);
}

function logHeader(message) {
  console.log();
  log('═'.repeat(60), colors.cyan);
  log(`  ${message}`, colors.bright + colors.cyan);
  log('═'.repeat(60), colors.cyan);
  console.log();
}

function logSuccess(message) {
  log(`✓ ${message}`, colors.green);
}

function logWarning(message) {
  log(`⚠ ${message}`, colors.yellow);
}

function logError(message) {
  log(`✗ ${message}`, colors.red);
}

function logInfo(message) {
  log(`  ${message}`, colors.blue);
}

/**
 * Check for NDI SDK installation on different platforms
 */
function findNdiSdk() {
  const platform = os.platform();
  const possiblePaths = [];

  if (platform === 'win32') {
    // Windows: Check common installation paths
    const programFiles = process.env['ProgramFiles'] || 'C:\\Program Files';
    const programFilesX86 = process.env['ProgramFiles(x86)'] || 'C:\\Program Files (x86)';
    
    // Check for different NDI SDK versions
    for (let version = 6; version >= 4; version--) {
      possiblePaths.push(path.join(programFiles, 'NDI', `NDI ${version} SDK`));
      possiblePaths.push(path.join(programFilesX86, 'NDI', `NDI ${version} SDK`));
    }
    
    // Also check NDI_SDK_DIR environment variable
    if (process.env.NDI_SDK_DIR) {
      possiblePaths.unshift(process.env.NDI_SDK_DIR);
    }
  } else if (platform === 'darwin') {
    // macOS
    possiblePaths.push('/Library/NDI SDK for Apple');
    possiblePaths.push('/Library/NDI SDK for macOS');
    possiblePaths.push(path.join(os.homedir(), 'Library', 'NDI SDK for Apple'));
    
    if (process.env.NDI_SDK_DIR) {
      possiblePaths.unshift(process.env.NDI_SDK_DIR);
    }
  } else if (platform === 'linux') {
    // Linux
    possiblePaths.push('/usr/local/NDI SDK for Linux');
    possiblePaths.push('/opt/ndi-sdk');
    possiblePaths.push(path.join(os.homedir(), 'NDI SDK for Linux'));
    
    if (process.env.NDI_SDK_DIR) {
      possiblePaths.unshift(process.env.NDI_SDK_DIR);
    }
  }

  // Check each path
  for (const sdkPath of possiblePaths) {
    if (fs.existsSync(sdkPath)) {
      return { found: true, path: sdkPath };
    }
  }

  return { found: false, searchedPaths: possiblePaths };
}

/**
 * Check if the NDI runtime is available
 */
function checkNdiRuntime() {
  const platform = os.platform();
  
  if (platform === 'win32') {
    // Check for NDI runtime in various locations
    const systemRoot = process.env.SystemRoot || 'C:\\Windows';
    const programFiles = process.env['ProgramFiles'] || 'C:\\Program Files';
    const programFilesX86 = process.env['ProgramFiles(x86)'] || 'C:\\Program Files (x86)';
    
    const dllPaths = [];
    
    // Check SDK Bin folders (where the DLL actually lives)
    for (let version = 6; version >= 4; version--) {
      dllPaths.push(path.join(programFiles, 'NDI', `NDI ${version} SDK`, 'Bin', 'x64', 'Processing.NDI.Lib.x64.dll'));
      dllPaths.push(path.join(programFiles, 'NDI', `NDI ${version} SDK`, 'Bin', 'x86', 'Processing.NDI.Lib.x86.dll'));
      dllPaths.push(path.join(programFilesX86, 'NDI', `NDI ${version} SDK`, 'Bin', 'x64', 'Processing.NDI.Lib.x64.dll'));
      dllPaths.push(path.join(programFilesX86, 'NDI', `NDI ${version} SDK`, 'Bin', 'x86', 'Processing.NDI.Lib.x86.dll'));
      // Also check for UWP variants
      dllPaths.push(path.join(programFiles, 'NDI', `NDI ${version} SDK`, 'Bin', 'x64', 'Processing.NDI.Lib.UWP.x64.dll'));
      dllPaths.push(path.join(programFiles, 'NDI', `NDI ${version} SDK`, 'Bin', 'x86', 'Processing.NDI.Lib.UWP.x86.dll'));
    }
    
    // Check NDI Tools installation
    for (let version = 6; version >= 4; version--) {
      dllPaths.push(path.join(programFiles, 'NDI', `NDI ${version} Tools`, 'Processing.NDI.Lib.x64.dll'));
    }
    
    // Check System32/SysWOW64
    dllPaths.push(path.join(systemRoot, 'System32', 'Processing.NDI.Lib.x64.dll'));
    dllPaths.push(path.join(systemRoot, 'SysWOW64', 'Processing.NDI.Lib.x86.dll'));
    
    for (const dllPath of dllPaths) {
      if (fs.existsSync(dllPath)) {
        return { found: true, path: dllPath };
      }
    }
  } else if (platform === 'darwin') {
    const dylibPaths = [
      '/usr/local/lib/libndi.dylib',
      '/Library/NDI SDK for Apple/lib/macOS/libndi.dylib'
    ];
    
    for (const dylibPath of dylibPaths) {
      if (fs.existsSync(dylibPath)) {
        return { found: true, path: dylibPath };
      }
    }
  } else if (platform === 'linux') {
    const soPaths = [
      '/usr/local/lib/libndi.so',
      '/usr/lib/libndi.so',
      '/opt/ndi-sdk/lib/x86_64-linux-gnu/libndi.so'
    ];
    
    for (const soPath of soPaths) {
      if (fs.existsSync(soPath)) {
        return { found: true, path: soPath };
      }
    }
  }

  return { found: false };
}

/**
 * Get platform-specific setup instructions
 */
function getSetupInstructions() {
  const platform = os.platform();
  
  if (platform === 'win32') {
    return `
  ${colors.bright}Windows Setup Instructions:${colors.reset}

  1. Download the NDI SDK from: ${colors.cyan}https://ndi.video/tools/${colors.reset}
     (Look for "NDI SDK" in the Developer section)

  2. Run the installer and note the installation path
     (Default: C:\\Program Files\\NDI\\NDI 6 SDK)

  3. Set the NDI_SDK_DIR environment variable (optional):
     ${colors.yellow}setx NDI_SDK_DIR "C:\\Program Files\\NDI\\NDI 6 SDK"${colors.reset}

  4. Copy the NDI runtime DLL to your project's build folder:
     ${colors.yellow}copy "C:\\Program Files\\NDI\\NDI 6 SDK\\Bin\\x64\\Processing.NDI.Lib.x64.dll" .\\build\\Release\\${colors.reset}

  5. Or install NDI Tools (includes the runtime):
     ${colors.cyan}https://ndi.video/tools/${colors.reset}
`;
  } else if (platform === 'darwin') {
    return `
  ${colors.bright}macOS Setup Instructions:${colors.reset}

  1. Download the NDI SDK from: ${colors.cyan}https://ndi.video/tools/${colors.reset}
     (Look for "NDI SDK for Apple")

  2. Run the installer package

  3. Set the NDI_SDK_DIR environment variable:
     ${colors.yellow}export NDI_SDK_DIR="/Library/NDI SDK for Apple"${colors.reset}

  4. Add the NDI library to your library path:
     ${colors.yellow}export DYLD_LIBRARY_PATH="/Library/NDI SDK for Apple/lib/macOS:$DYLD_LIBRARY_PATH"${colors.reset}

  5. Add these to your ~/.zshrc or ~/.bash_profile for persistence
`;
  } else {
    return `
  ${colors.bright}Linux Setup Instructions:${colors.reset}

  1. Download the NDI SDK from: ${colors.cyan}https://ndi.video/tools/${colors.reset}
     (Look for "NDI SDK for Linux")

  2. Extract the archive and run the install script:
     ${colors.yellow}tar -xzf InstallNDISDK_v6_Linux.tar.gz${colors.reset}
     ${colors.yellow}./InstallNDISDK_v6_Linux.sh${colors.reset}

  3. Set the NDI_SDK_DIR environment variable:
     ${colors.yellow}export NDI_SDK_DIR="/usr/local/NDI SDK for Linux"${colors.reset}

  4. Add the NDI library to your library path:
     ${colors.yellow}export LD_LIBRARY_PATH="/usr/local/NDI SDK for Linux/lib/x86_64-linux-gnu:$LD_LIBRARY_PATH"${colors.reset}

  5. Add these to your ~/.bashrc for persistence
`;
  }
}

/**
 * Main postinstall check
 */
function main() {
  logHeader('NDI Node.js Bindings - Post-Install Check');

  // Check for NDI SDK
  log('Checking for NDI SDK installation...', colors.bright);
  const sdkResult = findNdiSdk();
  
  if (sdkResult.found) {
    logSuccess(`NDI SDK found at: ${sdkResult.path}`);
  } else {
    logWarning('NDI SDK not found in standard locations');
  }

  // Check for NDI runtime
  console.log();
  log('Checking for NDI runtime...', colors.bright);
  const runtimeResult = checkNdiRuntime();
  
  if (runtimeResult.found) {
    logSuccess(`NDI runtime found at: ${runtimeResult.path}`);
  } else {
    logWarning('NDI runtime not found');
  }

  // Show status summary
  console.log();
  log('─'.repeat(60), colors.cyan);
  
  if (sdkResult.found && runtimeResult.found) {
    logSuccess('All NDI components found! You should be ready to go.');
    console.log();
    logInfo('Try running the finder example:');
    logInfo('  node examples/finder.js');
  } else {
    logError('Some NDI components are missing.');
    console.log();
    log('The NDI SDK is required to build and run ndi-node.', colors.yellow);
    log('Please follow the setup instructions below:', colors.yellow);
    console.log(getSetupInstructions());
    
    log('After installing the SDK, rebuild the module:', colors.bright);
    logInfo('  npm rebuild ndi-node');
    console.log();
    
    log('For more information, see:', colors.bright);
    logInfo('  https://github.com/CodeKing12/ndi-node#readme');
  }

  log('─'.repeat(60), colors.cyan);
  console.log();
}

// Run the postinstall check
main();
