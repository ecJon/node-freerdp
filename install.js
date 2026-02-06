#!/usr/bin/env node

const https = require('https');
const http = require('http');
const fs = require('fs');
const path = require('path');
const tar = require('tar');
const { execSync } = require('child_process');

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

const tempFile = path.join(__dirname, `${packageName}`);

function downloadFile(url, dest) {
  return new Promise((resolve, reject) => {
    const protocol = url.startsWith('https') ? https : http;
    const file = fs.createWriteStream(dest);

    protocol.get(url, (response) => {
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

async function install() {
  try {
    await downloadFile(downloadUrl, tempFile);

    console.log('Extracting...');
    await tar.x({
      file: tempFile,
      cwd: buildDir,
      strip: 0
    });

    // Move the .node file to the correct location
    const extractedDir = path.join(buildDir, path.basename(packageName, '.tar.gz'));
    const nodeFile = path.join(extractedDir, 'node-freerdp.node');
    const targetFile = path.join(buildDir, 'node-freerdp.node');

    if (fs.existsSync(nodeFile)) {
      fs.copyFileSync(nodeFile, targetFile);
      // Cleanup
      fs.unlinkSync(tempFile);
      fs.rmSync(extractedDir, { recursive: true, force: true });
      console.log('Installation complete!');
    } else {
      throw new Error('node-freerdp.node not found in archive');
    }
  } catch (err) {
    console.log(`\nDownload failed: ${err.message}`);
    console.log('Falling back to building from source...');
    fs.unlinkSync(tempFile);
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
