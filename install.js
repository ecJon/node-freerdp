#!/usr/bin/env node

const https = require('https');
const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');
const zlib = require('zlib');

const pkg = require('./package.json');

// Get platform info
const os = require('os');
const platform = os.platform();
const arch = os.arch();

// Map platforms
const platformMap = {
  'darwin': 'macos',
  'win32': 'win32',
  'linux': 'linux'
};

const archMap = {
  'x64': 'x64',
  'arm64': 'arm64',
  'arm': 'arm64'
};

const targetPlatform = platformMap[platform];
const targetArch = archMap[arch];

if (!targetPlatform || !targetArch) {
  console.log(`Platform ${platform}-${arch} not supported, building from source...`);
  buildFromSource();
  return;
}

const version = pkg.version.startsWith('v') ? pkg.version : `v${pkg.version}`;
const packageName = `node-freerdp-${targetPlatform}-${targetArch}.tar.gz`;
const downloadUrl = `${pkg.binary.host}${version}/${packageName}`;

console.log(`Downloading prebuilt binary: ${packageName}`);
console.log(`URL: ${downloadUrl}`);

const buildDir = path.join(__dirname, 'build', 'Release');
fs.mkdirSync(buildDir, { recursive: true });

const tempFile = path.join(__dirname, `temp-${packageName}`);

function downloadFile(url, dest) {
  return new Promise((resolve, reject) => {
    const file = fs.createWriteStream(dest);

    https.get(url, (response) => {
      if (response.statusCode === 302 || response.statusCode === 301) {
        downloadFile(response.headers.location, dest).then(resolve).catch(reject);
        return;
      }

      if (response.statusCode !== 200) {
        reject(new Error(`Failed to download: ${response.statusCode}`));
        return;
      }

      const totalSize = parseInt(response.headers['content-length'], 10);
      let downloadedSize = 0;

      response.pipe(file);

      response.on('data', (chunk) => {
        downloadedSize += chunk.length;
        if (totalSize) {
          const percent = Math.floor((downloadedSize / totalSize) * 100);
          process.stdout.write(`\rDownloading... ${percent}%`);
        }
      });

      file.on('finish', () => {
        file.close();
        console.log('\nDownload complete!');
        resolve();
      });

      file.on('error', (err) => {
        fs.unlink(dest, () => {});
        reject(err);
      });
    }).on('error', (err) => {
      reject(err);
    });
  });
}

function extractTarGzip(gzPath, dest) {
  return new Promise((resolve, reject) => {
    const unzip = zlib.createUnzip();
    const chunks = [];

    fs.createReadStream(gzPath)
      .pipe(unzip)
      .on('data', (chunk) => chunks.push(chunk))
      .on('end', () => {
        const tarBuffer = Buffer.concat(chunks);
        // Simple tar extraction for our specific structure
        let pos = 0;

        function readTarEntry() {
          if (pos >= tarBuffer.length - 1024) {
            resolve();
            return;
          }

          // Read header (512 bytes)
          const header = tarBuffer.slice(pos, pos + 512);
          pos += 512;

          // Check for end marker
          if (header[0] === 0) {
            resolve();
            return;
          }

          // Read file name (null-terminated)
          let nameEnd = 0;
          while (nameEnd < 100 && header[nameEnd] !== 0) nameEnd++;
          const fileName = header.toString('ascii', 0, nameEnd);

          // Read size (octal)
          let sizeEnd = 124;
          while (sizeEnd < 124 + 12 && header[sizeEnd] !== 0) sizeEnd++;
          const size = parseInt(header.toString('ascii', 124, sizeEnd), 8);

          // Skip to next 512-byte boundary
          const dataStart = pos;
          const dataEnd = pos + size;
          const paddedEnd = Math.ceil(size / 512) * 512;
          pos = dataStart + paddedEnd;

          // Extract .node file
          if (fileName.endsWith('node-freerdp.node')) {
            const fileData = tarBuffer.slice(dataStart, dataEnd);
            const outputPath = path.join(dest, 'node-freerdp.node');
            fs.writeFileSync(outputPath, fileData);
            console.log(`Extracted: node-freerdp.node (${size} bytes)`);
          }

          readTarEntry();
        }

        readTarEntry();
      })
      .on('error', reject);
  });
}

async function install() {
  try {
    await downloadFile(downloadUrl, tempFile);
    await extractTarGzip(tempFile, buildDir);

    // Cleanup
    fs.unlinkSync(tempFile);
    console.log('Installation complete!');
  } catch (err) {
    console.log(`\nDownload failed: ${err.message}`);
    console.log('Falling back to building from source...');
    if (fs.existsSync(tempFile)) fs.unlinkSync(tempFile);
    buildFromSource();
  }
}

function buildFromSource() {
  console.log('Building from source...');
  console.log('Note: This requires FreeRDP 3.x static libraries to be installed.');
  try {
    execSync('node-gyp rebuild', { stdio: 'inherit' });
  } catch (err) {
    console.error('Build failed. Please ensure FreeRDP 3.x static libraries are installed.');
    console.error('See https://github.com/ecJon/node-freerdp for build instructions.');
    process.exit(1);
  }
}

install();
