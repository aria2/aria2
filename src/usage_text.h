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
_(" -s, --split=N                Download a file using N connections. N must be\n"\
  "                              between 1 and 5. This option affects all URLs.\n"\
  "                              Thus, aria2 connects to each URL with\n"\
  "                              N connections.\n"\
  "                              Default: 1")
#define TEXT_RETRY_WAIT \
_(" --retry-wait=SEC             Set the seconds to wait to retry after an error\n"\
  "                              has occured. Specify a value between 0 and 60.\n"\
  "                              Default: 5")
#define TEXT_TIMEOUT \
_(" -t, --timeout=SEC            Set timeout in seconds. Default: 60")
#define TEXT_MAX_TRIES \
_(" -m, --max-tries=N            Set number of tries. 0 means unlimited.\n"\
  "                              Default: 5")
#define TEXT_HTTP_PROXY \
_(" --http-proxy=HOST:PORT       Use HTTP proxy server. This affects all URLs.")
#define TEXT_HTTP_USER \
_(" --http-user=USER             Set HTTP user. This affects all URLs.")
#define TEXT_HTTP_PASSWD \
_(" --http-passwd=PASSWD         Set HTTP password. This affects all URLs.")
#define TEXT_HTTP_PROXY_USER \
_(" --http-proxy-user=USER       Set HTTP proxy user. This affects all URLs.")
#define TEXT_HTTP_PROXY_PASSWD \
_(" --http-proxy-passwd=PASSWD   Set HTTP proxy password. This affects all URLs.")
#define TEXT_HTTP_PROXY_METHOD \
_(" --http-proxy-method=METHOD   Set the method to use in proxy request.\n"\
  "                              METHOD is either 'get' or 'tunnel'.\n"\
  "                              Default: tunnel")
#define TEXT_HTTP_AUTH_SCHEME \
_(" --http-auth-scheme=SCHEME    Set HTTP authentication scheme. Currently, basic\n"\
  "                              is the only supported scheme.\n"\
  "                              Default: basic")
#define TEXT_REFERER \
_(" --referer=REFERER            Set Referer. This affects all URLs.")
#define TEXT_FTP_USER \
_(" --ftp-user=USER              Set FTP user. This affects all URLs.\n"\
  "                              Default: anonymous")
#define TEXT_FTP_PASSWD \
_(" --ftp-passwd=PASSWD          Set FTP password. This affects all URLs.\n"\
  "                              Default: ARIA2USER@")
#define TEXT_FTP_TYPE \
_(" --ftp-type=TYPE              Set FTP transfer type. TYPE is either 'binary'\n"\
  "                              or 'ascii'.\n"\
  "                              Default: binary")
#define TEXT_FTP_PASV \
_(" -p, --ftp-pasv               Use passive mode in FTP.")
#define TEXT_FTP_VIA_HTTP_PROXY \
_(" --ftp-via-http-proxy=METHOD  Use HTTP proxy in FTP. METHOD is either 'get' or\n"\
  "                              'tunnel'.\n"\
  "                              Default: tunnel")
#define TEXT_LOWEST_SPEED_LIMIT \
_(" --lowest-speed-limit=SPEED   Close connection if download speed is lower than\n"\
  "                              or equal to this value(bytes per sec).\n"\
  "                              0 means aria2 does not have a lowest speed limit.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).\n"\
  "                              This option does not affect BitTorrent downloads.\n"\
  "                              Default: 0")
#define TEXT_MAX_DOWNLOAD_LIMIT \
_(" --max-download-limit=SPEED   Set max download speed in bytes per sec.\n"\
  "                              0 means unrestricted.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).\n"\
  "                              Default: 0")
#define TEXT_FILE_ALLOCATION \
_(" --file-allocation=METHOD     Specify file allocation method. METHOD is either\n"\
  "                              'none' or 'prealloc'. 'none' doesn't pre-allocate\n"\
  "                              file space. 'prealloc' pre-allocates file space\n"\
  "                              before download begins. This may take some time\n"\
  "                              depending on the size of the file.\n"\
  "                              Default: prealloc")
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
  "                              doesn't exist.\n"\
  "                              Default: false")
#define TEXT_ALLOW_PIECE_LENGTH_CHANGE \
_(" --allow-piece-length-change=true|false If false is given, aria2 aborts download\n"\
  "                              when a piece length is different from one in\n"\
  "                              a control file. If true is given, you can proceed\n"\
  "                              but some download progress will be lost.")  
#define TEXT_FORCE_SEQUENTIAL \
_(" -Z, --force-sequential[=true|false] Fetch URIs in the command-line sequentially\n"\
  "                              and download each URI in a separate session, like\n"\
  "                              the usual command-line download utilities.\n"\
  "                              Default: false")
#define TEXT_AUTO_FILE_RENAMING \
_(" --auto-file-renaming[=true|false] Rename file name if the same file already\n"\
  "                              exists. This option works only in http(s)/ftp\n"\
  "                              download.\n"\
  "                              The new file name has a dot and a number(1..9999)\n"\
  "                              appended.\n"\
  "                              Default: true")
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
  "                              required.\n"\
  "                              Default: false")
#define TEXT_ENABLE_HTTP_KEEP_ALIVE \
_(" --enable-http-keep-alive[=true|false] Enable HTTP/1.1 persistent connection.")
#define TEXT_ENABLE_HTTP_PIPELINING \
_(" --enable-http-pipelining[=true|false] Enable HTTP/1.1 pipelining.\n"\
  "                              Default: false")
#define TEXT_CHECK_INTEGRITY \
_(" --check-integrity=true|false  Check file integrity by validating piece hash.\n"\
  "                              This option only affects in BitTorrent downloads\n"\
  "                              and Metalink downloads with chunk checksums.\n"\
  "                              Use this option to re-download a damaged portion\n"\
  "                              of a file.\n"\
  "                              Default: false")
#define TEXT_REALTIME_CHUNK_CHECKSUM \
_(" --realtime-chunk-checksum=true|false  Validate chunk checksum while\n"\
  "                              downloading a file in Metalink mode. This option\n"\
  "                              on affects Metalink mode with chunk checksums.\n"\
  "                              Default: true")
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
  "                              Reads input from stdin when '-' is specified.")
#define TEXT_MAX_CONCURRENT_DOWNLOADS \
_(" -j, --max-concurrent-downloads=N Set maximum number of concurrent downloads.\n"\
  "                              It should be used with the -i option.\n"\
  "                              Default: 5")
#define TEXT_LOAD_COOKIES \
_(" --load-cookies=FILE          Load cookies from FILE. The format of FILE is\n"\
  "                              the same used by Netscape and Mozilla.")
#define TEXT_SHOW_FILES \
_(" -S, --show-files             Print file listing of .torrent or .metalink file\n"\
  "                              and exit. More detailed information will be listed\n"\
  "                              in case of torrent file.")
#define TEXT_SELECT_FILE \
_(" --select-file=INDEX...       Set file to download by specifing its index.\n"\
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
  "                              mentioned in .torrent file.\n"\
  "                              Default: true")
#define TEXT_LISTEN_PORT \
_(" --listen-port=PORT...        Set TCP port number for BitTorrent downloads.\n"\
  "                              Multiple ports can be specified by using ',',\n"\
  "                              for example: \"6881,6885\". You can also use '-'\n"\
  "                              to specify a range: \"6881-6999\". ',' and '-' can\n"\
  "                              be used together.")
#define TEXT_MAX_UPLOAD_LIMIT \
_(" --max-upload-limit=SPEED     Set max upload speed in bytes per sec.\n"\
  "                              0 means unrestricted.\n"\
  "                              You can append K or M(1K = 1024, 1M = 1024K).\n"\
  "                              Default: 0")
#define TEXT_SEED_TIME \
_(" --seed-time=MINUTES          Specify seeding time in minutes. Also see the\n"\
  "                              --seed-ratio option.")
#define TEXT_SEED_RATIO \
_(" --seed-ratio=RATIO           Specify share ratio. Seed completed torrents\n"\
  "                              until share ratio reaches RATIO. 1.0 is\n"\
  "                              encouraged. Specify 0.0 if you intend to do\n"\
  "                              seeding regardless of share ratio.\n"\
  "                              If --seed-time option is specified along with\n"\
  "                              this option, seeding ends when at least one of\n"\
  "                              the conditions is satisfied.")
#define TEXT_PEER_ID_PREFIX \
_(" --peer-id-prefix=PEERI_ID_PREFIX Specify the prefix of peer ID. The peer ID in\n"\
  "                              BitTorrent is 20 byte length. If more than 20\n"\
  "                              bytes are specified, only first 20\n"\
  "                              bytes are used. If less than 20 bytes are\n"\
  "                              specified, the random alphabet characters are\n"\
  "                              added to make it's length 20 bytes.\n"\
  "                              Default: -aria2-")
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
#define TEXT_BT_MIN_CRYPTO_LEVEL \
_(" --bt-min-crypto-level=plain|arc4 Set minimum level of encryption method.\n"\
  "                              If several encryption methods are provided by a\n"\
  "                              peer, aria2 chooses a lowest one which satisfies\n"\
  "                              the given level.")
#define TEXT_BT_REQUIRE_CRYPTO \
_(" --bt-require-crypto=true|false If true is given, aria2 doesn't accept and\n"\
  "                              establish connection with legacy BitTorrent\n"\
  "                              handshake. Thus aria2 always uses Obfuscation\n"\
  "                              handshake.")
#define TEXT_METALINK_FILE \
_(" -M, --metalink-file=METALINK_FILE The file path to the .metalink file.")
#define TEXT_METALINK_SERVERS \
_(" -C, --metalink-servers=NUM_SERVERS The number of servers to connect to\n"\
  "                              simultaneously.")
#define TEXT_METALINK_VERSION \
_(" --metalink-version=VERSION   The version of the file to download.")
#define TEXT_METALINK_LANGUAGE \
_(" --metalink-language=LANGUAGE The language of the file to download.")
#define TEXT_METALINK_OS \
_(" --metalink-os=OS             The operating system of the file to download.")
#define TEXT_METALINK_LOCATION \
_(" --metalink-location=LOCATION[,...] The location of the preferred server.\n"\
  "                              A comma-deliminated list of locations is\n"\
  "                              acceptable.")
#define TEXT_METALINK_PREFERRED_PROTOCOL \
_(" --metalink-preferred-protocol=PROTO Specify preferred protocol. The possible\n"\
  "                              values are 'http', 'https', 'ftp' and 'none'.\n"\
  "                              Specifiy none to disable this feature.")
#define TEXT_FOLLOW_METALINK \
_(" --follow-metalink=true|false|mem If true or mem is specified, when a file\n"\
  "                              whose suffix is .metaink or content type is\n"\
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
  "                              option name using a given word, in forward match\n"\
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
_(" -q, --quiet[=true|false]     Make aria2 quite (no console output).")
#define TEXT_ASYNC_DNS \
_(" --async-dns[=true|false]     Enable asynchronous DNS.")
#define TEXT_FTP_REUSE_CONNECTION \
_(" --ftp-reuse-connection[=true|false] Reuse connection in FTP.")
