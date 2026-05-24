/*
 * Reproduces aria2 issue #1948.
 *
 * Simulates a live stream:
 *   Request #1: respond with HTTP/1.1 + Transfer-Encoding: chunked,
 *               push ~5 MiB of 'A' as several chunks, then ABORT the
 *               TCP connection WITHOUT sending the chunked terminator.
 *               This is what a live stream looks like when the host
 *               disconnects mid-broadcast (or the network blips).
 *   Request #2: respond with chunked encoding, push only 64 bytes of 'B'
 *               (a "tail packet"), then properly terminate with 0\r\n\r\n.
 *               This simulates the broadcast having ended -- the
 *               server only has the latest tail to give to the retry.
 *
 * With the BUG, aria2 will:
 *   - download ~5 MiB into the local file during request #1,
 *   - on retry call DiskAdaptor::truncate(0) (HttpResponseCommand.cc line 314),
 *   - rewrite the file with only the 64 'B' bytes,
 *   - leaving a 64-byte file instead of a >5 MiB file.
 */

const http = require('http');

const PORT = 18080;
const FIRST_BYTES = 5 * 1024 * 1024;   // 5 MiB of 'A' on first attempt
const TAIL_BYTES  = 64;                // 64 bytes of 'B' on retry

let reqNum = 0;

function writeChunk(socket, buf) {
  socket.write(buf.length.toString(16) + '\r\n');
  socket.write(buf);
  socket.write('\r\n');
}

const server = http.createServer();

// Use raw socket handling so we can deliberately ABORT the connection
// without sending the chunked terminator.
server.on('request', (req, res) => {
  reqNum++;
  const id = reqNum;
  console.log(`[server] request #${id} ${req.method} ${req.url}` +
              ` range=${req.headers.range || '<none>'}`);

  const socket = res.socket;

  // Hand-rolled headers so we have full control over the framing.
  socket.write(
    'HTTP/1.1 200 OK\r\n' +
    'Content-Type: video/mp2t\r\n' +
    'Transfer-Encoding: chunked\r\n' +
    'Connection: close\r\n' +
    '\r\n'
  );

  if (id === 1) {
    // Push 5 MiB of 'A' in 64 KiB chunks, slowly enough that aria2
    // definitely flushes them to disk.
    const CHUNK = 64 * 1024;
    let written = 0;
    const pump = () => {
      if (written >= FIRST_BYTES) {
        console.log(`[server] request #1: wrote ${written} bytes, ABORTING socket`);
        // Abort the TCP connection WITHOUT the chunked terminator.
        // This mimics a live-stream host disconnecting.
        socket.destroy();
        return;
      }
      const buf = Buffer.alloc(CHUNK, 0x41 /* 'A' */);
      writeChunk(socket, buf);
      written += CHUNK;
      setTimeout(pump, 20);
    };
    pump();
  } else {
    // Retry: only a tiny tail is available.
    const buf = Buffer.alloc(TAIL_BYTES, 0x42 /* 'B' */);
    writeChunk(socket, buf);
    // Proper chunked terminator -> aria2 will think this attempt SUCCEEDED.
    socket.write('0\r\n\r\n');
    console.log(`[server] request #${id}: wrote ${TAIL_BYTES} bytes + terminator`);
    socket.end();
  }
});

server.listen(PORT, '127.0.0.1', () => {
  console.log(`[server] listening on http://127.0.0.1:${PORT}/stream.ts`);
});
