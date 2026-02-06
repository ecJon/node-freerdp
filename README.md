Node-FreeRDP
============

Node.js addon for FreeRDP 3.x - Connect to RDP servers with Node.js.

> **Static Linked Version**: This version uses static linking - all FreeRDP dependencies are bundled in the `.node` file. Users don't need to install FreeRDP separately!

## Features

- Connect to RDP servers (Windows, xrdp, etc.)
- Receive screen updates as bitmap buffers
- Send keyboard events (scancode-based)
- Send mouse events (move, click)
- Support for NLA (Network Level Authentication)
- Graphics acceleration (GFX) support
- **No external dependencies** - FreeRDP statically linked

## Installation

```bash
git clone https://github.com/ecJon/node-freerdp.git
cd node-freerdp
npm install
npm run install  # Runs node-gyp rebuild
```

### Building from Source (for contributors)

If you want to build with your own FreeRDP static libraries:

1. **Build FreeRDP static libraries**:

```bash
cd ~/Documents/code/FreeRDP
mkdir build-static && cd build-static

cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=~/freerdp-static \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TESTING=OFF \
  -DWITH_SERVER=OFF \
  -DWITH_CLIENT=ON \
  -DWITH_SAMPLE=OFF \
  -DWITH_CLIENT_SDL=OFF \
  -DWITH_CLIENT_MAC=OFF \
  -DWITH_X11=OFF \
  -DWITH_CUPS=OFF \
  -DWITH_PCSC=OFF \
  -DWITH_PKCS11=OFF \
  -DWITH_SMARTCARD_EMULATE=OFF \
  -DWITH_SMARTCARD_PCSC=OFF \
  -DWITH_FFMPEG=OFF \
  -DWITH_SWSCALE=OFF \
  -DWITH_GFX_H264=OFF \
  -DWITH_VIDEO_FFMPEG=OFF \
  -DWITH_DSP_FFMPEG=OFF \
  -DWITH_JPEG=OFF \
  -DWITH_WEBVIEW=OFF \
  -DWITH_AAD=OFF \
  -DWITH_LODEPNG=OFF \
  -DWITH_DEBUG_ALL=OFF \
  -DWITH_SDL=OFF \
  -DWITH_OPUS=OFF \
  -DWITH_OPENSSL=ON

make -j$(sysctl -n hw.ncpu)
make install
```

2. **Update binding.gyp** to point to your static library path.

3. **Build the module**:

```bash
npm run install
```

## API Usage

### Connect

```javascript
const freerdp = require('./build/Release/node-freerdp.node');

const args = [
  'node-freerdp',    // Program name (argv[0])
  '/v:192.168.1.100',  // Server address
  '/u:username',       // Username
  '/p:password',       // Password
  '/w:1280',           // Width
  '/h:720',            // Height
  '/gfx'               // Enable GFX graphics (optional)
];

const session = freerdp.connect(args, (event, data) => {
  switch (event) {
    case 'connect':
      console.log('Connected successfully!');
      break;

    case 'bitmap':
      // Screen update
      console.log(`Screen update: ${data.x}, ${data.y}, ${data.w}x${data.h}`);
      // data.buffer is a Buffer with pixel data (ABGR format)
      // data.bpp is bytes per pixel (4 for ABGR32)
      break;

    case 'close':
      console.log('Connection closed');
      break;
  }
});

console.log('Session ID:', session);
```

### Send Keyboard Event

```javascript
// Send scancode (e.g., 'A' key)
const scancode = 0x1e;  // Scancode for 'A'
freerdp.sendKeyEventScancode(session, scancode, 1);  // Press
freerdp.sendKeyEventScancode(session, scancode, 0);  // Release
```

Common scancodes:
- `0x1e` = A
- `0x30` = B
- `0x2e` = C
- `0x1c` = Enter
- `0x39` = Space
- `0x01` = Esc

See [Microsoft Scancode Documentation](https://learn.microsoft.com/en-us/windows/win32/inputdev/keyboard-input) for full list.

### Send Mouse Event

```javascript
const flags = {
  PTR_FLAGS_MOVE: 0x0800,
  PTR_FLAGS_DOWN: 0x8000,
  PTR_FLAGS_BUTTON1: 0x1000,  // Left
  PTR_FLAGS_BUTTON2: 0x2000,  // Middle
  PTR_FLAGS_BUTTON3: 0x4000,  // Right
};

// Move mouse
freerdp.sendPointerEvent(session, flags.PTR_FLAGS_MOVE, x, y);

// Left click
freerdp.sendPointerEvent(session, flags.PTR_FLAGS_DOWN | flags.PTR_FLAGS_BUTTON1, x, y);
```

### Close Connection

```javascript
freerdp.close(session);
```

## Command Line Arguments

FreeRDP command line arguments are supported. Common options:

| Argument | Description |
|----------|-------------|
| `/v:host` | Server address |
| `/u:user` | Username |
| `/p:pass` | Password |
| `/w:width` | Screen width |
| `/h:height` | Screen height |
| `/port:port` | Port (default: 3389) |
| `/gfx` | Enable GFX graphics |
| `/log-level:level` | Log level (trace, debug, info, warn, error) |
| `/cert:ignore` | Ignore certificate verification |
| `/sec:nla` | Enable NLA |

For full argument list, see [FreeRDP Documentation](https://freerdp.com/freerdp-user/Manual).

## Static Linking Details

The `.node` file is statically linked with:
- FreeRDP 3.x (libfreerdp3.a, libfreerdp-client3.a)
- WinPR 3.x (libwinpr3.a)
- OpenSSL (libssl.a, libcrypto.a)

Final binary size: ~8MB (contains all dependencies)

Users only need system frameworks (no FreeRDP installation required):
- **macOS**: Security, CoreFoundation, CoreServices, CoreGraphics, Carbon, etc.
- **Linux**: ssl, crypto, z, pthread, rt
- **Windows**: openssl libs, crypt32, secur32

## Electron Integration

For Electron apps, the static-linked `.node` file works out of the box:

1. Place `node-freerdp.node` in your app resources
2. Rebuild for Electron's Node version:
   ```bash
   npm install --runtime=electron --dist-url=https://electronjs.org/headers
   ```
3. Load in your Electron app:
   ```javascript
   const freerdp = require('./path/to/node-freerdp.node');
   ```

## Roadmap

- [ ] Rewrite clipboard functionality for FreeRDP 3.x
- [ ] Better error handling and reporting
- [ ] Memory leak investigation
- [ ] Funnel libfreerdp stdout messages to events
- [ ] TypeScript definitions
- [ ] Prebuilt binaries for multiple platforms (Linux x64/arm64, Windows x64/arm64)

## License

BSD

## Original Author

Michael Wasser

## Maintainers

This project was originally created for FreeRDP 1.x and has been updated to support FreeRDP 3.x with static linking.
