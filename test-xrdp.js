// Test xRDP connection example
const freerdp = require('./build/Release/node-freerdp.node');

// Connection arguments for xRDP
// Replace with your actual server details
const args = [
  'node-freerdp',
  '/v:10.0.3.4',        // Server address
  '/u:root',            // Username - replace with actual username
  '/p:123456',          // Password - replace with actual password
  '/w:1280',            // Width
  '/h:720',              // Height
  '/log-level:trace',   // Enable trace logging
  '/timeout:10000',    // 10 second timeout
  '/gfx'                // Enable GFX (modern graphics)
];

console.log('Connecting to xRDP...');
console.log('Args:', args);

const session = freerdp.connect(args, (event, data) => {
  console.log('Event received:', event);

  if (event === 'connect') {
    console.log('Connected successfully!');
  } else if (event === 'bitmap') {
    console.log('Screen update:', data.x, data.y, data.w, data.h, 'bpp:', data.bpp, 'buffer size:', data.buffer ? data.buffer.length : 0);
  } else if (event === 'close') {
    console.log('Connection closed');
  }
});

console.log('Session ID:', session);

// Keep the process running
process.on('SIGINT', () => {
  console.log('Closing connection...');
  freerdp.close(session);
  setTimeout(() => process.exit(0), 1000);
});

// Example: Send a key press (scan code 0x1E is 'A')
// setTimeout(() => {
//   freerdp.sendKeyEventScancode(session, 0x1E, 1); // Press
//   setTimeout(() => {
//     freerdp.sendKeyEventScancode(session, 0x1E, 0); // Release
//   }, 100);
// }, 2000);

// Example: Send mouse event
// setTimeout(() => {
//   freerdp.sendPointerEvent(session, 0x0000, 640, 360); // Move to center
// }, 2000);

// Close connection after 10 seconds (or remove to keep alive)
// setTimeout(() => {
//   freerdp.close(session);
// }, 10000);
