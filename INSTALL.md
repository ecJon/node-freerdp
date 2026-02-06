# Quick Install Guide

## Option 1: Download Prebuilt Binary (Recommended)

Go to [Releases](https://github.com/ecJon/node-freerdp/releases) and download the binary for your platform:

| Platform | File |
|----------|------|
| macOS x64 (Intel) | `node-freerdp-macos-x64.tar.gz` |
| macOS ARM64 (Apple Silicon) | `node-freerdp-macos-arm64.tar.gz` |
| Linux x64 | `node-freerdp-linux-x64.tar.gz` |
| Linux ARM64 | `node-freerdp-linux-arm64.tar.gz` |
| Windows x64 | `node-freerdp-win32-x64.zip` |

### Usage

```bash
# Extract
tar xzf node-freerdp-macos-arm64.tar.gz
cd node-freerdp-*

# Use in your project
const freerdp = require('./node-freerdp.node');
```

## Option 2: Install from npm (coming soon)

```bash
npm install node-freerdp
```

## Option 3: Build from Source

See [README.md](README.md#building-from-source-for-contributors) for detailed instructions.
