Node-FreeRDP
============

Node.js addon for FreeRDP 3.x - Connect to RDP servers with Node.js.

> **Note**: This project has been updated to use FreeRDP 3.x API. The original API using `new freerdp.Session()` has been changed to use a callback-based generator pattern.

## Dependencies

This requires **FreeRDP 3.x** to be installed on your system.

### Installing FreeRDP 3.x

#### macOS

```bash
# Option 1: Build from source
cd ~/Documents
git clone https://github.com/FreeRDP/FreeRDP.git
cd FreeRDP
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DCMAKE_INSTALL_PREFIX=/usr/local \
         -DWITH_X11=OFF \
         -DBUILD_TESTING=OFF \
         -DWITH_SERVER=OFF \
         -DWITH_CLIENT=ON
make -j$(sysctl -n hw.ncpu)
sudo make install

# Option 2: Using Homebrew (may have older version)
brew install freerdp
```

#### Ubuntu/Debian

```bash
# Ubuntu 22.04+ has FreeRDP 3.x
sudo apt update
sudo apt install freerdp3-dev libwinpr3-dev
```

#### Fedora

```bash
sudo dnf install freerdp-devel winpr-devel
```

#### Windows

Download pre-built binaries from [FreeRDP GitHub Releases](https://github.com/FreeRDP/FreeRDP/releases) or build from source using Visual Studio.

## Installation

```bash
git clone https://github.com/bloomapi/node-freerdp.git
cd node-freerdp
npm install
npm run install  # Runs node-gyp rebuild
```

## Features

- Connect to RDP servers (Windows, xrdp, etc.)
- Receive screen updates as bitmap buffers
- Send keyboard events (scancode-based)
- Send mouse events (move, click)
- Support for NLA (Network Level Authentication)
- Graphics acceleration (GFX) support

> **Note**: Clipboard functionality is currently disabled and needs to be rewritten for FreeRDP 3.x API.

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
freerdp.sendKey(session, scancode, 1);  // Press
freerdp.sendKey(session, scancode, 0);  // Release
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
freerdp.sendPointer(session, flags.PTR_FLAGS_MOVE, x, y);

// Left click
freerdp.sendPointer(session, flags.PTR_FLAGS_DOWN | flags.PTR_FLAGS_BUTTON1, x, y);
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

## Building from Source

If you need to modify the native code:

```bash
# Clean previous build
rm -rf build

# Rebuild
npm run install
```

## Roadmap

- [ ] Rewrite clipboard functionality for FreeRDP 3.x
- [ ] Better error handling and reporting
- [ ] Memory leak investigation
- [ ] Funnel libfreerdp stdout messages to events
- [ ] TypeScript definitions

## License

BSD

## Original Author

Michael Wasser

## Maintainers

This project was originally created for FreeRDP 1.x and has been updated to support FreeRDP 3.x.
