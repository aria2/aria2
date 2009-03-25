/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2006 Tatsuhiro Tsujikawa
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */

#define TEXT_DIR \
_(" -d, --dir=DIR                The directory to store the downloaded file.")
#define TEXT_OUT \
_(" -o, --out=FILE               The file name of the downloaded file.")
#define TEXT_LOG \
_(" -l, --log=LOG                The file name of the log file. If '-' is\n"\
  "                              specified, log is written to stdout.")
#define TEXT_DAEMON \
_(" -D, --daemon                 Run as daemon.")
#define TEXT_SPLIT \
_(" -s, --split=N                Download a file using N connections. If more\n"\
  "                              than N URLs are given, first N URLs are used and\n"\
  "                              remaining URLs are used for backup. If less than\n"\
  "                              N URLs are given, those URLs are used more than\n"\
  "                              once so that N connections total are made\n"\
  "                              simultaneously. Please see -j option too.\n"\
  "                              Please note that in Metalink download, this\n"\
  "                              option has no effect and use -C option instead.")
#define TEXT_RETRY_WAIT \
_(" --retry-wait=SEC             Set the seconds to wait to retry after an error\n"\
  "                              has occured.")
#define TEXT_TIMEOUT \
_(" -t, --timeout=SEC            Set timeout in seconds.")
#define TEXT_MAX_TRIES \
_(" -m, --max-tries=N            Set number of tries. 0 means unlimited.")
#define TEXT_HTTP_PROXY \
_(" --http-proxy=PROXY           Use this proxy server for HTTP.\n"\
  "                              See also  --all-proxy option.\n"\
  "                              This affects all URLs.")
#define TEXT_HTTPS_PROXY \
_(" --https-proxy=PROXY          Use this proxy server for HTTPS.\n"\
  "                              See also  --all-proxy option.\n"\
  "                              This affects all URLs.")
#define TEXT_FTP_PROXY \
_(" --ftp-proxy=PROXY            Use this proxy server for FTP.\n"\
  "                              See also  --all-proxy option.\n"\
  "                              This affects all URLs.")
#define TEXT_ALL_PROXY \
_(" --all-proxy=PROXY            Use this proxy server for all protocols.\n"\
  "                              You can override this setting and specify a\n"\
  "                              proxy server for a particular protocol using\n"\
  "                              --http-proxy, --https-proxy and --ftp-proxy\n"\
  "                              options.\n"\
  "                              This affects all URLs.")
#define TEXT_HTTP_USER \
_(" --http-user=USER             Set HTTP user. This affects all URLs.")
#define TEXT_HTTP_PASSWD \
_(" --http-passwd=PASSWD         Set HTTP password. This affects all URLs.")
#define TEXT_PROXY_METHOD \
_(" --proxy-method=METHOD        Set the method to use in proxy request.")
#define TEXT_HTTP_AUTH_SCHEME \
_(" --http-auth-scheme=SCHEME    Set HTTP authentication scheme. Currently, basic\n"\
  "                              is the only supported scheme.")
#define TEXT_REFERER \
_(" --referer=REFERER            Set Referer. This affects all URLs.")
#define TEXT_FTP_USER \
_(" --ftp-user=USER              Set FTP user. This affects all URLs.")
#define TEXT_FTP_PASSWD \
_(" --ftp-passwd=PASSWD          Set FTP password. This affects all URLs.")
#define TEXT_FTP_TYPE \
_(" --ftp-type=TYPE              Set FTP transfer type.")
#define TEXT_FTP_PASV \
_(" -p, --ftp-pasv[=true|false]  Use the passive mode in FTP. If false is given,\n"\
  "                              the active mode will be used.")
#define TEXT_LOWEST_SPEED_LIMIT \
_(" --lowest-speed-limit=SPEED   Close connection if download speed is lower than\n"\
  "                              or equal to this value(bytes per sec).\n"\
  "                              0 means aria2 does not have a lowest speed limit.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).\n"\
  "                              This option does not affect BitTorrent downloads.")
#define TEXT_MAX_OVERALL_DOWNLOAD_LIMIT \
_(" --max-overall-download-limit=SPEED Set max overall download speed in bytes/sec.\n"\
  "                              0 means unrestricted.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).\n"\
  "                              To limit the download speed per download, use\n"\
  "                              --max-download-limit option.")
#define TEXT_MAX_DOWNLOAD_LIMIT \
_(" --max-download-limit=SPEED   Set max download speed per each download in\n"\
  "                              bytes/sec. 0 means unrestricted.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).\n"\
  "                              To limit the overall download speed, use\n"\
  "                              --max-overall-download-limit option.")
#define TEXT_FILE_ALLOCATION \
_(" --file-allocation=METHOD     Specify file allocation method.\n"\
  "                              'none' doesn't pre-allocate file space. 'prealloc'\n"\
  "                              pre-allocates file space before download begins.\n"\
  "                              This may take some time depending on the size of\n"\
  "                              the file.\n"\
  "                              If you are using newer file systems such as ext4\n"\
  "                              (with extents support), btrfs or xfs, 'falloc' is\n"\
  "                              your best choice. It allocates large(few GiB)\n"\
  "                              files almost instantly. Don't use 'falloc' with\n"\
  "                              legacy file systems such as ext3 because it takes\n"\
  "                              almost same time as 'prealloc' and it blocks aria2\n"\
  "                              entirely until allocation finishes. 'falloc' may\n"\
  "                              not be available if your system doesn't have\n"\
  "                              posix_fallocate() function.")
#define TEXT_NO_FILE_ALLOCATION_LIMIT \
_(" --no-file-allocation-limit=SIZE No file allocation is made for files whose\n"\
  "                              size is smaller than SIZE.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).")
# define TEXT_ENABLE_DIRECT_IO \
_(" --enable-direct-io[=true|false] Enable directI/O, which lowers cpu usage while\n"\
  "                              allocating files.\n"\
  "                              Turn off if you encounter any error")
#define TEXT_ALLOW_OVERWRITE \
_(" --allow-overwrite=true|false If false, aria2 doesn't download a file which\n"\
  "                              already exists but the corresponding .aria2 file\n"\
  "                              doesn't exist.")
#define TEXT_ALLOW_PIECE_LENGTH_CHANGE \
_(" --allow-piece-length-change=true|false If false is given, aria2 aborts download\n"\
  "                              when a piece length is different from one in\n"\
  "                              a control file. If true is given, you can proceed\n"\
  "                              but some download progress will be lost.")  
#define TEXT_FORCE_SEQUENTIAL \
_(" -Z, --force-sequential[=true|false] Fetch URIs in the command-line sequentially\n"\
  "                              and download each URI in a separate session, like\n"\
  "                              the usual command-line download utilities.")
#define TEXT_AUTO_FILE_RENAMING \
_(" --auto-file-renaming[=true|false] Rename file name if the same file already\n"\
  "                              exists. This option works only in http(s)/ftp\n"\
  "                              download.\n"\
  "                              The new file name has a dot and a number(1..9999)\n"\
  "                              appended.")
#define TEXT_PARAMETERIZED_URI \
_(" -P, --parameterized-uri[=true|false] Enable parameterized URI support.\n"\
  "                              You can specify set of parts:\n"\
  "                              http://{sv1,sv2,sv3}/foo.iso\n"\
  "                              Also you can specify numeric sequences with step\n"\
  "                              counter:\n"\
  "                              http://host/image[000-100:2].img\n"\
  "                              A step counter can be omitted.\n"\
  "                              If all URIs do not point to the same file, such\n"\
  "                              as the second example above, -Z option is\n"\
  "                              required.")
#define TEXT_ENABLE_HTTP_KEEP_ALIVE \
_(" --enable-http-keep-alive[=true|false] Enable HTTP/1.1 persistent connection.")
#define TEXT_ENABLE_HTTP_PIPELINING \
_(" --enable-http-pipelining[=true|false] Enable HTTP/1.1 pipelining.")
#define TEXT_CHECK_INTEGRITY \
_(" -V, --check-integrity[=true|false] Check file integrity by validating piece\n"\
  "                              hashes. This option has effect only in BitTorrent\n"\
  "                              and Metalink downloads with chunk checksums.\n"\
  "                              Use this option to re-download a damaged portion\n"\
  "                              of a file. See also --bt-hash-check-seed option.")
#define TEXT_BT_HASH_CHECK_SEED \
_(" --bt-hash-check-seed[=true|false] If true is given, after hash check using\n"\
  "                              --check-integrity option and file is complete,\n"\
  "                              continue to seed file. If you want to check file\n"\
  "                              and download it only when it is damaged or\n"\
  "                              incomplete, set this option to false.\n"\
  "                              This option has effect only on BitTorrent\n"\
  "                              download.")
#define TEXT_REALTIME_CHUNK_CHECKSUM \
_(" --realtime-chunk-checksum=true|false  Validate chunk of data by calculating\n"\
  "                              checksum while downloading a file if chunk\n"\
  "                              checksums are provided.")
#define TEXT_CONTINUE \
_(" -c, --continue               Continue downloading a partially downloaded\n"\
  "                              file. Use this option to resume a download\n"\
  "                              started by a web browser or another program\n"\
  "                              which downloads files sequentially from the\n"\
  "                              beginning. Currently this option is only\n"\
  "                              applicable to http(s)/ftp downloads.")
#define TEXT_USER_AGENT \
_(" -U, --user-agent=USER_AGENT  Set user agent for http(s) downloads.")
#define TEXT_NO_NETRC \
_(" -n, --no-netrc               Disables netrc support.")
#define TEXT_INPUT_FILE \
_(" -i, --input-file=FILE        Downloads URIs found in FILE. You can specify\n"\
  "                              multiple URIs for a single entity: separate\n"\
  "                              URIs on a single line using the TAB character.\n"\
  "                              Reads input from stdin when '-' is specified.\n"\
  "                              The additional out and dir options can be\n"\
  "                              specified after each line of URIs. This optional\n"\
  "                              line must start with white space(s). See INPUT\n"\
  "                              FILE section of man page for details.")
#define TEXT_MAX_CONCURRENT_DOWNLOADS \
_(" -j, --max-concurrent-downloads=N Set maximum number of parallel downloads for\n"\
  "                              every static (HTTP/FTP) URL, torrent and metalink.\n"\
  "                              See also -s and -C options.")
#define TEXT_LOAD_COOKIES \
_(" --load-cookies=FILE          Load Cookies from FILE using the Firefox3 format\n"\
  "                              and Mozilla/Firefox(1.x/2.x)/Netscape format.")
#define TEXT_SHOW_FILES \
_(" -S, --show-files             Print file listing of .torrent or .metalink file\n"\
  "                              and exit. More detailed information will be listed\n"\
  "                              in case of torrent file.")
#define TEXT_SELECT_FILE \
_(" --select-file=INDEX...       Set file to download by specifying its index.\n"\
  "                              You can find the file index using the\n"\
  "                              --show-files option. Multiple indexes can be\n"\
  "                              specified by using ',', for example: \"3,6\".\n"\
  "                              You can also use '-' to specify a range: \"1-5\".\n"\
  "                              ',' and '-' can be used together.\n"\
  "                              When used with the -M option, index may vary\n"\
  "                              depending on the query(see --metalink-* options).")
#define TEXT_TORRENT_FILE \
_(" -T, --torrent-file=TORRENT_FILE  The path to the .torrent file.")
#define TEXT_FOLLOW_TORRENT \
_(" --follow-torrent=true|false|mem If true or mem is specified, when a file\n"\
  "                              whose suffix is .torrent or content type is\n"\
  "                              application/x-bittorrent is downloaded, aria2\n"\
  "                              parses it as a torrent file and downloads files\n"\
  "                              mentioned in it.\n"\
  "                              If mem is specified, a torrent file is not\n"\
  "                              written to the disk, but is just kept in memory.\n"\
  "                              If false is specified, the action mentioned above\n"\
  "                              is not taken.")
#define TEXT_DIRECT_FILE_MAPPING \
_(" --direct-file-mapping=true|false Directly read from and write to each file\n"\
  "                              mentioned in .torrent file.")
#define TEXT_LISTEN_PORT \
_(" --listen-port=PORT...        Set TCP port number for BitTorrent downloads.\n"\
  "                              Multiple ports can be specified by using ',',\n"\
  "                              for example: \"6881,6885\". You can also use '-'\n"\
  "                              to specify a range: \"6881-6999\". ',' and '-' can\n"\
  "                              be used together.")
#define TEXT_MAX_OVERALL_UPLOAD_LIMIT \
_(" --max-overall-upload-limit=SPEED Set max overall upload speed in bytes/sec.\n"\
  "                              0 means unrestricted.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).\n"\
  "                              To limit the upload speed per torrent, use\n"\
  "                              --max-upload-limit option.")
#define TEXT_MAX_UPLOAD_LIMIT \
_(" -u, --max-upload-limit=SPEED Set max upload speed per each torrent in\n"\
  "                              bytes/sec. 0 means unrestricted.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).\n"\
  "                              To limit the overall upload speed, use\n"\
  "                              --max-overall-upload-limit option.")
#define TEXT_SEED_TIME \
_(" --seed-time=MINUTES          Specify seeding time in minutes. Also see the\n"\
  "                              --seed-ratio option.")
#define TEXT_SEED_RATIO \
_(" --seed-ratio=RATIO           Specify share ratio. Seed completed torrents\n"\
  "                              until share ratio reaches RATIO.\n"\
  "                              You are strongly encouraged to specify equals or\n"\
  "                              more than 1.0 here. Specify 0.0 if you intend to\n"\
  "                              do seeding regardless of share ratio.\n" \
  "                              If --seed-time option is specified along with\n"\
  "                              this option, seeding ends when at least one of\n"\
  "                              the conditions is satisfied.")
#define TEXT_PEER_ID_PREFIX \
_(" --peer-id-prefix=PEERI_ID_PREFIX Specify the prefix of peer ID. The peer ID in\n"\
  "                              BitTorrent is 20 byte length. If more than 20\n"\
  "                              bytes are specified, only first 20\n"\
  "                              bytes are used. If less than 20 bytes are\n"\
  "                              specified, the random alphabet characters are\n"\
  "                              added to make it's length 20 bytes.")
#define TEXT_ENABLE_PEER_EXCHANGE \
_(" --enable-peer-exchange[=true|false] Enable Peer Exchange extension.")
#define TEXT_ENABLE_DHT \
_(" --enable-dht[=true|false]    Enable DHT functionality.")
#define TEXT_DHT_LISTEN_PORT \
_(" --dht-listen-port=PORT...    Set UDP listening port for DHT.\n"\
  "                              Multiple ports can be specified by using ',',\n"\
  "                              for example: \"6881,6885\". You can also use '-'\n"\
  "                              to specify a range: \"6881-6999\". ',' and '-' can\n"\
  "                              be used together.")
#define TEXT_DHT_ENTRY_POINT \
_(" --dht-entry-point=HOST:PORT  Set host and port as an entry point to DHT\n"\
  "                              network.")
#define TEXT_DHT_FILE_PATH \
_(" --dht-file-path=PATH         Change the DHT routing table file to PATH.")
#define TEXT_BT_MIN_CRYPTO_LEVEL \
_(" --bt-min-crypto-level=plain|arc4 Set minimum level of encryption method.\n"\
  "                              If several encryption methods are provided by a\n"\
  "                              peer, aria2 chooses the lowest one which satisfies\n"\
  "                              the given level.")
#define TEXT_BT_REQUIRE_CRYPTO \
_(" --bt-require-crypto=true|false If true is given, aria2 doesn't accept and\n"\
  "                              establish connection with legacy BitTorrent\n"\
  "                              handshake. Thus aria2 always uses Obfuscation\n"\
  "                              handshake.")
#define TEXT_BT_REQUEST_PEER_SPEED_LIMIT \
_(" --bt-request-peer-speed-limit=SPEED If the whole download speed of every\n"\
  "                              torrent is lower than SPEED, aria2 temporarily\n"\
  "                              increases the number of peers to try for more\n"\
  "                              download speed. Configuring this option with your\n"\
  "                              preferred download speed can increase your\n"\
  "                              download speed in some cases.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).")
#define TEXT_BT_MAX_OPEN_FILES \
_(" --bt-max-open-files=NUM      Specify maximum number of files to open in each\n"\
  "                              BitTorrent download.")
#define TEXT_BT_SEED_UNVERIFIED \
_(" --bt-seed-unverified[=true|false] Seed previously downloaded files without\n"\
  "                              verifying piece hashes.")
#define TEXT_BT_MAX_PEERS \
_(" --bt-max-peers=NUM           Specify the maximum number of peers per torrent.\n"\
  "                              0 means unlimited.\n"\
  "                              See also --bt-request-peer-speed-limit option.")
#define TEXT_METALINK_FILE \
_(" -M, --metalink-file=METALINK_FILE The file path to the .metalink file.")
#define TEXT_METALINK_SERVERS \
_(" -C, --metalink-servers=NUM_SERVERS The number of servers to connect to\n"\
  "                              simultaneously. Some Metalinks regulate the\n"\
  "                              number of servers to connect. aria2 strictly\n"\
  "                              respects them. This means that if Metalink defines\n"\
  "                              the maxconnections attribute lower than\n"\
  "                              NUM_SERVERS, then aria2 uses the value of\n"\
  "                              maxconnections attribute instead of NUM_SERVERS.\n"\
  "                              See also -s and -j options.")
#define TEXT_METALINK_VERSION \
_(" --metalink-version=VERSION   The version of the file to download.")
#define TEXT_METALINK_LANGUAGE \
_(" --metalink-language=LANGUAGE The language of the file to download.")
#define TEXT_METALINK_OS \
_(" --metalink-os=OS             The operating system of the file to download.")
#define TEXT_METALINK_LOCATION \
_(" --metalink-location=LOCATION[,...] The location of the preferred server.\n"\
  "                              A comma-delimited list of locations is\n"\
  "                              acceptable.")
#define TEXT_METALINK_PREFERRED_PROTOCOL \
_(" --metalink-preferred-protocol=PROTO Specify preferred protocol. Specify 'none'\n"\
  "                              if you don't have any preferred protocol.")
#define TEXT_FOLLOW_METALINK \
_(" --follow-metalink=true|false|mem If true or mem is specified, when a file\n"\
  "                              whose suffix is .metaink or content type of\n"\
  "                              application/metalink+xml is downloaded, aria2\n"\
  "                              parses it as a metalink file and downloads files\n"\
  "                              mentioned in it.\n"\
  "                              If mem is specified, a metalink file is not\n"\
  "                              written to the disk, but is just kept in memory.\n"\
  "                              If false is specified, the action mentioned above\n"\
  "                              is not taken.")
#define TEXT_METALINK_ENABLE_UNIQUE_PROTOCOL \
_(" --metalink-enable-unique-protocol=true|false If true is given and several\n"\
  "                              protocols are available for a mirror in a metalink\n"\
  "                              file, aria2 uses one of them.\n"\
  "                              Use --metalink-preferred-protocol option to\n"\
  "                              specify the preference of protocol.")
#define TEXT_VERSION \
_(" -v, --version                Print the version number and exit.")
#define TEXT_HELP \
_(" -h, --help[=CATEGORY]        Print usage and exit.\n"\
  "                              The help messages are classified in several\n"\
  "                              categories. For example, type \"--help=http\" for\n"\
  "                              detailed explanation for the options related to\n"\
  "                              http. If no matching category is found, search\n"\
  "                              option name using a given word in middle match\n"\
  "                              and print the result.")
#define TEXT_NO_CONF \
_(" --no-conf                    Disable loading aria2.conf file.")
#define TEXT_CONF_PATH \
_(" --conf-path=PATH             Change the configuration file path to PATH.")
#define TEXT_STOP \
_(" --stop=SEC                   Stop application after SEC seconds has passed.\n" \
  "                              If 0 is given, this feature is disabled.")
#define TEXT_HEADER \
_(" --header=HEADER              Append HEADER to HTTP request header. You can use\n"\
  "                              this option repeatedly to specify more than one\n"\
  "                              header:\n"\
  "                              aria2c --header=\"X-A: b78\" --header=\"X-B: 9J1\"\n"\
  "                              http://host/file")
#define TEXT_QUIET \
_(" -q, --quiet[=true|false]     Make aria2 quiet(no console output).")
#define TEXT_ASYNC_DNS \
_(" --async-dns[=true|false]     Enable asynchronous DNS.")
#define TEXT_FTP_REUSE_CONNECTION \
_(" --ftp-reuse-connection[=true|false] Reuse connection in FTP.")
#define TEXT_SUMMARY_INTERVAL \
_(" --summary-interval=SEC       Set interval to output download progress summary.\n"\
  "                              Setting 0 suppresses the output.")
#define TEXT_LOG_LEVEL \
_(" --log-level=LEVEL            Set log level to output.")
#define TEXT_REMOTE_TIME \
_(" -R, --remote-time[=true|false] Retrieve timestamp of the remote file from the\n"\
  "                              remote HTTP/FTP server and if it is available,\n"\
  "                              apply it to the local file.")
#define TEXT_CONNECT_TIMEOUT \
_(" --connect-timeout=SEC        Set the connect timeout in seconds to establish\n"\
  "                              connection to HTTP/FTP/proxy server. After the\n"\
  "                              connection is established, this option makes no\n"\
  "                              effect and --timeout option is used instead.")
#define TEXT_MAX_FILE_NOT_FOUND \
_(" --max-file-not-found=NUM     If aria2 receives `file not found' status from the\n"\
  "                              remote HTTP/FTP servers NUM times without getting\n"\
  "                              a single byte, then force the download to fail.\n"\
  "                              Specify 0 to disable this option.\n"\
  "                              This options is effective only when using\n"\
  "                              HTTP/FTP servers.")
#define TEXT_URI_SELECTOR \
_(" --uri-selector=SELECTOR      Specify URI selection algorithm.\n"\
  "                              If 'inorder' is given, URI is tried in the order\n"\
  "                              appeared in the URI list.\n"\
  "                              If 'feedback' is given, aria2 uses download speed\n"\
  "                              observed in the previous downloads and choose\n"\
  "                              fastest server in the URI list. This also\n"\
  "                              effectively skips dead mirrors. The observed\n"\
  "                              download speed is a part of performance profile\n"\
  "                              of servers mentioned in --server-stat-of and\n"\
  "                              --server-stat-if options.\n"\
  "                              If 'adaptive' is given, selects one of the best\n"\
  "                              mirrors for the first and reserved connections.\n"\
  "                              For supplementary ones, it returns mirrors which\n" \
  "                              has not been tested yet, and if each of them has\n"\
  "                              already been tested, returns mirrors which has to\n"\
  "                              be tested again. Otherwise, it doesn't select\n"\
  "                              anymore mirrors. Like 'feedback', it uses a\n"\
  "                              performance profile of servers.")
#define TEXT_SERVER_STAT_OF \
_(" --server-stat-of=FILE        Specify the filename to which performance profile\n"\
  "                              of the servers is saved. You can load saved data\n"\
  "                              using --server-stat-if option.")
#define TEXT_SERVER_STAT_IF \
_(" --server-stat-if=FILE        Specify the filename to load performance profile\n"\
  "                              of the servers. The loaded data will be used in\n"\
  "                              some URI selector such as 'feedback'.\n"\
  "                              See also --uri-selector option")
#define TEXT_SERVER_STAT_TIMEOUT \
_(" --server-stat-timeout=SEC    Specifies timeout in seconds to invalidate\n"\
  "                              performance profile of the servers since the last\n"\
  "                              contact to them.")
#define TEXT_AUTO_SAVE_INTERVAL \
_(" --auto-save-interval=SEC     Save a control file(*.aria2) every SEC seconds.\n"\
  "                              If 0 is given, a control file is not saved during\n"\
  "                              download. aria2 saves a control file when it stops\n"\
  "                              regardless of the value.")
#define TEXT_CERTIFICATE \
_(" --certificate=FILE           Use the client certificate in FILE.\n"\
  "                              The certificate must be in PEM format.\n"\
  "                              You may use --private-key option to specify the\n"\
  "                              private key.")
#define TEXT_PRIVATE_KEY \
_(" --private-key=FILE           Use the private key in FILE.\n"\
  "                              The private key must be decrypted and in PEM\n"\
  "                              format. See also --certificate option.")
#define TEXT_CA_CERTIFICATE \
_(" --ca-certificate=FILE        Use the certificate authorities in FILE to verify\n"\
  "                              the peers. The certificate file must be in PEM\n"\
  "                              format and can contain multiple CA certificates.\n"\
  "                              Use --check-certificate option to enable\n"\
  "                              verification.")
#define TEXT_CHECK_CERTIFICATE \
_(" --check-certificate[=true|false] Verify the peer using certificates specified\n"\
  "                              in --ca-certificate option.")
#define TEXT_NO_PROXY \
_(" --no-proxy=DOMAINS           Specify comma separated hostnames or domains where\n"\
  "                              proxy should not be used.")
#define TEXT_USE_HEAD \
_(" --use-head[=true|false]      Use HEAD method for the first request to the HTTP\n"\
  "                              server.")
#define TEXT_EVENT_POLL \
_(" --event-poll=POLL            Specify the method for polling events.")
#define TEXT_HTTP_SERVER_LISTEN_PORT \
  " --http-server-listen-port=PORT Specify a port number for the built-in HTTP\n"\
  "                              Server to listen to."
// Excluded from translation candidiates because it is subject to change.
#define TEXT_ENABLE_HTTP_SERVER \
  " --enable-http-server[=true|false] Enable the built-in HTTP server. Currently,\n"\
  "                              this is the experimental feature and it just\n"\
  "                              provides the current download progress. Use your\n"\
  "                              web browser(console-based ones, such as elinks,\n"\
  "                              w3m, are recommended) to connect the server and\n"\
  "                              see what's what."
#define TEXT_BT_EXTERNAL_IP \
_(" --bt-external-ip=IPADDRESS   Specify the external IP address to report to a\n"\
  "                              BitTorrent tracker. Although this function is\n"\
  "                              named 'external', it can accept any kind of IP\n"\
  "                              addresses.")
#define TEXT_HTTP_AUTH_CHALLENGE \
_(" --http-auth-challenge[=true|false] Send HTTP authorization header only when it\n"\
  "                              is requested by the server. If false is set, then\n"\
  "                              authorization header is always sent to the server.\n"\
  "                              There is an exception: if username and password\n"\
  "                              are embedded in URI, authorization header is\n"\
  "                              always sent to the server regardless of this\n"\
  "                              option.")
#define TEXT_INDEX_OUT \
_(" -O, --index-out=INDEX=PATH   Set file path for file with index=INDEX. You can\n"\
  "                              find the file index using the --show-files option.\n"\
  "                              PATH is a relative path to the path specified in\n"\
  "                              --dir option. You can use this option multiple\n"\
  "                              times.")
#define TEXT_DRY_RUN \
_(" --dry-run[=true|false]       If true is given, aria2 just checks whether the\n"\
  "                              remote file is available and doesn't download\n"\
  "                              data. This option has effect on HTTP/FTP download.\n"\
  "                              BitTorrent downloads are canceled if true is\n"\
  "                              specified.")
#define TEXT_BT_TRACKER_INTERVAL \
_(" --bt-tracker-interval=SEC    Set the interval in seconds between tracker\n"\
  "                              requests. This completely overrides interval value\n"\
  "                              and aria2 just uses this value and ignores the\n"\
  "                              min interval and interval value in the response of\n"\
  "                              tracker. If 0 is set, aria2 determines interval\n"\
  "                              based on the response of tracker and the download\n"\
  "                              progress.")
