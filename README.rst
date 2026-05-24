This branch fixes the issue of downloading live streams: https://github.com/aria2/aria2/issues/1948

The issue can be reproduced by running node stream-server.js and then aria2c http://127.0.0.1:18080/stream.ts.

Using the official aria2c, it will end up with a 64-byte stream.ts file. However, after applying this patch, it will download a 5 MB file, retaining all the data downloaded previously.
