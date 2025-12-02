# NDI SDK Setup Instructions

This directory should contain the NDI SDK files.

## Download NDI SDK

1. Visit https://ndi.video/download-ndi-sdk/
2. Register/sign in to download
3. Download the NDI SDK for your platform:
   - Windows: NDI SDK for Windows
   - macOS: NDI SDK for macOS
   - Linux: NDI SDK for Linux

## Install SDK Files

After downloading, extract and copy the necessary files:

### Windows

Copy from the installed SDK location (usually `C:\Program Files\NDI\NDI 5 SDK`):

```
deps/ndi/
├── include/
│   └── Processing.NDI.Lib.h     (from SDK include folder)
└── lib/
    └── x64/
        ├── Processing.NDI.Lib.x64.lib    (from SDK lib/x64 folder)
        └── Processing.NDI.Lib.x64.dll    (from SDK bin/x64 folder)
```

### macOS

```
deps/ndi/
├── include/
│   └── Processing.NDI.Lib.h
└── lib/
    └── libndi.dylib
```

### Linux

```
deps/ndi/
├── include/
│   └── Processing.NDI.Lib.h
└── lib/
    └── libndi.so
```

## NDI Runtime

Users of your application will also need the NDI runtime installed:
- https://ndi.video/tools/

## Important Notes

1. The NDI SDK is subject to NDI's license agreement
2. Do not redistribute NDI SDK files without proper licensing
3. The header file provided in this project is a mock for compilation reference
4. Replace it with the actual SDK header for full functionality
