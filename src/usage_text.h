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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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

#define TEXT_DIR                                                        \
  _(" -d, --dir=DIR                The directory to store the downloaded file.")
#define TEXT_OUT                                                        \
  _(" -o, --out=FILE               The file name of the downloaded file. When -Z\n"\
    "                              option is used, this option is ignored.")
#define TEXT_LOG                                                        \
  _(" -l, --log=LOG                The file name of the log file. If '-' is\n" \
    "                              specified, log is written to stdout.")
#define TEXT_DAEMON                                                     \
  _(" -D, --daemon[=true|false]    Run as daemon. The current working directory will\n" \
    "                              be changed to \"/\" and standard input, standard\n" \
    "                              output and standard error will be redirected to\n" \
    "                              \"/dev/null\".")
#define TEXT_SPLIT                                                      \
  _(" -s, --split=N                Download a file using N connections. If more\n" \
    "                              than N URLs are given, first N URLs are used and\n" \
    "                              remaining URLs are used for backup. If less than\n" \
    "                              N URLs are given, those URLs are used more than\n" \
    "                              once so that N connections total are made\n" \
    "                              simultaneously. The number of connections to the\n" \
    "                              same host is restricted by\n"        \
    "                              --max-connection-per-server option. See also\n" \
    "                              --min-split-size option.")
#define TEXT_RETRY_WAIT                                                 \
  _(" --retry-wait=SEC             Set the seconds to wait between retries. \n" \
    "                              With SEC > 0, aria2 will retry download when the\n" \
    "                              HTTP server returns 503 response.")
#define TEXT_TIMEOUT                                            \
  _(" -t, --timeout=SEC            Set timeout in seconds.")
#define TEXT_MAX_TRIES                                                  \
  _(" -m, --max-tries=N            Set number of tries. 0 means unlimited.")
#define TEXT_HTTP_PROXY                                                 \
  _(" --http-proxy=PROXY           Use this proxy server for HTTP. To erase\n"\
    "                              previously defined proxy, use \"\".\n"   \
    "                              See also  --all-proxy option.\n"     \
    "                              This affects all URLs.")
#define TEXT_HTTPS_PROXY                                                \
  _(" --https-proxy=PROXY          Use this proxy server for HTTPS. To erase\n"  \
    "                              previously defined proxy, use \"\".\n" \
    "                              See also  --all-proxy option.\n"     \
    "                              This affects all URLs.")
#define TEXT_FTP_PROXY                                                  \
  _(" --ftp-proxy=PROXY            Use this proxy server for FTP. To erase previously\n"    \
    "                              defined proxy, use \"\".\n" \
    "                              See also  --all-proxy option.\n"     \
    "                              This affects all URLs.")
#define TEXT_ALL_PROXY                                                  \
  _(" --all-proxy=PROXY            Use this proxy server for all protocols. To erase\n" \
    "                              previously defined proxy, use \"\".\n" \
    "                              You can override this setting and specify a\n" \
    "                              proxy server for a particular protocol using\n" \
    "                              --http-proxy, --https-proxy and --ftp-proxy\n" \
    "                              options.\n"                          \
    "                              This affects all URLs.")
#define TEXT_HTTP_USER                                                  \
  _(" --http-user=USER             Set HTTP user. This affects all URLs.")
#define TEXT_HTTP_PASSWD                                                \
  _(" --http-passwd=PASSWD         Set HTTP password. This affects all URLs.")
#define TEXT_PROXY_METHOD                                               \
  _(" --proxy-method=METHOD        Set the method to use in proxy request.")
#define TEXT_REFERER                                                    \
  _(" --referer=REFERER            Set Referer. This affects all URLs.")
#define TEXT_FTP_USER                                                   \
  _(" --ftp-user=USER              Set FTP user. This affects all URLs.")
#define TEXT_FTP_PASSWD                                                 \
  _(" --ftp-passwd=PASSWD          Set FTP password. This affects all URLs.")
#define TEXT_FTP_TYPE                                           \
  _(" --ftp-type=TYPE              Set FTP transfer type.")
#define TEXT_FTP_PASV                                                   \
  _(" -p, --ftp-pasv[=true|false]  Use the passive mode in FTP. If false is given,\n" \
    "                              the active mode will be used.")
#define TEXT_LOWEST_SPEED_LIMIT                                         \
  _(" --lowest-speed-limit=SPEED   Close connection if download speed is lower than\n" \
    "                              or equal to this value(bytes per sec).\n" \
    "                              0 means aria2 does not have a lowest speed limit.\n" \
    "                              You can append K or M(1K = 1024, 1M = 1024K).\n" \
    "                              This option does not affect BitTorrent downloads.")
#define TEXT_MAX_OVERALL_DOWNLOAD_LIMIT                                 \
  _(" --max-overall-download-limit=SPEED Set max overall download speed in bytes/sec.\n" \
    "                              0 means unrestricted.\n"             \
    "                              You can append K or M(1K = 1024, 1M = 1024K).\n" \
    "                              To limit the download speed per download, use\n" \
    "                              --max-download-limit option.")
#define TEXT_MAX_DOWNLOAD_LIMIT                                         \
  _(" --max-download-limit=SPEED   Set max download speed per each download in\n" \
    "                              bytes/sec. 0 means unrestricted.\n"  \
    "                              You can append K or M(1K = 1024, 1M = 1024K).\n" \
    "                              To limit the overall download speed, use\n" \
    "                              --max-overall-download-limit option.")
#define TEXT_FILE_ALLOCATION                                            \
  _(" --file-allocation=METHOD     Specify file allocation method.\n"   \
    "                              'none' doesn't pre-allocate file space. 'prealloc'\n" \
    "                              pre-allocates file space before download begins.\n" \
    "                              This may take some time depending on the size of\n" \
    "                              the file.\n"                         \
    "                              If you are using newer file systems such as ext4\n" \
    "                              (with extents support), btrfs, xfs or NTFS\n" \
    "                              (MinGW build only), 'falloc' is your best\n"   \
    "                              choice. It allocates large(few GiB) files\n" \
    "                              almost instantly. Don't use 'falloc' with legacy\n" \
    "                              file systems such as ext3 and FAT32 because it\n" \
    "                              takes almost same time as 'prealloc' and it\n" \
    "                              blocks aria2 entirely until allocation finishes.\n" \
    "                              'falloc' may not be available if your system\n" \
    "                              doesn't have posix_fallocate() function.\n" \
    "                              'trunc' uses ftruncate() system call or\n" \
    "                              platform-specific counterpart to truncate a file\n" \
    "                              to a specified length.")
#define TEXT_NO_FILE_ALLOCATION_LIMIT                                   \
  _(" --no-file-allocation-limit=SIZE No file allocation is made for files whose\n" \
    "                              size is smaller than SIZE.\n"        \
    "                              You can append K or M(1K = 1024, 1M = 1024K).")
# define TEXT_ENABLE_DIRECT_IO                                          \
  _(" --enable-direct-io[=true|false] Enable directI/O, which lowers cpu usage while\n" \
    "                              allocating files.\n"                 \
    "                              Turn off if you encounter any error")
#define TEXT_ALLOW_OVERWRITE                                            \
  _(" --allow-overwrite[=true|false] Restart download from scratch if the\n" \
    "                              corresponding control file doesn't exist.  See\n" \
    "                              also --auto-file-renaming option.")
#define TEXT_ALLOW_PIECE_LENGTH_CHANGE                                  \
  _(" --allow-piece-length-change[=true|false] If false is given, aria2 aborts\n" \
    "                              download when a piece length is different from\n" \
    "                              one in a control file. If true is given, you can\n" \
    "                              proceed but some download progress will be lost.")  
#define TEXT_FORCE_SEQUENTIAL                                           \
  _(" -Z, --force-sequential[=true|false] Fetch URIs in the command-line sequentially\n" \
    "                              and download each URI in a separate session, like\n" \
    "                              the usual command-line download utilities.")
#define TEXT_AUTO_FILE_RENAMING                                         \
  _(" --auto-file-renaming[=true|false] Rename file name if the same file already\n" \
    "                              exists. This option works only in http(s)/ftp\n" \
    "                              download.\n"                         \
    "                              The new file name has a dot and a number(1..9999)\n" \
    "                              appended.")
#define TEXT_PARAMETERIZED_URI                                          \
  _(" -P, --parameterized-uri[=true|false] Enable parameterized URI support.\n" \
    "                              You can specify set of parts:\n"     \
    "                              http://{sv1,sv2,sv3}/foo.iso\n"      \
    "                              Also you can specify numeric sequences with step\n" \
    "                              counter:\n"                          \
    "                              http://host/image[000-100:2].img\n"  \
    "                              A step counter can be omitted.\n"    \
    "                              If all URIs do not point to the same file, such\n" \
    "                              as the second example above, -Z option is\n" \
    "                              required.")
#define TEXT_ENABLE_HTTP_KEEP_ALIVE                                     \
  _(" --enable-http-keep-alive[=true|false] Enable HTTP/1.1 persistent connection.")
#define TEXT_ENABLE_HTTP_PIPELINING                                     \
  _(" --enable-http-pipelining[=true|false] Enable HTTP/1.1 pipelining.")
#define TEXT_CHECK_INTEGRITY                                            \
  _(" -V, --check-integrity[=true|false] Check file integrity by validating piece\n" \
    "                              hashes or a hash of entire file. This option has\n" \
    "                              effect only in BitTorrent, Metalink downloads\n" \
    "                              with checksums or HTTP(S)/FTP downloads with\n" \
    "                              --checksum option. If piece hashes are provided,\n" \
    "                              this option can detect damaged portions of a file\n" \
    "                              and re-download them. If a hash of entire file is\n" \
    "                              provided, hash check is only done when file has\n" \
    "                              been already download. This is determined by file\n" \
    "                              length. If hash check fails, file is\n" \
    "                              re-downloaded from scratch. If both piece hashes\n" \
    "                              and a hash of entire file are provided, only\n" \
    "                              piece hashes are used.")
#define TEXT_BT_HASH_CHECK_SEED                                         \
  _(" --bt-hash-check-seed[=true|false] If true is given, after hash check using\n" \
    "                              --check-integrity option and file is complete,\n" \
    "                              continue to seed file. If you want to check file\n" \
    "                              and download it only when it is damaged or\n" \
    "                              incomplete, set this option to false.\n" \
    "                              This option has effect only on BitTorrent\n" \
    "                              download.")
#define TEXT_REALTIME_CHUNK_CHECKSUM                                    \
  _(" --realtime-chunk-checksum[=true|false]  Validate chunk of data by calculating\n" \
    "                              checksum while downloading a file if chunk\n" \
    "                              checksums are provided.")
#define TEXT_CONTINUE                                                   \
  _(" -c, --continue[=true|false]  Continue downloading a partially downloaded\n" \
    "                              file. Use this option to resume a download\n" \
    "                              started by a web browser or another program\n" \
    "                              which downloads files sequentially from the\n" \
    "                              beginning. Currently this option is only\n" \
    "                              applicable to http(s)/ftp downloads.")
#define TEXT_USER_AGENT                                                 \
  _(" -U, --user-agent=USER_AGENT  Set user agent for http(s) downloads.")
#define TEXT_NO_NETRC                                           \
  _(" -n, --no-netrc[=true|false]  Disables netrc support.")
#define TEXT_INPUT_FILE                                                 \
  _(" -i, --input-file=FILE        Downloads URIs found in FILE. You can specify\n" \
    "                              multiple URIs for a single entity: separate\n" \
    "                              URIs on a single line using the TAB character.\n" \
    "                              Reads input from stdin when '-' is specified.\n" \
    "                              Additionally, options can be specified after each\n" \
    "                              line of URI. This optional line must start with\n" \
    "                              one or more white spaces and have one option per\n" \
    "                              single line. See INPUT FILE section of man page\n" \
    "                              for details. See also --deferred-input option.")
#define TEXT_MAX_CONCURRENT_DOWNLOADS                                   \
  _(" -j, --max-concurrent-downloads=N Set maximum number of parallel downloads for\n" \
    "                              every static (HTTP/FTP) URL, torrent and metalink.\n" \
    "                              See also --split option.")
#define TEXT_LOAD_COOKIES                                               \
  _(" --load-cookies=FILE          Load Cookies from FILE using the Firefox3 format\n" \
    "                              and Mozilla/Firefox(1.x/2.x)/Netscape format.")
#define TEXT_SAVE_COOKIES                                               \
  _(" --save-cookies=FILE          Save Cookies to FILE in Mozilla/Firefox(1.x/2.x)/\n" \
    "                              Netscape format. If FILE already exists, it is\n" \
    "                              overwritten. Session Cookies are also saved and\n" \
    "                              their expiry values are treated as 0.")
#define TEXT_SHOW_FILES                                                 \
  _(" -S, --show-files[=true|false] Print file listing of .torrent, .meta4 and\n" \
    "                              .metalink file and exit. More detailed\n" \
    "                              information will be listed in case of torrent\n" \
    "                              file.")
#define TEXT_SELECT_FILE                                                \
  _(" --select-file=INDEX...       Set file to download by specifying its index.\n" \
    "                              You can find the file index using the\n" \
    "                              --show-files option. Multiple indexes can be\n" \
    "                              specified by using ',', for example: \"3,6\".\n" \
    "                              You can also use '-' to specify a range: \"1-5\".\n" \
    "                              ',' and '-' can be used together.\n" \
    "                              When used with the -M option, index may vary\n" \
    "                              depending on the query(see --metalink-* options).")
#define TEXT_TORRENT_FILE                                               \
  _(" -T, --torrent-file=TORRENT_FILE  The path to the .torrent file.")
#define TEXT_FOLLOW_TORRENT                                             \
  _(" --follow-torrent=true|false|mem If true or mem is specified, when a file\n" \
    "                              whose suffix is .torrent or content type is\n" \
    "                              application/x-bittorrent is downloaded, aria2\n" \
    "                              parses it as a torrent file and downloads files\n" \
    "                              mentioned in it.\n"                  \
    "                              If mem is specified, a torrent file is not\n" \
    "                              written to the disk, but is just kept in memory.\n" \
    "                              If false is specified, the action mentioned above\n" \
    "                              is not taken.")
#define TEXT_LISTEN_PORT                                                \
  _(" --listen-port=PORT...        Set TCP port number for BitTorrent downloads.\n" \
    "                              Multiple ports can be specified by using ',',\n" \
    "                              for example: \"6881,6885\". You can also use '-'\n" \
    "                              to specify a range: \"6881-6999\". ',' and '-' can\n" \
    "                              be used together.")
#define TEXT_MAX_OVERALL_UPLOAD_LIMIT                                   \
  _(" --max-overall-upload-limit=SPEED Set max overall upload speed in bytes/sec.\n" \
    "                              0 means unrestricted.\n"             \
    "                              You can append K or M(1K = 1024, 1M = 1024K).\n" \
    "                              To limit the upload speed per torrent, use\n" \
    "                              --max-upload-limit option.")
#define TEXT_MAX_UPLOAD_LIMIT                                           \
  _(" -u, --max-upload-limit=SPEED Set max upload speed per each torrent in\n" \
    "                              bytes/sec. 0 means unrestricted.\n"  \
    "                              You can append K or M(1K = 1024, 1M = 1024K).\n" \
    "                              To limit the overall upload speed, use\n" \
    "                              --max-overall-upload-limit option.")
#define TEXT_SEED_TIME                                                  \
  _(" --seed-time=MINUTES          Specify seeding time in minutes. Also see the\n" \
    "                              --seed-ratio option.")
#define TEXT_SEED_RATIO                                                 \
  _(" --seed-ratio=RATIO           Specify share ratio. Seed completed torrents\n" \
    "                              until share ratio reaches RATIO.\n"  \
    "                              You are strongly encouraged to specify equals or\n" \
    "                              more than 1.0 here. Specify 0.0 if you intend to\n" \
    "                              do seeding regardless of share ratio.\n" \
    "                              If --seed-time option is specified along with\n" \
    "                              this option, seeding ends when at least one of\n" \
    "                              the conditions is satisfied.")
#define TEXT_PEER_ID_PREFIX                                             \
  _(" --peer-id-prefix=PEER_ID_PREFIX Specify the prefix of peer ID. The peer ID in\n" \
    "                              BitTorrent is 20 byte length. If more than 20\n" \
    "                              bytes are specified, only first 20 bytes are\n" \
    "                              used. If less than 20 bytes are specified, random\n" \
    "                              byte data are added to make its length 20 bytes.")
#define TEXT_ENABLE_PEER_EXCHANGE                                       \
  _(" --enable-peer-exchange[=true|false] Enable Peer Exchange extension.")
#define TEXT_ENABLE_DHT                                         \
  _(" --enable-dht[=true|false]    Enable IPv4 DHT functionality.")
#define TEXT_DHT_LISTEN_PORT                                            \
  _(" --dht-listen-port=PORT...    Set UDP listening port for both IPv4 and IPv6\n"   \
    "                              DHT. Multiple ports can be specified by using\n" \
    "                              ',', for example: \"6881,6885\". You can also\n" \
    "                              use '-' to specify a range: \"6881-6999\". ','\n" \
    "                              and '-' can be used together.")
#define TEXT_DHT_ENTRY_POINT                                            \
  _(" --dht-entry-point=HOST:PORT  Set host and port as an entry point to IPv4 DHT\n" \
    "                              network.")
#define TEXT_DHT_FILE_PATH                                              \
  _(" --dht-file-path=PATH         Change the IPv4 DHT routing table file to PATH.")
#define TEXT_BT_MIN_CRYPTO_LEVEL                                        \
  _(" --bt-min-crypto-level=plain|arc4 Set minimum level of encryption method.\n" \
    "                              If several encryption methods are provided by a\n" \
    "                              peer, aria2 chooses the lowest one which satisfies\n" \
    "                              the given level.")
#define TEXT_BT_REQUIRE_CRYPTO                                          \
  _(" --bt-require-crypto[=true|false] If true is given, aria2 doesn't accept and\n" \
    "                              establish connection with legacy BitTorrent\n" \
    "                              handshake. Thus aria2 always uses Obfuscation\n" \
    "                              handshake.")
#define TEXT_BT_REQUEST_PEER_SPEED_LIMIT                                \
  _(" --bt-request-peer-speed-limit=SPEED If the whole download speed of every\n" \
    "                              torrent is lower than SPEED, aria2 temporarily\n" \
    "                              increases the number of peers to try for more\n" \
    "                              download speed. Configuring this option with your\n" \
    "                              preferred download speed can increase your\n" \
    "                              download speed in some cases.\n"     \
    "                              You can append K or M(1K = 1024, 1M = 1024K).")
#define TEXT_BT_MAX_OPEN_FILES                                          \
  _(" --bt-max-open-files=NUM      Specify maximum number of files to open in each\n" \
    "                              BitTorrent download.")
#define TEXT_BT_SEED_UNVERIFIED                                         \
  _(" --bt-seed-unverified[=true|false] Seed previously downloaded files without\n" \
    "                              verifying piece hashes.")
#define TEXT_BT_MAX_PEERS                                               \
  _(" --bt-max-peers=NUM           Specify the maximum number of peers per torrent.\n" \
    "                              0 means unlimited.\n"                \
    "                              See also --bt-request-peer-speed-limit option.")
#define TEXT_METALINK_FILE                                              \
  _(" -M, --metalink-file=METALINK_FILE The file path to the .meta4 and .metalink\n" \
    "                              file. Reads input from stdin when '-' is\n" \
    "                              specified.")
#define TEXT_METALINK_SERVERS                                           \
  _(" -C, --metalink-servers=NUM_SERVERS The number of servers to connect to\n" \
    "                              simultaneously. Some Metalinks regulate the\n" \
    "                              number of servers to connect. aria2 strictly\n" \
    "                              respects them. This means that if Metalink defines\n" \
    "                              the maxconnections attribute lower than\n" \
    "                              NUM_SERVERS, then aria2 uses the value of\n" \
    "                              maxconnections attribute instead of NUM_SERVERS.\n" \
    "                              See also -s and -j options.")
#define TEXT_METALINK_VERSION                                           \
  _(" --metalink-version=VERSION   The version of the file to download.")
#define TEXT_METALINK_LANGUAGE                                          \
  _(" --metalink-language=LANGUAGE The language of the file to download.")
#define TEXT_METALINK_OS                                                \
  _(" --metalink-os=OS             The operating system of the file to download.")
#define TEXT_METALINK_LOCATION                                          \
  _(" --metalink-location=LOCATION[,...] The location of the preferred server.\n" \
    "                              A comma-delimited list of locations is\n" \
    "                              acceptable.")
#define TEXT_METALINK_PREFERRED_PROTOCOL                                \
  _(" --metalink-preferred-protocol=PROTO Specify preferred protocol. Specify 'none'\n" \
    "                              if you don't have any preferred protocol.")
#define TEXT_FOLLOW_METALINK                                            \
  _(" --follow-metalink=true|false|mem If true or mem is specified, when a file\n" \
    "                              whose suffix is .meta4 or .metalink, or content\n" \
    "                              type of application/metalink4+xml or\n" \
    "                              application/metalink+xml is downloaded, aria2\n" \
    "                              parses it as a metalink file and downloads files\n" \
    "                              mentioned in it.\n"                  \
    "                              If mem is specified, a metalink file is not\n" \
    "                              written to the disk, but is just kept in memory.\n" \
    "                              If false is specified, the action mentioned above\n" \
    "                              is not taken.")
#define TEXT_METALINK_ENABLE_UNIQUE_PROTOCOL                            \
  _(" --metalink-enable-unique-protocol[=true|false] If true is given and several\n" \
    "                              protocols are available for a mirror in a metalink\n" \
    "                              file, aria2 uses one of them.\n"     \
    "                              Use --metalink-preferred-protocol option to\n" \
    "                              specify the preference of protocol.")
#define TEXT_VERSION                                                    \
  _(" -v, --version                Print the version number and exit.")
#define TEXT_HELP                                                       \
  _(" -h, --help[=TAG|KEYWORD]     Print usage and exit.\n"             \
    "                              The help messages are classified with tags. A tag\n" \
    "                              starts with \"#\". For example, type \"--help=#http\"\n" \
    "                              to get the usage for the options tagged with\n" \
    "                              \"#http\". If non-tag word is given, print the usage\n" \
    "                              for the options whose name includes that word.")
#define TEXT_NO_CONF                                                    \
  _(" --no-conf[=true|false]       Disable loading aria2.conf file.")
#define TEXT_CONF_PATH                                                  \
  _(" --conf-path=PATH             Change the configuration file path to PATH.")
#define TEXT_STOP                                                       \
  _(" --stop=SEC                   Stop application after SEC seconds has passed.\n" \
    "                              If 0 is given, this feature is disabled.")
#define TEXT_HEADER                                                     \
  _(" --header=HEADER              Append HEADER to HTTP request header. You can use\n" \
    "                              this option repeatedly to specify more than one\n" \
    "                              header:\n"                           \
    "                              aria2c --header=\"X-A: b78\" --header=\"X-B: 9J1\"\n" \
    "                              http://host/file")
#define TEXT_QUIET                                                      \
  _(" -q, --quiet[=true|false]     Make aria2 quiet(no console output).")
#define TEXT_ASYNC_DNS                                          \
  _(" --async-dns[=true|false]     Enable asynchronous DNS.")
#define TEXT_FTP_REUSE_CONNECTION                                       \
  _(" --ftp-reuse-connection[=true|false] Reuse connection in FTP.")
#define TEXT_SUMMARY_INTERVAL                                           \
  _(" --summary-interval=SEC       Set interval to output download progress summary.\n" \
    "                              Setting 0 suppresses the output.")
#define TEXT_LOG_LEVEL                                          \
  _(" --log-level=LEVEL            Set log level to output.")
#define TEXT_REMOTE_TIME                                                \
  _(" -R, --remote-time[=true|false] Retrieve timestamp of the remote file from the\n" \
    "                              remote HTTP/FTP server and if it is available,\n" \
    "                              apply it to the local file.")
#define TEXT_CONNECT_TIMEOUT                                            \
  _(" --connect-timeout=SEC        Set the connect timeout in seconds to establish\n" \
    "                              connection to HTTP/FTP/proxy server. After the\n" \
    "                              connection is established, this option makes no\n" \
    "                              effect and --timeout option is used instead.")
#define TEXT_MAX_FILE_NOT_FOUND                                         \
  _(" --max-file-not-found=NUM     If aria2 receives `file not found' status from the\n" \
    "                              remote HTTP/FTP servers NUM times without getting\n" \
    "                              a single byte, then force the download to fail.\n" \
    "                              Specify 0 to disable this option.\n" \
    "                              This options is effective only when using\n" \
    "                              HTTP/FTP servers.")
#define TEXT_URI_SELECTOR                                               \
  _(" --uri-selector=SELECTOR      Specify URI selection algorithm.\n"  \
    "                              If 'inorder' is given, URI is tried in the order\n" \
    "                              appeared in the URI list.\n"         \
    "                              If 'feedback' is given, aria2 uses download speed\n" \
    "                              observed in the previous downloads and choose\n" \
    "                              fastest server in the URI list. This also\n" \
    "                              effectively skips dead mirrors. The observed\n" \
    "                              download speed is a part of performance profile\n" \
    "                              of servers mentioned in --server-stat-of and\n" \
    "                              --server-stat-if options.\n"         \
    "                              If 'adaptive' is given, selects one of the best\n" \
    "                              mirrors for the first and reserved connections.\n" \
    "                              For supplementary ones, it returns mirrors which\n" \
    "                              has not been tested yet, and if each of them has\n" \
    "                              already been tested, returns mirrors which has to\n" \
    "                              be tested again. Otherwise, it doesn't select\n" \
    "                              anymore mirrors. Like 'feedback', it uses a\n" \
    "                              performance profile of servers.")
#define TEXT_SERVER_STAT_OF                                             \
  _(" --server-stat-of=FILE        Specify the filename to which performance profile\n" \
    "                              of the servers is saved. You can load saved data\n" \
    "                              using --server-stat-if option.")
#define TEXT_SERVER_STAT_IF                                             \
  _(" --server-stat-if=FILE        Specify the filename to load performance profile\n" \
    "                              of the servers. The loaded data will be used in\n" \
    "                              some URI selector such as 'feedback'.\n" \
    "                              See also --uri-selector option")
#define TEXT_SERVER_STAT_TIMEOUT                                        \
  _(" --server-stat-timeout=SEC    Specifies timeout in seconds to invalidate\n" \
    "                              performance profile of the servers since the last\n" \
    "                              contact to them.")
#define TEXT_AUTO_SAVE_INTERVAL                                         \
  _(" --auto-save-interval=SEC     Save a control file(*.aria2) every SEC seconds.\n" \
    "                              If 0 is given, a control file is not saved during\n" \
    "                              download. aria2 saves a control file when it stops\n" \
    "                              regardless of the value.")
#define TEXT_CERTIFICATE                                                \
  _(" --certificate=FILE           Use the client certificate in FILE.\n" \
    "                              The certificate must be in PEM format.\n" \
    "                              You may use --private-key option to specify the\n" \
    "                              private key.")
#define TEXT_PRIVATE_KEY                                                \
  _(" --private-key=FILE           Use the private key in FILE.\n"      \
    "                              The private key must be decrypted and in PEM\n" \
    "                              format. See also --certificate option.")
#define TEXT_CA_CERTIFICATE                                             \
  _(" --ca-certificate=FILE        Use the certificate authorities in FILE to verify\n" \
    "                              the peers. The certificate file must be in PEM\n" \
    "                              format and can contain multiple CA certificates.\n" \
    "                              Use --check-certificate option to enable\n" \
    "                              verification.")
#define TEXT_CHECK_CERTIFICATE                                          \
  _(" --check-certificate[=true|false] Verify the peer using certificates specified\n" \
    "                              in --ca-certificate option.")
#define TEXT_NO_PROXY                                                   \
  _(" --no-proxy=DOMAINS           Specify comma separated hostnames, domains or\n" \
    "                              network address with or without CIDR block where\n" \
    "                              proxy should not be used.")
#define TEXT_USE_HEAD                                                   \
  _(" --use-head[=true|false]      Use HEAD method for the first request to the HTTP\n" \
    "                              server.")
#define TEXT_EVENT_POLL                                                 \
  _(" --event-poll=POLL            Specify the method for polling events.")
#define TEXT_BT_EXTERNAL_IP                                             \
  _(" --bt-external-ip=IPADDRESS   Specify the external IP address to report to a\n" \
    "                              BitTorrent tracker. Although this function is\n" \
    "                              named 'external', it can accept any kind of IP\n" \
    "                              addresses.")
#define TEXT_HTTP_AUTH_CHALLENGE                                        \
  _(" --http-auth-challenge[=true|false] Send HTTP authorization header only when it\n" \
    "                              is requested by the server. If false is set, then\n" \
    "                              authorization header is always sent to the server.\n" \
    "                              There is an exception: if username and password\n" \
    "                              are embedded in URI, authorization header is\n" \
    "                              always sent to the server regardless of this\n" \
    "                              option.")
#define TEXT_INDEX_OUT                                                  \
  _(" -O, --index-out=INDEX=PATH   Set file path for file with index=INDEX. You can\n" \
    "                              find the file index using the --show-files option.\n" \
    "                              PATH is a relative path to the path specified in\n" \
    "                              --dir option. You can use this option multiple\n" \
    "                              times.")
#define TEXT_DRY_RUN                                                    \
  _(" --dry-run[=true|false]       If true is given, aria2 just checks whether the\n" \
    "                              remote file is available and doesn't download\n" \
    "                              data. This option has effect on HTTP/FTP download.\n" \
    "                              BitTorrent downloads are canceled if true is\n" \
    "                              specified.")
#define TEXT_BT_TRACKER_INTERVAL                                        \
  _(" --bt-tracker-interval=SEC    Set the interval in seconds between tracker\n" \
    "                              requests. This completely overrides interval value\n" \
    "                              and aria2 just uses this value and ignores the\n" \
    "                              min interval and interval value in the response of\n" \
    "                              tracker. If 0 is set, aria2 determines interval\n" \
    "                              based on the response of tracker and the download\n" \
    "                              progress.")
#define TEXT_ON_DOWNLOAD_COMPLETE                                       \
  _(" --on-download-complete=COMMAND Set the command to be executed after download\n" \
    "                              completed.\n"                        \
    "                              See --on-download-start option for the\n" \
    "                              requirement of COMMAND.\n"           \
    "                              See also --on-download-stop option.")
#define TEXT_ON_DOWNLOAD_START                                          \
  _(" --on-download-start=COMMAND  Set the command to be executed after download\n" \
    "                              got started. aria2 passes 3 arguments to COMMAND:\n" \
    "                              GID, the nubmer of files and file path. See Event\n" \
    "                              Hook in man page for more details.")
#define TEXT_ON_DOWNLOAD_PAUSE                                          \
  _(" --on-download-pause=COMMAND  Set the command to be executed after download\n" \
    "                              was paused.\n"\
    "                              See --on-download-start option for the\n" \
    "                              requirement of COMMAND.")
#define TEXT_ON_DOWNLOAD_ERROR                                          \
  _(" --on-download-error=COMMAND  Set the command to be executed after download\n" \
    "                              aborted due to error.\n"              \
    "                              See --on-download-start option for the\n" \
    "                              requirement of COMMAND.\n"           \
    "                              See also --on-download-stop option.")
#define TEXT_ON_DOWNLOAD_STOP                                           \
  _(" --on-download-stop=COMMAND   Set the command to be executed after download\n" \
    "                              stopped. You can override the command to be\n" \
    "                              executed for particular download result using\n" \
    "                              --on-download-complete and --on-download-error. If\n" \
    "                              they are specified, command specified in this\n" \
    "                              option is not executed.\n"           \
    "                              See --on-download-start option for the\n" \
    "                              requirement of COMMAND.")
#define TEXT_BT_STOP_TIMEOUT                                            \
  _(" --bt-stop-timeout=SEC        Stop BitTorrent download if download speed is 0 in\n" \
    "                              consecutive SEC seconds. If 0 is given, this\n" \
    "                              feature is disabled.")
#define TEXT_BT_PRIORITIZE_PIECE                                        \
  _(" --bt-prioritize-piece=head[=SIZE],tail[=SIZE] Try to download first and last\n" \
    "                              pieces of each file first. This is useful for\n" \
    "                              previewing files. The argument can contain 2\n" \
    "                              keywords:head and tail. To include both keywords,\n" \
    "                              they must be separated by comma. These keywords\n" \
    "                              can take one parameter, SIZE. For example, if\n" \
    "                              head=SIZE is specified, pieces in the range of\n" \
    "                              first SIZE bytes of each file get higher priority.\n" \
    "                              tail=SIZE means the range of last SIZE bytes of\n" \
    "                              each file. SIZE can include K or M(1K = 1024, 1M =\n" \
    "                              1024K). If SIZE is omitted, SIZE=1M is used.")
#define TEXT_INTERFACE                                                  \
  _(" --interface=INTERFACE        Bind sockets to given interface. You can specify\n" \
    "                              interface name, IP address and hostname.")
#define TEXT_DISABLE_IPV6                               \
  _(" --disable-ipv6[=true|false]  Disable IPv6.")
#define TEXT_BT_SAVE_METADATA                                           \
  _(" --bt-save-metadata[=true|false] Save metadata as .torrent file. This option has\n" \
    "                              effect only when BitTorrent Magnet URI is used.\n" \
    "                              The filename is hex encoded info hash with suffix\n" \
    "                              .torrent. The directory to be saved is the same\n" \
    "                              directory where download file is saved. If the\n" \
    "                              same file already exists, metadata is not saved.\n" \
    "                              See also --bt-metadata-only option.")
#define TEXT_HTTP_NO_CACHE                      \
  _(" --http-no-cache[=true|false] Send Cache-Control: no-cache and Pragma: no-cache\n" \
    "                              header to avoid cached content.  If false is\n" \
    "                              given, these headers are not sent and you can add\n" \
    "                              Cache-Control header with a directive you like\n" \
    "                              using --header option.")
#define TEXT_BT_METADATA_ONLY                   \
  _(" --bt-metadata-only[=true|false] Download metadata only. The file(s) described\n" \
    "                              in metadata will not be downloaded. This option\n" \
    "                              has effect only when BitTorrent Magnet URI is\n" \
    "                              used. See also --bt-save-metadata option.")
#define TEXT_HUMAN_READABLE                     \
  _(" --human-readable[=true|false] Print sizes and speed in human readable format\n" \
    "                              (e.g., 1.2Ki, 3.4Mi) in the console readout.")
#define TEXT_BT_ENABLE_LPD                      \
  _(" --bt-enable-lpd[=true|false] Enable Local Peer Discovery.")
#define TEXT_BT_LPD_INTERFACE                                           \
  _(" --bt-lpd-interface=INTERFACE Use given interface for Local Peer Discovery. If\n" \
    "                              this option is not specified, the default\n" \
    "                              interface is chosen. You can specify interface\n" \
    "                              name and IP address.")
#define TEXT_REUSE_URI                          \
  _(" --reuse-uri[=true|false]     Reuse already used URIs if no unused URIs are\n" \
    "                              left.")
#define TEXT_ALL_PROXY_USER                                             \
  _(" --all-proxy-user=USER        Set user for --all-proxy option.")
#define TEXT_ALL_PROXY_PASSWD                                           \
  _(" --all-proxy-passwd=PASSWD    Set password for --all-proxy option.")
#define TEXT_HTTP_PROXY_USER                                            \
  _(" --http-proxy-user=USER       Set user for --http-proxy option.")
#define TEXT_HTTP_PROXY_PASSWD                                          \
  _(" --http-proxy-passwd=PASSWD   Set password for --http-proxy option.")
#define TEXT_HTTPS_PROXY_USER                                           \
  _(" --https-proxy-user=USER      Set user for --https-proxy option.")
#define TEXT_HTTPS_PROXY_PASSWD                                         \
  _(" --https-proxy-passwd=PASSWD  Set password for --https-proxy option.")
#define TEXT_FTP_PROXY_USER                                             \
  _(" --ftp-proxy-user=USER        Set user for --ftp-proxy option.")
#define TEXT_FTP_PROXY_PASSWD                                           \
  _(" --ftp-proxy-passwd=PASSWD    Set password for --ftp-proxy option.")
#define TEXT_REMOVE_CONTROL_FILE                \
  _(" --remove-control-file[=true|false] Remove control file before download. Using\n" \
    "                              with --allow-overwrite=true, download always\n" \
    "                              starts from scratch. This will be useful for\n" \
    "                              users behind proxy server which disables resume.")
#define TEXT_ALWAYS_RESUME                      \
  _(" --always-resume[=true|false] Always resume download. If true is given, aria2\n" \
    "                              always tries to resume download and if resume is\n" \
    "                              not possible, aborts download. If false is given,\n" \
    "                              when all given URIs do not support resume or\n" \
    "                              aria2 encounters N URIs which does not support\n" \
    "                              resume (N is the value specified using\n"   \
    "                              --max-resume-failure-tries option), aria2\n" \
    "                              downloads file from scratch.\n"       \
    "                              See --max-resume-failure-tries option.")
#define TEXT_MAX_RESUME_FAILURE_TRIES                                   \
  _(" --max-resume-failure-tries=N When used with --always-resume=false, aria2\n" \
    "                              downloads file from scratch when aria2 detects N\n" \
    "                              number of URIs that does not support resume. If N\n" \
    "                              is 0, aria2 downloads file from scratch when all\n" \
    "                              given URIs do not support resume.\n" \
    "                              See --always-resume option.")
#define TEXT_BT_TRACKER_TIMEOUT                                 \
  _(" --bt-tracker-timeout=SEC     Set timeout in seconds.")
#define TEXT_BT_TRACKER_CONNECT_TIMEOUT                                 \
  _(" --bt-tracker-connect-timeout=SEC Set the connect timeout in seconds to\n" \
    "                              establish connection to tracker. After the\n" \
    "                              connection is established, this option makes no\n" \
    "                              effect and --bt-tracker-timeout option is used\n" \
    "                              instead.")
#define TEXT_DHT_MESSAGE_TIMEOUT                \
  _(" --dht-message-timeout=SEC    Set timeout in seconds.")
#define TEXT_HTTP_ACCEPT_GZIP                   \
  _(" --http-accept-gzip[=true|false] Send 'Accept: deflate, gzip' request header\n" \
    "                              and inflate response if remote server responds\n" \
    "                              with 'Content-Encoding: gzip' or\n"  \
    "                              'Content-Encoding: deflate'.")
#define TEXT_SAVE_SESSION                       \
  _(" --save-session=FILE          Save error/unfinished downloads to FILE on exit.\n" \
    "                              You can pass this output file to aria2c with -i\n" \
    "                              option on restart. Please note that downloads\n" \
    "                              added by aria2.addTorrent and aria2.addMetalink\n" \
    "                              RPC method and whose metadata could not be saved\n" \
    "                              as a file will not be saved. Downloads removed\n" \
    "                              using aria2.remove and aria2.forceRemove will not\n" \
    "                              be saved.")
#define TEXT_MAX_CONNECTION_PER_SERVER          \
  _(" -x, --max-connection-per-server=NUM The maximum number of connections to one\n" \
    "                              server for each download.")
#define TEXT_MIN_SPLIT_SIZE                     \
  _(" -k, --min-split-size=SIZE    aria2 does not split less than 2*SIZE byte range.\n" \
    "                              For example, let's consider downloading 20MiB\n" \
    "                              file. If SIZE is 10M, aria2 can split file into 2\n" \
    "                              range [0-10MiB) and [10MiB-20MiB) and download it\n" \
    "                              using 2 sources(if --split >= 2, of course).\n" \
    "                              If SIZE is 15M, since 2*15M > 20MiB, aria2 does\n" \
    "                              not split file and download it using 1 source.\n" \
    "                              You can append K or M(1K = 1024, 1M = 1024K).")
#define TEXT_CONDITIONAL_GET                    \
  _(" --conditional-get[=true|false] Download file only when the local file is older\n" \
    "                              than remote file. Currently, this function has\n" \
    "                              many limitations. See man page for details.")
#define TEXT_ON_BT_DOWNLOAD_COMPLETE            \
  _(" --on-bt-download-complete=COMMAND For BitTorrent, a command specified in\n" \
    "                              --on-download-complete is called after download\n" \
    "                              completed and seeding is over. On the other hand,\n" \
    "                              this option sets the command to be executed after\n" \
    "                              download completed but before seeding.\n" \
    "                              See --on-download-start option for the\n" \
    "                              requirement of COMMAND.")
#define TEXT_ENABLE_ASYNC_DNS6                  \
  _(" --enable-async-dns6[=true|false] Enable IPv6 name resolution in asynchronous\n" \
    "                              DNS resolver. This option will be ignored when\n" \
    "                              --async-dns=false.")
#define TEXT_ENABLE_DHT6                        \
  _(" --enable-dht6[=true|false]   Enable IPv6 DHT functionality.\n" \
    "                              Use --dht-listen-port option to specify port\n" \
    "                              number to listen on. See also --dht-listen-addr6\n" \
    "                              option.")
#define TEXT_DHT_LISTEN_ADDR6                   \
  _(" --dht-listen-addr6=ADDR      Specify address to bind socket for IPv6 DHT. \n" \
    "                              It should be a global unicast IPv6 address of the\n" \
    "                              host.")
#define TEXT_DHT_ENTRY_POINT6                   \
  _(" --dht-entry-point6=HOST:PORT Set host and port as an entry point to IPv6 DHT\n" \
    "                              network.")
#define TEXT_DHT_FILE_PATH6                     \
  _(" --dht-file-path6=PATH        Change the IPv6 DHT routing table file to PATH.")
#define TEXT_BT_TRACKER                                                 \
  _(" --bt-tracker=URI[,...]       Comma separated list of additional BitTorrent\n" \
    "                              tracker's announce URI. These URIs are not\n" \
    "                              affected by --bt-exclude-tracker option because\n" \
    "                              they are added after URIs in --bt-exclude-tracker\n" \
    "                              option are removed.")
#define TEXT_BT_EXCLUDE_TRACKER                                         \
  _(" --bt-exclude-tracker=URI[,...] Comma separated list of BitTorrent tracker's\n" \
    "                              announce URI to remove. You can use special value\n" \
    "                              '*' which matches all URIs, thus removes all\n" \
    "                              announce URIs. When specifying '*' in shell\n" \
    "                              command-line, don't forget to escape or quote it.\n" \
    "                              See also --bt-tracker option.")
#define TEXT_MAX_DOWNLOAD_RESULT                \
  _(" --max-download-result=NUM    Set maximum number of download result kept in\n" \
    "                              memory. The download results are completed/error/\n" \
    "                              removed downloads. The download results are stored\n" \
    "                              in FIFO queue and it can store at most NUM\n" \
    "                              download results. When queue is full and new\n" \
    "                              download result is created, oldest download result\n" \
    "                              is removed from the front of the queue and new one\n" \
    "                              is pushed to the back. Setting big number in this\n" \
    "                              option may result high memory consumption after\n" \
    "                              thousands of downloads. Specifying 0 means no\n" \
    "                              download result is kept.")
#define TEXT_ASYNC_DNS_SERVER                   \
  _(" --async-dns-server=IPADDRESS[,...] Comma separated list of DNS server address\n" \
    "                              used in asynchronous DNS resolver. Usually\n" \
    "                              asynchronous DNS resolver reads DNS server\n" \
    "                              addresses from /etc/resolv.conf. When this option\n" \
    "                              is used, it uses DNS servers specified in this\n" \
    "                              option instead of ones in /etc/resolv.conf. You\n" \
    "                              can specify both IPv4 and IPv6 address. This\n" \
    "                              option is useful when the system does not have\n" \
    "                              /etc/resolv.conf and user does not have the\n" \
    "                              permission to create it.")
#define TEXT_ENABLE_RPC                                             \
  _(" --enable-rpc[=true|false]    Enable JSON-RPC/XML-RPC server.\n"   \
    "                              It is strongly recommended to set username and\n" \
    "                              password using --rpc-user and --rpc-passwd\n" \
    "                              option. See also --rpc-listen-port option.")
#define TEXT_RPC_MAX_REQUEST_SIZE                                   \
  _(" --rpc-max-request-size=SIZE  Set max size of JSON-RPC/XML-RPC request. If aria2\n" \
    "                              detects the request is more than SIZE bytes, it\n" \
    "                              drops connection.")
#define TEXT_RPC_USER                               \
  _(" --rpc-user=USER              Set JSON-RPC/XML-RPC user.")
#define TEXT_RPC_PASSWD                                     \
  _(" --rpc-passwd=PASSWD          Set JSON-RPC/XML-RPC password.")
#define TEXT_RPC_LISTEN_ALL                                         \
  _(" --rpc-listen-all[=true|false] Listen incoming JSON-RPC/XML-RPC requests on all\n" \
    "                              network interfaces. If false is given, listen only\n" \
    "                              on local loopback interface.")
#define TEXT_RPC_LISTEN_PORT                                        \
  _(" --rpc-listen-port=PORT       Specify a port number for JSON-RPC/XML-RPC server\n" \
    "                              to listen to.")
#define TEXT_SHOW_CONSOLE_READOUT                                       \
  _(" --show-console-readout[=true|false] Show console readout.")
#define TEXT_METALINK_BASE_URI                  \
  _(" --metalink-base-uri=URI      Specify base URI to resolve relative URI in\n" \
    "                              metalink:url and metalink:metaurl element in a\n" \
    "                              metalink file stored in local disk. If URI points\n" \
    "                              to a directory, URI must end with '/'.")
#define TEXT_STREAM_PIECE_SELECTOR              \
  _(" --stream-piece-selector=SELECTOR Specify piece selection algorithm\n" \
    "                              used in HTTP/FTP download. Piece means fixed\n" \
    "                              length segment which is downloaded in parallel\n" \
    "                              in segmented download. If 'default' is given,\n" \
    "                              aria2 selects piece so that it reduces the\n" \
    "                              number of establishing connection. This is\n" \
    "                              reasonable default behaviour because\n" \
    "                              establishing connection is an expensive\n" \
    "                              operation.\n"                        \
    "                              If 'inorder' is given, aria2 selects piece which\n" \
    "                              has minimum index. Index=0 means first of the\n" \
    "                              file. This will be useful to view movie while\n" \
    "                              downloading it. --enable-http-pipelining option\n" \
    "                              may be useful to reduce reconnection overhead.\n" \
    "                              Please note that aria2 honors\n"     \
    "                              --min-split-size option, so it will be necessary\n" \
    "                              to specify a reasonable value to\n"  \
    "                              --min-split-size option.\n"          \
    "                              If 'geom' is given, at the beginning aria2\n" \
    "                              selects piece which has minimum index like\n" \
    "                              'inorder', but it exponentially increasingly\n" \
    "                              keeps space from previously selected piece. This\n" \
    "                              will reduce the number of establishing connection\n" \
    "                              and at the same time it will download the\n" \
    "                              beginning part of the file first. This will be\n" \
    "                              useful to view movie while downloading it.")
#define TEXT_TRUNCATE_CONSOLE_READOUT                                   \
  _(" --truncate-console-readout[=true|false] Truncate console readout to fit in\n"\
    "                              a single line.")
#define TEXT_PAUSE                              \
  _(" --pause[=true|false]         Pause download after added. This option is\n" \
    "                              effective only when --enable-rpc=true is given.")
#define TEXT_RPC_ALLOW_ORIGIN_ALL                                       \
  _(" --rpc-allow-origin-all[=true|false] Add Access-Control-Allow-Origin header\n" \
    "                              field with value '*' to the RPC response.")
#define TEXT_DOWNLOAD_RESULT                    \
  _(" --download-result=OPT        This option changes the way \"Download Results\"\n" \
    "                              is formatted. If OPT is 'default', print GID,\n" \
    "                              status, average download speed and path/URI. If\n" \
    "                              multiple files are involved, path/URI of first\n" \
    "                              requested file is printed and remaining ones are\n" \
    "                              omitted.\n"                          \
    "                              If OPT is 'full', print GID, status, average\n" \
    "                              download speed, percentage of progress and\n" \
    "                              path/URI. The percentage of progress and\n" \
    "                              path/URI are printed for each requested file in\n" \
    "                              each row.")
#define TEXT_HASH_CHECK_ONLY                    \
  _(" --hash-check-only[=true|false] If true is given, after hash check using\n" \
    "                              --check-integrity option, abort download whether\n" \
    "                              or not download is complete.")
#define TEXT_CHECKSUM                                                   \
  _(" --checksum=TYPE=DIGEST       Set checksum. TYPE is hash type. The supported\n" \
    "                              hash type is listed in \"Hash Algorithms\" in\n" \
    "                              \"aria2c -v\". DIGEST is hex digest.\n" \
    "                              For example, setting sha-1 digest looks like\n" \
    "                              this:\n"                             \
    "                              sha-1=0192ba11326fe2298c8cb4de616f4d4140213838\n" \
    "                              This option applies only to HTTP(S)/FTP\n" \
    "                              downloads.")
#define TEXT_PIECE_LENGTH                       \
  _(" --piece-length=LENGTH        Set a piece length for HTTP/FTP downloads. This\n" \
    "                              is the boundary when aria2 splits a file. All\n" \
    "                              splits occur at multiple of this length. This\n" \
    "                              option will be ignored in BitTorrent downloads.\n" \
    "                              It will be also ignored if Metalink file\n" \
    "                              contains piece hashes.")
#define TEXT_STOP_WITH_PROCESS                                          \
  _(" --stop-with-process=PID      Stop application when process PID is not running.\n" \
    "                              This is useful if aria2 process is forked from a\n" \
    "                              parent process. The parent process can fork aria2\n" \
    "                              with its own pid and when parent process exits\n" \
    "                              for some reason, aria2 can detect it and shutdown\n" \
    "                              itself.")
#define TEXT_DEFERRED_INPUT                     \
  _(" --deferred-input[=true|false] If true is given, aria2 does not read all URIs\n" \
    "                              and options from file specified by -i option at\n" \
    "                              startup, but it reads one by one when it needs\n" \
    "                              later. This may reduce memory usage if input\n" \
    "                              file contains a lot of URIs to download.\n" \
    "                              If false is given, aria2 reads all URIs and\n" \
    "                              options at startup.")
#define TEXT_BT_REMOVE_UNSELECTED_FILE                                  \
  _(" --bt-remove-unselected-file[=true|false] Removes the unselected files when\n" \
    "                              download is completed in BitTorrent. To\n" \
    "                              select files, use --select-file option. If\n" \
    "                              it is not used, all files are assumed to be\n" \
    "                              selected. Please use this option with care\n" \
    "                              because it will actually remove files from\n" \
    "                              your disk.")
#define TEXT_ENABLE_MMAP                        \
  _(" --enable-mmap[=true|false]   Map files into memory.")
