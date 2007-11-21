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
#include "common.h"
#include "FeatureConfig.h"
#ifdef ENABLE_MESSAGE_DIGEST
# include "messageDigest.h"
#endif // ENABLE_MESSAGE_DIGEST

void showVersion() {
  cout << PACKAGE << _(" version ") << PACKAGE_VERSION << "\n"
       << "Copyright (C) 2006, 2007 Tatsuhiro Tsujikawa" << "\n"
       << "\n"
       <<
    _("This program is free software; you can redistribute it and/or modify\n"
      "it under the terms of the GNU General Public License as published by\n"
      "the Free Software Foundation; either version 2 of the License, or\n"
      "(at your option) any later version.\n"
      "\n"
      "This program is distributed in the hope that it will be useful,\n"
      "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
      "GNU General Public License for more details.\n"
      "\n"
      "You should have received a copy of the GNU General Public License\n"
      "along with this program; if not, write to the Free Software\n"
      "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA\n")
       << "\n"

       <<
    "In addition, as a special exception, the copyright holders give\n"
    "permission to link the code of portions of this program with the\n"
    "OpenSSL library under certain conditions as described in each\n"
    "individual source file, and distribute linked combinations\n"
    "including the two.\n"
    "You must obey the GNU General Public License in all respects\n"
    "for all of the code used other than OpenSSL.  If you modify\n"
    "file(s) with this exception, you may extend this exception to your\n"
    "version of the file(s), but you are not obligated to do so.  If you\n"
    "do not wish to do so, delete this exception statement from your\n"
    "version.  If you delete this exception statement from all source\n"
    "files in the program, then also delete it here.\n"
       << "\n"
       << "** Configuration **" << "\n"
       << FeatureConfig::getInstance()->getConfigurationSummary()
#ifdef ENABLE_MESSAGE_DIGEST
       << "message digest algorithms: " << MessageDigestContext::getSupportedAlgoString() << "\n"
#endif // ENABLE_MESSAGE_DIGEST
       << "\n"
       << _("Contact Info:") << "\n"
       << "Tatsuhiro Tsujikawa <tujikawa at users dot sourceforge dot net>"
       << endl;
}

void showUsage() {
  printf(_("Usage: %s [options] URL ...\n"), PACKAGE_NAME);
#ifdef ENABLE_BITTORRENT
  printf(_("       %s [options] -T TORRENT_FILE URL ...\n"), PACKAGE_NAME);
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  printf(_("       %s [options] -M METALINK_FILE\n"), PACKAGE_NAME);
#endif // ENABLE_METALINK
  cout << endl;
  cout << _("Options:") << endl;
  cout << _(" -d, --dir=DIR                The directory to store the downloaded file.") << endl;
  cout << _(" -o, --out=FILE               The file name of the downloaded file.") << endl;
  cout << _(" -l, --log=LOG                The file name of the log file. If '-' is\n"
	    "                              specified, log is written to stdout.") << endl;
#ifdef HAVE_DAEMON
  cout << _(" -D, --daemon                 Run as daemon.") << endl;
#endif // HAVE_DAEMON
  cout << _(" -s, --split=N                Download a file using N connections. N must be\n"
	    "                              between 1 and 5. This option affects all URLs.\n"
	    "                              Thus, aria2 connects to each URL with\n"
	    "                              N connections.\n"
	    "                              Default: 1") << endl;
  cout << _(" --retry-wait=SEC             Set the seconds to wait to retry after an error\n"
	    "                              has occured. Specify a value between 0 and 60.\n"
	    "                              Default: 5") << endl;
  cout << _(" -t, --timeout=SEC            Set timeout in seconds. Default: 60") << endl;
  cout << _(" -m, --max-tries=N            Set number of tries. 0 means unlimited.\n"
	    "                              Default: 5") << endl;
  /*
  cout << _(" --min-segment-size=SIZE[K|M] Set minimum segment size. You can append\n"
	    "                              K or M(1K = 1024, 1M = 1024K). This\n"
	    "                              value must be greater than or equal to\n"
	    "                              1024. Default: 1M") << endl;
  */
  cout << _(" --http-proxy=HOST:PORT       Use HTTP proxy server. This affects all URLs.") << endl;
  cout << _(" --http-user=USER             Set HTTP user. This affects all URLs.") << endl;
  cout << _(" --http-passwd=PASSWD         Set HTTP password. This affects all URLs.") << endl;
  cout << _(" --http-proxy-user=USER       Set HTTP proxy user. This affects all URLs.") << endl;
  cout << _(" --http-proxy-passwd=PASSWD   Set HTTP proxy password. This affects all URLs.") << endl;
  cout << _(" --http-proxy-method=METHOD   Set the method to use in proxy request.\n"
	    "                              METHOD is either 'get' or 'tunnel'.\n"
	    "                              Default: tunnel") << endl;
  cout << _(" --http-auth-scheme=SCHEME    Set HTTP authentication scheme. Currently, basic\n"
	    "                              is the only supported scheme.\n"
	    "                              Default: basic") << endl;
  cout << _(" --referer=REFERER            Set Referer. This affects all URLs.") << endl;
  cout << _(" --ftp-user=USER              Set FTP user. This affects all URLs.\n"
	    "                              Default: anonymous") << endl;
  cout << _(" --ftp-passwd=PASSWD          Set FTP password. This affects all URLs.\n"
	    "                              Default: ARIA2USER@") << endl;
  cout << _(" --ftp-type=TYPE              Set FTP transfer type. TYPE is either 'binary'\n"
	    "                              or 'ascii'.\n"
	    "                              Default: binary") << endl;
  cout << _(" -p, --ftp-pasv               Use passive mode in FTP.") << endl;
  cout << _(" --ftp-via-http-proxy=METHOD  Use HTTP proxy in FTP. METHOD is either 'get' or\n"
	    "                              'tunnel'.\n"
	    "                              Default: tunnel") << endl;
  cout << _(" --lowest-speed-limit=SPEED   Close connection if download speed is lower than\n"
	    "                              or equal to this value(bytes per sec).\n"
	    "                              0 means aria2 does not have a lowest speed limit.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"

	    "                              This option does not affect BitTorrent downloads.\n"
	    "                              Default: 0") << endl;
  cout << _(" --max-download-limit=SPEED   Set max download speed in bytes per sec.\n"
	    "                              0 means unrestricted.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"
	    "                              Default: 0") << endl;
  cout << _(" --file-allocation=METHOD     Specify file allocation method. METHOD is either\n"
	    "                              'none' or 'prealloc'. 'none' doesn't pre-allocate\n"
	    "                              file space. 'prealloc' pre-allocates file space\n"
	    "                              before download begins. This may take some time\n"
	    "                              depending on the size of the file.\n"
	    "                              Default: prealloc") << endl;
  cout << _(" --no-file-allocation-limit=SIZE No file allocation is made for files whose\n"
	    "                              size is smaller than SIZE.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"
	    "                              BitTorrent downloads ignore this option.\n"
	    "                              Default: 5M") << endl;
  cout << _(" --allow-overwrite=true|false If false, aria2 doesn't download a file which\n"
  		"                              already exists but the corresponding .aria2 file\n"
  		"                              doesn't exist.\n"
            "                              Default: false") << endl;
  cout << _(" -Z, --force-sequential[=true|false] Fetch URIs in the command-line sequentially\n"
	    "                              and download each URI in a separate session, like\n"
	    "                              the usual command-line download utilities.\n"
	    "                              Default: false") << endl;
  cout << _(" --auto-file-renaming[=true|false] Rename file name if the same file already\n"
	    "                              exists. This option works only in http(s)/ftp\n"
	    "                              download.\n"
	    "                              The new file name has a dot and a number(1..9999)\n"
	    "                              appended.\n"
	    "                              Default: true") << endl;
  cout << _(" -P, --parameterized-uri[=true|false] Enable parameterized URI support.\n"
	    "                              You can specify set of parts:\n"
	    "                              http://{sv1,sv2,sv3}/foo.iso\n"
	    "                              Also you can specify numeric sequences with step\n"
	    "                              counter:\n"
	    "                              http://host/image[000-100:2].img\n"
	    "                              A step counter can be omitted.\n"
	    "                              If all URIs do not point to the same file, such\n"
	    "                              as the second example above, -Z option is\n"
	    "                              required.\n"
	    "                              Default: false") << endl;
  cout << _(" --enable-http-keep-alive[=true|false] Enable HTTP/1.1 persistent connection.\n"
	    "                              Default: false") << endl;
  cout << _(" --enable-http-pipelining[=true|false] Enable HTTP/1.1 pipelining.\n"
	    "                              Default: false") << endl;
#ifdef ENABLE_MESSAGE_DIGEST
  cout << _(" --check-integrity=true|false  Check file integrity by validating piece hash.\n"
	    "                              This option only affects in BitTorrent downloads\n"
	    "                              and Metalink downloads with chunk checksums.\n"
	    "                              Use this option to re-download a damaged portion\n"
	    "                              of a file.\n"
	    "                              Default: false") << endl;
  cout << _(" --realtime-chunk-checksum=true|false  Validate chunk checksum while\n"
	    "                              downloading a file in Metalink mode. This option\n"
	    "                              on affects Metalink mode with chunk checksums.\n"
	    "                              Default: true") << endl;
#endif // ENABLE_MESSAGE_DIGEST
  cout << _(" -c, --continue               Continue downloading a partially downloaded\n"
	    "                              file. Use this option to resume a download\n"
	    "                              started by a web browser or another program\n"
	    "                              which downloads files sequentially from the\n"
	    "                              beginning. Currently this option is only\n"
	    "                              applicable to http(s)/ftp downloads.") << endl;
  cout << _(" -U, --user-agent=USER_AGENT  Set user agent for http(s) downloads.") << endl;
  cout << _(" -n, --no-netrc               Disables netrc support.") << endl;
  cout << _(" -i, --input-file=FILE        Downloads URIs found in FILE. You can specify\n"
	    "                              multiple URIs for a single entity: separate\n"
	    "                              URIs on a single line using the TAB character.\n"
	    "                              Reads input from stdin when '-' is specified.") << endl;
  cout << _(" -j, --max-concurrent-downloads=N Set maximum number of concurrent downloads.\n"
	    "                              It should be used with the -i option.\n"
	    "                              Default: 5") << endl;
  cout << _(" --load-cookies=FILE          Load cookies from FILE. The format of FILE is\n"
	    "                              the same used by Netscape and Mozilla.") << endl;
#if defined ENABLE_BITTORRENT || ENABLE_METALINK
  cout << _(" -S, --show-files             Print file listing of .torrent or .metalink file\n"
	    "                              and exit. More detailed information will be listed\n"
	    "                              in case of torrent file.") << endl;
  cout << _(" --select-file=INDEX...       Set file to download by specifing its index.\n"
	    "                              You can find the file index using the\n"
	    "                              --show-files option. Multiple indexes can be\n"
	    "                              specified by using ',', for example: \"3,6\".\n"
	    "                              You can also use '-' to specify a range: \"1-5\".\n"
	    "                              ',' and '-' can be used together.\n"
	    "                              When used with the -M option, index may vary\n"
	    "                              depending on the query(see --metalink-* options).") << endl;
#endif // ENABLE_BITTORRENT || ENABLE_METALINK
#ifdef ENABLE_BITTORRENT
  cout << _(" -T, --torrent-file=TORRENT_FILE  The path to the .torrent file.") << endl;
  cout << _(" --follow-torrent=true|false  Set to false to prevent aria2 from\n"
	    "                              entering BitTorrent mode even if the filename of\n"
	    "                              the downloaded file ends with .torrent.\n"
	    "                              Default: true") << endl;
  cout << _(" --direct-file-mapping=true|false Directly read from and write to each file\n"
	    "                              mentioned in .torrent file.\n"
	    "                              Default: true") << endl;
  cout << _(" --listen-port=PORT...        Set TCP port number for BitTorrent downloads.\n"
	    "                              Multiple values can be specified by using ',',\n"
	    "                              for example: \"6881,6885\".\n"
	    "                              You can also use '-' to specify a range: \"6881-6999\".\n"
	    "                              ',' and '-' can be used together.\n"
	    "                              Default: 6881-6999") << endl;
  cout << _(" --max-upload-limit=SPEED     Set max upload speed in bytes per sec.\n"
	    "                              0 means unrestricted.\n"
	    "                              You can append K or M(1K = 1024, 1M = 1024K).\n"
	    "                              Default: 0") << endl;
  cout << _(" --seed-time=MINUTES          Specify seeding time in minutes. Also see the\n"
	    "                              --seed-ratio option.") << endl;
  cout << _(" --seed-ratio=RATIO           Specify share ratio. Seed completed torrents\n"
	    "                              until share ratio reaches RATIO. 1.0 is\n"
	    "                              encouraged. Specify 0.0 if you intend to do\n"
	    "                              seeding regardless of share ratio.\n"
	    "                              If --seed-time option is specified along with\n"
	    "                              this option, seeding ends when at least one of\n"
	    "                              the conditions is satisfied.\n"
	    "                              Default: 1.0") << endl;
  cout << _(" --peer-id-prefix=PEERI_ID_PREFIX Specify the prefix of peer ID. The peer ID in\n"
	    "                              in BitTorrent is 20 byte length. If more than 20\n"
	    "                              bytes are specified, only first 20\n"
	    "                              bytes are used. If less than 20 bytes are\n"
	    "                              specified, the random alphabet characters are\n"
	    "                              added to make it's length 20 bytes.\n"
	    "                              Default: -aria2-") << endl;
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  cout << _(" -M, --metalink-file=METALINK_FILE The file path to the .metalink file.") << endl;
  cout << _(" -C, --metalink-servers=NUM_SERVERS The number of servers to connect to\n"
	    "                              simultaneously.\n"
	    "                              Default: 5") << endl;
  cout << _(" --metalink-version=VERSION   The version of the file to download.") << endl;
  cout << _(" --metalink-language=LANGUAGE The language of the file to download.") << endl;
  cout << _(" --metalink-os=OS             The operating system of the file to download.") << endl;
  cout << _(" --metalink-location=LOCATION[,...] The location of the preferred server.\n"
	    "                              A comma-deliminated list of locations is\n"
	    "                              acceptable.") << endl;
  cout << _(" --follow-metalink=true|false Set to false to prevent aria2 from\n"
	    "                              entering Metalink mode even if the filename of\n"
	    "                              the downloaded file ends with .metalink.\n"
	    "                              Default: true") << endl;
#endif // ENABLE_METALINK
  cout << _(" -v, --version                Print the version number and exit.") << endl;
  cout << _(" -h, --help                   Print this message and exit.") << endl;
  cout << endl;
  cout << "URL:" << endl;
  cout << _(" You can specify multiple URLs. Unless you specify -Z option, all URLs must\n"
	    " point to the same file or downloading will fail.") << endl;
  cout << _(" You can specify both torrent file with -T option and URLs. By doing this,\n"
	    " download a file from both torrent swarm and http/ftp server at the same time,\n"
	    " while the data from http/ftp are uploaded to the torrent swarm. Note that\n"
	    " only single file torrent can be integrated with http/ftp.") << endl;
  cout << endl;
  cout << _("Examples:") << endl;
  cout << _(" Download a file using 1 connection:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip" << endl;
  cout << _(" Download a file using 2 connections:") << endl;
  cout << "  aria2c -s 2 http://AAA.BBB.CCC/file.zip" << endl;
  cout << _(" Download a file using 2 connections, each connects to a different server:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip http://DDD.EEE.FFF/GGG/file.zip" << endl;
  cout << _(" You can mix up different protocols:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.zip ftp://DDD.EEE.FFF/GGG/file.zip" << endl;
  cout << _(" Parameterized URI:") << endl;
  cout << "  aria2c -P http://{server1,server2,server3}/file.iso" << endl;
  cout << _(" Parameterized URI. -Z option is required in this case:") << endl;
  cout << "  aria2c -P -Z http://host/file[001-100:2].img" << endl;
#ifdef ENABLE_BITTORRENT
  cout << endl;
  cout << _(" Download a torrent:") << endl;
  cout << "  aria2c -o test.torrent http://AAA.BBB.CCC/file.torrent" << endl;
  cout << _(" Download a torrent using a local .torrent file:") << endl;
  cout << "  aria2c -T test.torrent" << endl;
  cout << _(" Download only selected files:") << endl;
  cout << "  aria2c -T test.torrent dir/file1.zip dir/file2.zip" << endl;
  cout << _(" Print file listing of .torrent file:") << endl;
  cout << "  aria2c -T test.torrent -S" << endl;
  cout << _(" Download a file using torrent and http/ftp server") << endl;
  cout << "  aria2c -T test.torrent http://host1/file ftp://host2/file" << endl;
#endif // ENABLE_BITTORRENT
#ifdef ENABLE_METALINK
  cout << endl;
  cout << _(" Metalink downloading:") << endl;
  cout << "  aria2c http://AAA.BBB.CCC/file.metalink" << endl;
  cout << _(" Download a file using local .metalink file:") << endl;
  cout << "  aria2c -M test.metalink" << endl;

  cout << _(" Download a file using a local .metalink file with preferred server locations:") << endl;
  cout << "  aria2c -M test.metalink --metalink-location=JP,US" << endl;
  cout << _(" Metalink downloading with preferences:") << endl;
  cout << "  aria2c -M test.metalink --metalink-version=1.1.1 --metalink-language=en-US" << endl;
  cout << _(" Download only selected files:") << endl;
  cout << "  aria2c -M test.metalink --metalink-language=en-US dir/file1.zip dir/file2.zip" << endl;
  cout << _(" Download only selected files using index:") << endl;
  cout << "  aria2c -M test.metalink --metalink-language=en-US --select-file 1,3-5" << endl;
  cout << _(" Print file listing of .metalink file:") << endl;
  cout << "  aria2c -M test.metalink -S --metalink-language=en-US" << endl;
#endif // ENABLE_METALINK
  cout << endl;
  printf(_("Report bugs to %s"), "<tujikawa at users dot sourceforge dot net>");
  cout << endl;
}
