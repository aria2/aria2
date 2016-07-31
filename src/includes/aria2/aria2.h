/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2013 Tatsuhiro Tsujikawa
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
#ifndef ARIA2_H
#define ARIA2_H

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>

#include <string>
#include <vector>

// Libaria2: The aim of this library is provide same functionality
// available in RPC methods. The function signatures are not
// necessarily the same, because we can take advantage of the direct,
// no latency, access to the aria2 core.
//
// Therefore, this library is not meant to be the fine-grained,
// customizable, complete HTTP/FTP/BitTorrent library. If you are
// looking for such library for HTTP/FTP access, consider libcurl.

namespace aria2 {

/**
 * @struct
 *
 * This object identifies aria2 session. To create session, use
 * :func:`sessionNew()` function.
 */
struct Session;

/**
 * @function
 *
 * Initializes the global data. It also initializes underlying
 * libraries libaria2 depends on. This function returns 0 if it
 * succeeds, or negative error code.
 *
 * Call this function only once before calling any other API
 * functions.
 */
int libraryInit();

/**
 * @function
 *
 * Releases the global data. This function returns 0 if it succeeds,
 * or negative error code.
 *
 * Call this function only once at the end of the application.
 */
int libraryDeinit();

/**
 * @typedef
 *
 * The type of GID, persistent identifier of each download.
 */
typedef uint64_t A2Gid;

/**
 * @typedef
 *
 * The type of Key/Value pairs.
 */
typedef std::vector<std::pair<std::string, std::string>> KeyVals;

/**
 * @enum
 *
 * Download event constants
 */
enum DownloadEvent {
  /**
   * Indicating download has started.
   */
  EVENT_ON_DOWNLOAD_START = 1,
  /**
   * Indicating download has paused.
   */
  EVENT_ON_DOWNLOAD_PAUSE,
  /**
   * Indicating download has stopped.
   */
  EVENT_ON_DOWNLOAD_STOP,
  /**
   * Indicating download has completed.
   */
  EVENT_ON_DOWNLOAD_COMPLETE,
  /**
   * Indicating download has stopped because of the error.
   */
  EVENT_ON_DOWNLOAD_ERROR,
  /**
   * Indicating BitTorrent download has completed, but it may still
   * continue to perform seeding.
   */
  EVENT_ON_BT_DOWNLOAD_COMPLETE
};

/**
 * @functypedef
 *
 * Callback function invoked when download event occurred. The |event|
 * indicates the event. See :type:`DownloadEvent` for events. The
 * |gid| refers to the download which this event was fired on. The
 * |userData| is a pointer specified in
 * :member:`SessionConfig::userData`.
 *
 * At the moment, the return value is ignored, but the implementation
 * of this callback should return 0 for compatibility.
 */
typedef int (*DownloadEventCallback)(Session* session, DownloadEvent event,
                                     A2Gid gid, void* userData);

/**
 * @struct
 *
 * The configuration for the session.
 */
struct SessionConfig {
  /**
   * The constructor fills default values for all members.
   */
  SessionConfig();
  /**
   * If the |keepRunning| member is true, ``run(session, RUN_ONCE)``
   * will return 1 even if there are no download to perform. The
   * behavior is very similar to RPC server, except that this option
   * does not enable RPC functionality. To stop aria2, use
   * :func:`shutdown()` function.  The default value is false.
   */
  bool keepRunning;
  /**
   * If the |useSignalHandler| is true, the library setups following
   * signal handlers in :func:`sessionNew()`. These signal handlers
   * are removed in :func:`sessionFinal()`. The default value is
   * true. If the application sets this member to false, it must
   * handle these signals and ensure that run() is repeatedly called
   * until it returns 0 and :func:`sessionFinal()` is called after
   * that. Failing these steps will lead to not saving .aria2 control
   * file and no session serialization.
   *
   * ``SIGPIPE``, ``SIGCHLD``:
   *   ignored
   * ``SIGHUP``, ``SIGTERM``:
   *   handled like shutdown(session, true) is called.
   * ``SIGINT``:
   *   handled like shutdown(session, false) is called.
   */
  bool useSignalHandler;
  /**
   * Specify the callback function which will be invoked when download
   * event occurred. See :type:`DownloadEvent` about the download
   * event. The default value is ``NULL``.
   */
  DownloadEventCallback downloadEventCallback;
  /**
   * Pointer to user defined data. libaria2 treats this as opaque
   * pointer and will not free it. The default value is ``NULL``.
   */
  void* userData;
};

/**
 * @function
 *
 * Creates new Session object using the |options| as additional
 * parameters. The |options| is treated as if they are specified in
 * command-line to :manpage:`aria2c(1)`. This function returns the
 * pointer to the created Session object if it succeeds, or ``NULL``.
 *
 * Please note that only one Session object can be created per
 * process.
 */
Session* sessionNew(const KeyVals& options, const SessionConfig& config);

/**
 * @function
 *
 * Performs post-download action, including saving sessions etc and
 * destroys the |session| object, releasing the allocated resources
 * for it. This function returns the last error code and it is the
 * equivalent to the :ref:`exit-status` of :manpage:`aria2c(1)`.
 */
int sessionFinal(Session* session);

/**
 * @enum
 *
 * Execution mode for :func:`run()`
 */
enum RUN_MODE {
  /**
   * :func:`run()` returns when no downloads are left.
   */
  RUN_DEFAULT,
  /**
   * :func:`run()` returns after one event polling.
   */
  RUN_ONCE
};

/**
 * @function
 *
 * Performs event polling and actions for them. If the |mode| is
 * :c:macro:`RUN_DEFAULT`, this function returns when no downloads are
 * left to be processed. In this case, this function returns 0.
 *
 * If the |mode| is :c:macro:`RUN_ONCE`, this function returns after
 * one event polling. In the current implementation, event polling
 * timeouts in 1 second. This function also returns on each
 * timeout. On return, when no downloads are left to be processed,
 * this function returns 0. Otherwise, returns 1, indicating that the
 * caller must call this function one or more time to complete
 * downloads.
 *
 * In either case, this function returns negative error code on error.
 */
int run(Session* session, RUN_MODE mode);

/**
 * @function
 *
 * Returns textual representation of the |gid|.
 */
std::string gidToHex(A2Gid gid);

/**
 * @function
 *
 * Returns GID converted from the textual representation |hex|.
 */
A2Gid hexToGid(const std::string& hex);

/**
 * @function
 *
 * Returns true if the |gid| is invalid.
 */
bool isNull(A2Gid gid);

/**
 * @function
 *
 * Adds new HTTP(S)/FTP/BitTorrent Magnet URI.  On successful return,
 * if the |gid| is not ``NULL``, the GID of added download will be
 * assigned to the |*gid|.  The |uris| includes URI to be downloaded.
 * For BitTorrent Magnet URI, the |uris| must have only one element
 * and it should be BitTorrent Magnet URI. URIs in the |uris| must
 * point to the same file. If you mix other URIs which point to
 * another file, aria2 does not complain but download may fail. The
 * |options| is an array of a pair of option name and value. If
 * unknown options are included in |options|, they are simply
 * ignored. If the |position| is not negative integer, the new
 * download is inserted at position in the waiting queue. If the
 * |position| is negative or the |position| is larger than the size of
 * the queue, it is appended at the end of the queue.  This function
 * returns 0 if it succeeds, or negative error code.
 */
int addUri(Session* session, A2Gid* gid, const std::vector<std::string>& uris,
           const KeyVals& options, int position = -1);

/**
 * @function
 *
 * Adds Metalink download. The path to Metalink file is specified by
 * the |metalinkFile|.  On successful return, if the |gids| is not
 * ``NULL``, the GIDs of added downloads are appended to the
 * |*gids|. The |options| is an array of a pair of option name and
 * value. If unknown options are included in |options|, they are
 * simply ignored. If the |position| is not negative integer, the new
 * download is inserted at position in the waiting queue. If the
 * |position| is negative or the |position| is larger than the size of
 * the queue, it is appended at the end of the queue. This function
 * returns 0 if it succeeds, or negative error code.
 */
int addMetalink(Session* session, std::vector<A2Gid>* gids,
                const std::string& metalinkFile, const KeyVals& options,
                int position = -1);

/**
 * @function
 *
 * Adds BitTorrent download. On successful return, if the |gid| is not
 * ``NULL``, the GID of added download will be assigned to the
 * |*gid|. The path to ".torrent" file is specified by the
 * |torrentFile|. BitTorrent Magnet URI cannot be used with this
 * function. Use :func:`addUri()` instead. The |webSeedUris| contains
 * URIs used for Web-seeding. For single file torrents, URI can be a
 * complete URI pointing to the resource or if URI ends with /, name
 * in torrent file is added. For multi-file torrents, name and path in
 * torrent are added to form a URI for each file. The |options| is an
 * array of a pair of option name and value.  If unknown options are
 * included in |options|, they are simply ignored. If the |position|
 * is not negative integer, the new download is inserted at position
 * in the waiting queue. If the |position| is negative or the
 * |position| is larger than the size of the queue, it is appended at
 * the end of the queue.
 *
 * This function returns 0 if it succeeds, or negative error code.
 *
 */
int addTorrent(Session* session, A2Gid* gid, const std::string& torrentFile,
               const std::vector<std::string>& webSeedUris,
               const KeyVals& options, int position = -1);

/**
 * @function
 *
 * Same as :func:`addTorrent()` with an empty vector as the
 * |webSeedUris|.
 */
int addTorrent(Session* session, A2Gid* gid, const std::string& torrentFile,
               const KeyVals& options, int position = -1);

/**
 * @function
 *
 * Returns the array of active download GID.
 */
std::vector<A2Gid> getActiveDownload(Session* session);

/**
 * @function
 *
 * Removes the download denoted by the |gid|. If the specified
 * download is in progress, it is stopped at first. The status of
 * removed download becomes :c:macro:`DOWNLOAD_REMOVED`. If the
 * |force| is true, removal will take place without any action which
 * takes time such as contacting BitTorrent tracker. This function
 * returns 0 if it succeeds, or negative error code.
 */
int removeDownload(Session* session, A2Gid gid, bool force = false);

/**
 * @function
 *
 * Pauses the download denoted by the |gid|. The status of paused
 * download becomes :c:macro:`DOWNLOAD_PAUSED`. If the download is
 * active, the download is placed on the first position of waiting
 * queue. As long as the status is :c:macro:`DOWNLOAD_PAUSED`, the
 * download will not start. To change status to
 * :c:macro:`DOWNLOAD_WAITING`, use :func:`unpauseDownload()`
 * function.  If the |force| is true, pause will take place without
 * any action which takes time such as contacting BitTorrent
 * tracker. This function returns 0 if it succeeds, or negative error
 * code.
 *
 * Please note that, to make pause work, the application must set
 * :member:`SessionConfig::keepRunning` to true. Otherwise, the
 * behavior is undefined.
 */
int pauseDownload(Session* session, A2Gid gid, bool force = false);

/**
 * @function
 *
 * Changes the status of the download denoted by the |gid| from
 * :c:macro:`DOWNLOAD_PAUSED` to :c:macro:`DOWNLOAD_WAITING`. This
 * makes the download eligible to restart. This function returns 0 if
 * it succeeds, or negative error code.
 */
int unpauseDownload(Session* session, A2Gid gid);

/**
 * @function
 *
 * Apply options in the |options| to the download denoted by the |gid|
 * dynamically. The following options can be changed for downloads in
 * :c:macro:`DOWNLOAD_ACTIVE` status:
 *
 * * :option:`bt-max-peers <--bt-max-peers>`
 * * :option:`bt-request-peer-speed-limit <--bt-request-peer-speed-limit>`
 * * :option:`bt-remove-unselected-file <--bt-remove-unselected-file>`
 * * :option:`force-save <--force-save>`
 * * :option:`max-download-limit <--max-download-limit>`
 * * :option:`max-upload-limit <-u>`
 *
 * For downloads in :c:macro:`DOWNLOAD_WAITING` or
 * :c:macro:`DOWNLOAD_PAUSED` status, in addition to the above
 * options, options listed in :ref:`input-file` subsection are available,
 * except for following options:
 * :option:`dry-run <--dry-run>`,
 * :option:`metalink-base-uri <--metalink-base-uri>`,
 * :option:`parameterized-uri <-P>`,
 * :option:`pause <--pause>`,
 * :option:`piece-length <--piece-length>` and
 * :option:`rpc-save-upload-metadata <--rpc-save-upload-metadata>` option.
 *
 * The options which are not applicable or unknown, they are just
 * ignored.
 *
 * This function returns 0 if it succeeds, or negative error code.
 */
int changeOption(Session* session, A2Gid gid, const KeyVals& options);

/**
 * @function
 *
 * Returns global option denoted by the |name|. If such option is not
 * available, returns empty string.
 */
const std::string& getGlobalOption(Session* session, const std::string& name);

/**
 * @function
 *
 * Returns global options. Note that this function does not return
 * options which have no default value and have not been set by
 * :func:`sessionNew()`, configuration files or API functions.
 */
KeyVals getGlobalOptions(Session* session);

/**
 * @function
 *
 * Apply global options in the |options| dynamically.  The following
 * options are available:
 *
 * * :option:`download-result <--download-result>`
 * * :option:`log <-l>`
 * * :option:`log-level <--log-level>`
 * * :option:`max-concurrent-downloads <-j>`
 * * :option:`max-download-result <--max-download-result>`
 * * :option:`max-overall-download-limit <--max-overall-download-limit>`
 * * :option:`max-overall-upload-limit <--max-overall-upload-limit>`
 * * :option:`save-cookies <--save-cookies>`
 * * :option:`save-session <--save-session>`
 * * :option:`server-stat-of <--server-stat-of>`
 *
 * In addition to them, options listed in :ref:`input-file` subsection
 * are available, except for following options:
 * :option:`checksum <--checksum>`,
 * :option:`index-out <-O>`,
 * :option:`out <-o>`,
 * :option:`pause <--pause>` and
 * :option:`select-file <--select-file>`.
 *
 * The options which are not applicable or unknown, they are just
 * ignored.
 *
 * This function returns 0 if it succeeds, or negative error code.
 */
int changeGlobalOption(Session* session, const KeyVals& options);

/**
 * @struct
 *
 * Global statistics of current aria2 session.
 */
struct GlobalStat {
  /**
   * Overall download speed (byte/sec).
   */
  int downloadSpeed;
  /**
   * Overall upload speed(byte/sec).
   */
  int uploadSpeed;
  /**
   * The number of active downloads.
   */
  int numActive;
  /**
   * The number of waiting downloads.
   */
  int numWaiting;
  /**
   * The number of stopped downloads.
   */
  int numStopped;
};

/**
 * @function
 *
 * Returns global statistics such as overall download and upload
 * speed.
 */
GlobalStat getGlobalStat(Session* session);

/**
 * @enum
 *
 * Constants how to re-position a download.
 */
enum OffsetMode {
  /**
   * Moves the download to a position relative to the beginning of the
   * queue.
   */
  OFFSET_MODE_SET,
  /**
   * Moves the download to a position relative to the current
   * position.
   */
  OFFSET_MODE_CUR,
  /**
   * Moves the download to a position relative to the end of the
   * queue.
   */
  OFFSET_MODE_END
};

/**
 * @function
 *
 * Changes the position of the download denoted by the |gid|. if it is
 * in :c:macro:`DOWNLOAD_WAITING` or :c:macro:`DOWNLOAD_PAUSED` state.
 * If the |how| is :c:macro:`OFFSET_MODE_SET`, it moves the download
 * to a position |pos| relative to the beginning of the queue. If the
 * |how| is :c:macro:`OFFSET_MODE_CUR`, it moves the download to a
 * position |pos| relative to the current position. If the |how| is
 * :c:macro:`OFFSET_MODE_END`, it moves the download to a position
 * |pos| relative to the end of the queue. If the destination position
 * is less than 0 or beyond the end of the queue, it moves the
 * download to the beginning or the end of the queue respectively. The
 * response is the destination position on success.
 *
 * For example, if the download having GID gid is placed in position
 * 3, ``changePosition(gid, -1, OFFSET_MODE_CUR)`` will change its
 * position to 2. Additional call ``changePosition(gid, 0,
 * OFFSET_MODE_SET)`` will change its position to 0 (the beginning of
 * the queue).
 *
 * This function returns the final destination position of this
 * download, or negative error code.
 */
int changePosition(Session* session, A2Gid gid, int pos, OffsetMode how);

/**
 * @function
 *
 * Schedules shutdown. If the |force| is true, shutdown will take
 * place without any action which takes time such as contacting
 * BitTorrent tracker. After this call, the application must keep
 * calling :func:`run()` function until it returns 0.  This function
 * returns 0 if it succeeds, or negative error code.
 */
int shutdown(Session* session, bool force = false);

/**
 * @enum
 *
 * The status of URI.
 */
enum UriStatus {
  /**
   * Indicating the URI has been used.
   */
  URI_USED,
  /**
   * Indicating the URI has not been used.
   */
  URI_WAITING
};

/**
 * @struct
 *
 * This object contains URI and its status.
 */
struct UriData {
  /**
   * URI
   */
  std::string uri;
  /**
   * The status of URI
   */
  UriStatus status;
};

/**
 * @struct
 *
 * This object contains information of file to download.
 */
struct FileData {
  /**
   * 1-based index of the file in the download. This is the same order
   * with the files in multi-file torrent. This index is used to get
   * this object using :func:`DownloadHandle::getFile()` function.
   */
  int index;
  /**
   * The local file path to this file when downloaded.
   */
  std::string path;
  /**
   * The file size in bytes. This is not the current size of the local
   * file.
   */
  int64_t length;
  /**
   * The completed length of this file in bytes. Please note that it
   * is possible that sum of |completedLength| is less than the return
   * value of :func:`DownloadHandle::getCompletedLength()`
   * function. This is because the |completedLength| only calculates
   * completed pieces. On the other hand,
   * :func:`DownloadHandle::getCompletedLength()` takes into account
   * of partially completed piece.
   */
  int64_t completedLength;
  /**
   * true if this file is selected by ``select-file`` option. If
   * ``select-file`` is not specified or this is single torrent or no
   * torrent download, this value is always true.
   */
  bool selected;
  /**
   * Returns the list of URI for this file.
   */
  std::vector<UriData> uris;
};

/**
 * @enum
 *
 * BitTorrent file mode
 */
enum BtFileMode {
  /**
   * Indicating no mode. This value is used when file mode is not
   * available.
   */
  BT_FILE_MODE_NONE,
  /**
   * Indicating single file torrent
   */
  BT_FILE_MODE_SINGLE,
  /**
   * Indicating multi file torrent
   */
  BT_FILE_MODE_MULTI
};

/**
 * @struct
 *
 * BitTorrent metainfo data retrieved from ".torrent" file.
 */
struct BtMetaInfoData {
  /**
   * List of lists of announce URI. If ".torrent" file contains
   * ``announce`` and no ``announce-list``, ``announce`` is converted
   * to ``announce-list`` format.
   */
  std::vector<std::vector<std::string>> announceList;
  /**
   * ``comment`` for the torrent. ``comment.utf-8`` is used if
   * available.
   */
  std::string comment;
  /**
   * The creation time of the torrent. The value is an integer since
   * the Epoch, measured in seconds.
   */
  time_t creationDate;
  /**
   * File mode of the torrent.
   */
  BtFileMode mode;
  /**
   * ``name`` in ``info`` dictionary. ``name.utf-8`` is used if
   * available.
   */
  std::string name;
};

/**
 * @enum
 *
 * The status of download item.
 */
enum DownloadStatus {
  /**
   * Indicating currently downloading/seeding.
   */
  DOWNLOAD_ACTIVE,
  /**
   * Indicating in the queue; download is not started.
   */
  DOWNLOAD_WAITING,
  /**
   * Indicating the download is paused.
   */
  DOWNLOAD_PAUSED,
  /**
   * Indicating stopped and completed download.
   */
  DOWNLOAD_COMPLETE,
  /**
   * Indicating stopped download because of error.
   */
  DOWNLOAD_ERROR,
  /**
   * Indicating removed by user's discretion.
   */
  DOWNLOAD_REMOVED
};

/**
 * @class
 *
 * The interface to get information of download item.
 */
class DownloadHandle {
public:
  virtual ~DownloadHandle() = default;
  /**
   * Returns status of this download.
   */
  virtual DownloadStatus getStatus() = 0;
  /**
   * Returns the total length of this download in bytes.
   */
  virtual int64_t getTotalLength() = 0;
  /**
   * Returns the completed length of this download in bytes.
   */
  virtual int64_t getCompletedLength() = 0;
  /**
   * Returns the uploaded length of this download in bytes.
   */
  virtual int64_t getUploadLength() = 0;
  /**
   * Returns the download progress in byte-string. The highest bit
   * corresponds to piece index 0. The set bits indicate the piece is
   * available and unset bits indicate the piece is missing. The spare
   * bits at the end are set to zero. When download has not started
   * yet, returns empty string.
   */
  virtual std::string getBitfield() = 0;
  /**
   * Returns download speed of this download measured in bytes/sec.
   */
  virtual int getDownloadSpeed() = 0;
  /**
   * Returns upload speed of this download measured in bytes/sec.
   */
  virtual int getUploadSpeed() = 0;
  /**
   * Returns 20 bytes InfoHash if BitTorrent transfer is
   * involved. Otherwise the empty string is returned.
   */
  virtual const std::string& getInfoHash() = 0;
  /**
   * Returns piece length in bytes.
   */
  virtual size_t getPieceLength() = 0;
  /**
   * Returns the number of pieces.
   */
  virtual int getNumPieces() = 0;
  /**
   * Returns the number of peers/servers the client has connected to.
   */
  virtual int getConnections() = 0;
  /**
   * Returns the last error code occurred in this download. The error
   * codes are defined in :ref:`exit-status` section of
   * :manpage:`aria2c(1)`. This value has its meaning only for
   * stopped/completed downloads.
   */
  virtual int getErrorCode() = 0;
  /**
   * Returns array of GIDs which are generated by the consequence of
   * this download. For example, when aria2 downloaded Metalink file,
   * it generates downloads described in it (see
   * :option:`--follow-metalink` option). This value is useful to
   * track these auto generated downloads. If there is no such
   * downloads, this function returns empty array.
   */
  virtual const std::vector<A2Gid>& getFollowedBy() = 0;
  /**
   * Returns the GID of the download which generated this download.
   * This is a reverse link of
   * :func:`DownloadHandle::getFollowedBy()`.
   */
  virtual A2Gid getFollowing() = 0;
  /**
   * Returns the GID of a parent download. Some downloads are a part
   * of another download. For example, if a file in Metalink has
   * BitTorrent resource, the download of ".torrent" is a part of that
   * file. If this download has no parent, the invalid GID is returned
   * (``isNull(gid)`` is true).
   */
  virtual A2Gid getBelongsTo() = 0;
  /**
   * Returns the directory to save files.
   */
  virtual const std::string& getDir() = 0;
  /**
   * Returns the array of files this download contains.
   */
  virtual std::vector<FileData> getFiles() = 0;
  /**
   * Returns the number of files. The return value is equivalent to
   * ``DownloadHandle::getFiles().size()``.
   */
  virtual int getNumFiles() = 0;
  /**
   * Returns the FileData of the file at the specified |index|. Please
   * note that the index is 1-based. It is undefined when the |index|
   * is out-of-bound.
   */
  virtual FileData getFile(int index) = 0;
  /**
   * Returns the information retrieved from ".torrent" file. This
   * function is only meaningful only when BitTorrent transfer is
   * involved in the download and the download is not
   * stopped/completed.
   */
  virtual BtMetaInfoData getBtMetaInfo() = 0;
  /**
   * Returns the option value denoted by the |name|.  If the option
   * denoted by the |name| is not available, returns empty string.
   *
   * Calling this function for the download which is not in
   * :c:macro:`DOWNLOAD_ACTIVE`, :c:macro:`DOWNLOAD_PAUSED` or
   * :c:macro:`DOWNLOAD_WAITING` will return empty string.
   */
  virtual const std::string& getOption(const std::string& name) = 0;
  /**
   * Returns options for this download. Note that this function does
   * not return options which have no default value and have not been
   * set by :func:`sessionNew()`, configuration files or API
   * functions.
   */
  virtual KeyVals getOptions() = 0;
};

/**
 * @function
 *
 * Returns handle for the download denoted by the |gid|. The caller
 * can retrieve various information of the download via returned
 * handle's member functions. The lifetime of the returned handle is
 * before the next call of :func:`run()` or
 * :func:`sessionFinal()`. The caller must call
 * :func:`deleteDownloadHandle()` before that. This function returns
 * ``NULL`` if no download denoted by the |gid| is present. It is the
 * responsibility of the caller to call :func:`deleteDownloadHandle()`
 * to delete handle object.
 */
DownloadHandle* getDownloadHandle(Session* session, A2Gid gid);

/**
 * @function
 *
 * Deallocates the |dh|. Calling this function with ``NULL`` is safe.
 */
void deleteDownloadHandle(DownloadHandle* dh);

} // namespace aria2

#endif // ARIA2_H
