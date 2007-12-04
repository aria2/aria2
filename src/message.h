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
#ifndef _D_MESSAGE_H_
#define _D_MESSAGE_H_

#include "common.h"

#define MSG_SEGMENT_DOWNLOAD_COMPLETED _("CUID#%d - The download for one segment completed successfully.")
#define MSG_NO_SEGMENT_AVAILABLE _("CUID#%d - No segment available.")
#define MSG_CONNECTING_TO_SERVER _("CUID#%d - Connecting to %s:%d")
#define MSG_SEGMENT_CHANGED _("CUID#%d - The segment changed. We send the request again with new Range header.")
#define MSG_REDIRECT _("CUID#%d - Redirecting to %s")
#define MSG_SENDING_REQUEST _("CUID#%d - Requesting:\n%s")
#define MSG_RECEIVE_RESPONSE _("CUID#%d - Response received:\n%s")
#define MSG_DOWNLOAD_ABORTED _("CUID#%d - Download aborted. URI=%s")
#define MSG_RESTARTING_DOWNLOAD _("CUID#%d - Restarting the download. URI=%s")
#define MSG_TORRENT_DOWNLOAD_ABORTED _("CUID#%d - Download aborted.")
#define MSG_MAX_TRY _("CUID#%d - %d times attempted, but no success. Download aborted.")
#define MSG_UNREGISTER_CUID _("CUID#%d - Unregistering cuid from segmentManager.")
#define MSG_SEND_PEER_MESSAGE "CUID#%d - To: %s:%d %s"
#define MSG_SEND_PEER_MESSAGE_WITH_INDEX "CUID#%d - To: %s:%d %s index=%d"
#define MSG_SEND_PEER_MESSAGE_WITH_BITFIELD "CUID#%d - To: %s:%d %s %s"
#define MSG_SEND_PEER_MESSAGE_WITH_INDEX_BEGIN_LENGTH "CUID#%d - To: %s:%d %s index=%d, begin=%d, length=%d"
#define MSG_RECEIVE_PEER_MESSAGE "CUID#%d - From: %s:%d %s"
#define MSG_GOT_NEW_PIECE _("CUID#%d - we got new piece. index=%d")
#define MSG_GOT_WRONG_PIECE _("CUID#%d - we got wrong piece. index=%d")
#define MSG_DOWNLOAD_NOT_COMPLETE _("CUID#%d - Download not complete: %s")
#define MSG_DOWNLOAD_ALREADY_COMPLETED _("#%d - Download has already completed: %s")
#define MSG_GOOD_CHECKSUM _("CUID#%d - Good checksum: %s")
#define MSG_BAD_CHECKSUM _("CUID#%d - Bad checksum: %s")
#define MSG_RESOLVING_HOSTNAME _("CUID#%d - Resolving hostname %s")
#define MSG_NAME_RESOLUTION_COMPLETE _("CUID#%d - Name resolution complete: %s -> %s")
#define MSG_NAME_RESOLUTION_FAILED _("CUID#%d - Name resolution for %s failed:%s")
#define MSG_DNS_CACHE_HIT _("CUID#%d - DNS cache hit: %s -> %s")
#define MSG_ABORT_REQUESTED _("CUID#%d - Abort requested.")
#define MSG_CONNECTING_TO_PEER _("CUID#%d - Connecting to the peer %s")
#define MSG_PIECE_RECEIVED _("CUID#%d - Piece received. index=%d, begin=%d, length=%d, offset=%llu, blockIndex=%d")
#define MSG_PIECE_BITFIELD _("CUID#%d - Piece bitfield %s")
#define MSG_REJECT_PIECE_CHOKED _("CUID#%d - Reject piece message in queue because the peer has been choked. index=%d, begin=%d, length=%d")
#define MSG_REJECT_PIECE_CANCEL _("CUID#%d - Reject piece message in queue because cancel message received. index=%d, begin=%d, length=%d")
#define MSG_FILE_VALIDATION_FAILURE _("CUID#%d - Exception caught while validating file integrity.")
#define MSG_PEER_INTERESTED _("CUID#%d - Interested in the peer")
#define MSG_PEER_NOT_INTERESTED _("CUID#%d - Not interested in the peer")
#define MSG_DELETING_REQUEST_SLOT _("CUID#%d - Deleting request slot index=%d, blockIndex=%d")
#define MSG_DELETING_REQUEST_SLOT_CHOKED _("CUID#%d - Deleting request slot index=%d, blockIndex=%d because localhost got choked.")
#define MSG_DELETING_REQUEST_SLOT_TIMEOUT _("CUID#%d - Deleting request slot blockIndex=%d because of time out")
#define MSG_DELETING_REQUEST_SLOT_ACQUIRED _("CUID#%d - Deleting request slot blockIndex=%d because the block has been acquired.")
#define MSG_FAST_EXTENSION_ENABLED _("CUID#%d - Fast extension enabled.")
#define MSG_FILE_ALLOCATION_FAILURE _("CUID#%d - Exception caught while allocating file space.")
#define MSG_CONTENT_DISPOSITION_DETECTED _("CUID#%d - Content-Disposition detected. Use %s as filename")
#define MSG_PEER_BANNED _("CUID#%d - Peer %s:%d banned.")
#define MSG_LISTENING_PORT _("CUID#%d - Using port %d for accepting new connections")
#define MSG_BIND_FAILURE _("CUID#%d - An error occurred while binding port=%d")
#define MSG_INCOMING_PEER_CONNECTION _("CUID#%d - Incoming connection, adding new command CUID#%d")
#define MSG_ACCEPT_FAILURE _("CUID#%d - Error in accepting connection")
#define MSG_TRACKER_RESPONSE_PROCESSING_FAILED _("CUID#%d - Error occurred while processing tracker response.")
#define MSG_TRACKER_REQUEST_CREATION_FAILED _("CUID#%d - Cannot create tracker request.")
#define MSG_CREATING_TRACKER_REQUEST _("CUID#%d - Creating new tracker request command #%d")

#define MSG_UNRECOGNIZED_URI _("Unrecognized URI or unsupported protocol: %s")
#define MSG_TRACKER_WARNING_MESSAGE _("Tracker returned warning message: %s")
#define MSG_SEGMENT_FILE_EXISTS _("The segment file %s exists.")
#define MSG_SEGMENT_FILE_DOES_NOT_EXIST _("The segment file %s does not exist.")
#define MSG_SAVING_SEGMENT_FILE _("Saving the segment file %s")
#define MSG_SAVED_SEGMENT_FILE _("The segment file was saved successfully.")
#define MSG_LOADING_SEGMENT_FILE _("Loading the segment file %s.")
#define MSG_LOADED_SEGMENT_FILE _("The segment file was loaded successfully.")
#define MSG_NO_URL_TO_DOWNLOAD _("No URI to download. Download aborted.")
#define MSG_FILE_ALREADY_EXISTS _("File %s exists, but a control file(*.aria2) does not exist. Download was canceled in order to prevent your file from being truncated to 0. If you are sure to download the file all over again, then delete it or add --allow-overwrite=true option and restart aria2.")
#define MSG_ALLOCATING_FILE _("Allocating file %s, %s bytes")
#define MSG_FILE_NOT_FOUND _("File not found")
#define MSG_NOT_DIRECTORY _("Not a directory")
#define MSG_INSUFFICIENT_CHECKSUM _("Insufficient checksums. checksumLength=%d, numChecksum=%d")
#define MSG_WRITING_FILE _("Writing file %s")
#define MSG_NO_PEER_LIST_RECEIVED _("No peer list received.")
#define MSG_ADDING_PEER _("Adding peer %s:%d")
#define MSG_DELETING_USED_PIECE _("Deleting used piece index=%d, fillRate(%%)=%d<=%d")
#define MSG_SELECTIVE_DOWNLOAD_COMPLETED _("Download of selected files was complete.")
#define MSG_DOWNLOAD_COMPLETED _("The download was complete.")
#define MSG_REMOVED_HAVE_ENTRY _("Removed %d have entries.")
#define MSG_VALIDATING_FILE _("Validating file %s")
#define MSG_ALLOCATION_COMPLETED _("%d seconds to allocate %s byte(s)")
#define MSG_FILE_ALLOCATION_DISPATCH _("Dispatching FileAllocationCommand for CUID#%d.")
#define MSG_METALINK_QUEUEING _("Metalink: Queueing %s for download.")
#define MSG_FILE_DOWNLOAD_COMPLETED _("Download complete: %s")
#define MSG_SEEDING_END _("Seeding is over.")
#define MSG_SEGMENT_FORWARDING _("CUID#%d cancels segment index=%d. CUID#%d handles it instead.")
#define MSG_NO_CHUNK_CHECKSUM _("No chunk to verify.")
#define MSG_GOOD_CHUNK_CHECKSUM _("Good chunk checksum. hash=%s")
#define MSG_LOADING_COOKIE_FAILED _("Failed to load cookies from %s")
#define MSG_INCORRECT_NETRC_PERMISSION _(".netrc file %s does not have correct permissions. It should be 600. netrc support disabled.")
#define MSG_LOGGING_STARTED _("Logging started.")
#define MSG_URI_REQUIRED _("Specify at least one URL.")
#define MSG_DAEMON_FAILED _("daemon failed.")
#define MSG_VERIFICATION_SUCCESSFUL _("Verification finished successfully. file=%s")
#define MSG_VERIFICATION_FAILED _("Checksum error detected. file=%s")
#define MSG_INCOMPLETE_RANGE _("Incomplete range specified. %s")
#define MSG_STRING_INTEGER_CONVERSION_FAILURE _("Failed to convert string into value: %s")
#define MSG_RESOURCE_NOT_FOUND _("Resource not found")
#define MSG_FILE_RENAMED _("File already exists. Renamed to %s.")
#define MSG_CANNOT_PARSE_METALINK _("Cannot parse metalink XML file. XML may be malformed.")

#define EX_TIME_OUT _("Timeout.")
#define EX_INVALID_CHUNK_SIZE _("Invalid chunk size.")
#define EX_TOO_LARGE_CHUNK _("Too large chunk. size=%d")
#define EX_INVALID_HEADER _("Invalid header.")
#define EX_INVALID_RESPONSE _("Invalid response.")
#define EX_NO_HEADER _("No header found.")
#define EX_NO_STATUS_HEADER _("No status header.")
#define EX_PROXY_CONNECTION_FAILED _("Proxy connection failed.")
#define EX_CONNECTION_FAILED _("Connection failed.")
#define EX_FILENAME_MISMATCH _("The requested filename and the previously registered one are not same. Expected:%s Actual:%s")
#define EX_BAD_STATUS _("The response status is not successful. status=%d")
#define EX_TOO_LARGE_FILE _("Too large file size. size=%s")
#define EX_TRANSFER_ENCODING_NOT_SUPPORTED _("Transfer encoding %s is not supported.")
#define EX_SSL_INIT_FAILURE _("SSL initialization failed: %s")
#define EX_SSL_IO_ERROR _("SSL I/O error")
#define EX_SSL_PROTOCOL_ERROR _("SSL protocol error")
#define EX_SSL_UNKNOWN_ERROR _("SSL unknown error %d")
#define EX_SSL_CONNECT_ERROR _("SSL initialization failed: OpenSSL connect error %d")
#define EX_SIZE_MISMATCH _("Size mismatch Expected:%s Actual:%s")
#define EX_AUTH_FAILED _("Authorization failed.")
#define EX_GOT_EOF _("Got EOF from the server.")
#define EX_EOF_FROM_PEER _("Got EOF from peer.")
#define EX_MULFORMED_META_INFO _("Malformed meta info.")

#define EX_FILE_OPEN _("Failed to open the file %s, cause: %s")
#define EX_FILE_WRITE _("Failed to write into the file %s, cause: %s")
#define EX_FILE_READ _("Failed to read from the file %s, cause: %s")
#define EX_DATA_READ _("Failed to read data from disk.")
#define EX_FILE_SHA1SUM _("Failed to calculate SHA1 digest of or a part of the file %s, cause: %s")
#define EX_FILE_SEEK _("Failed to seek the file %s, cause: %s")
#define EX_FILE_OFFSET_OUT_OF_RANGE _("The offset is out of range, offset=%s")
#define EX_NOT_DIRECTORY _("%s is not a directory.")
#define EX_MAKE_DIR _("Failed to make the directory %s, cause: %s")
#define EX_SEGMENT_FILE_OPEN _("Failed to open the segment file %s, cause: %s")
#define EX_SEGMENT_FILE_WRITE _("Failed to write into the segment file %s, cause: %s")
#define EX_SEGMENT_FILE_READ _("Failed to read from the segment file %s, cause: %s")

#define EX_SOCKET_OPEN _("Failed to open a socket, cause: %s")
#define EX_SOCKET_SET_OPT _("Failed to set a socket option, cause: %s")
#define EX_SOCKET_BLOCKING _("Failed to set a socket as blocking, cause: %s")
#define EX_SOCKET_NONBLOCKING _("Failed to set a socket as non-blocking, cause: %s")
#define EX_SOCKET_BIND _("Failed to bind a socket, cause: %s")
#define EX_SOCKET_LISTEN _("Failed to listen to a socket, cause: %s")
#define EX_SOCKET_ACCEPT _("Failed to accept a peer connection, cause: %s")
#define EX_SOCKET_GET_NAME _("Failed to get the name of socket, cause: %s")
#define EX_SOCKET_GET_PEER _("Failed to get the name of connected peer, cause: %s")
#define EX_RESOLVE_HOSTNAME _("Failed to resolve the hostname %s, cause: %s")
#define EX_SOCKET_CONNECT _("Failed to connect to the host %s, cause: %s")
#define EX_SOCKET_CHECK_WRITABLE _("Failed to check whether the socket is writable, cause: %s")
#define EX_SOCKET_CHECK_READABLE _("Failed to check whether the socket is readable, cause: %s")
#define EX_SOCKET_SEND _("Failed to send data, cause: %s")
#define EX_SOCKET_RECV _("Failed to receive data, cause: %s")
#define EX_SOCKET_PEEK _("Failed to peek data, cause: %s")
#define EX_SOCKET_UNKNOWN_ERROR _("Unknown socket error %d (0x%x)")
#define EX_FILE_ALREADY_EXISTS _("File %s exists, but %s does not exist.")
#define EX_INVALID_PAYLOAD_SIZE _("Invalid payload size for %s, size=%d. It should be %d.")
#define EX_INVALID_BT_MESSAGE_ID _("Invalid ID=%d for %s. It should be %d.")
#define EX_INVALID_CHUNK_CHECKSUM _("Chunk checksum validation failed. checksumIndex=%d, offset=%s, expectedHash=%s, actualHash=%s")
#define EX_DOWNLOAD_ABORTED _("Download aborted.")
#define EX_DUPLICATE_FILE_DOWNLOAD _("File %s is being downloaded by other command.")
#define EX_INSUFFICIENT_CHECKSUM _("Insufficient checksums.")
#define EX_TRACKER_FAILURE _("Tracker returned failure reason: %s")
#define EX_FLOODING_DETECTED _("Flooding detected.")
#define EX_DROP_INACTIVE_CONNECTION _("Drop connection because no request/piece messages were exchanged in a certain period(%d seconds).")
#define EX_INFOHASH_MISMATCH_IN_SEGFILE _("The infoHash in torrent file doesn't match to one in .aria2 file.")
#define EX_NO_SUCH_FILE_ENTRY _("No such file entry %s")
#define EX_TOO_SLOW_DOWNLOAD_SPEED _("Too slow Downloading speed: %d <= %d(B/s), host:%s")
#define EX_NO_HTTP_REQUEST_ENTRY_FOUND _("No HttpRequestEntry found.")
#define EX_LOCATION_HEADER_REQUIRED _("Got %d status, but no location header provided.")
#define EX_INVALID_RANGE_HEADER _("Invalid range header. Request: %s-%s/%s, Response: %s-%s/%s")
#define EX_NO_RESULT_WITH_YOUR_PREFS _("No file matched with your preference.")
#define EX_EXCEPTION_CAUGHT _("Exception caught")
#define EX_TOO_LONG_PAYLOAD _("Max payload length exceeded or invalid. length = %d")
#define EX_FILE_LENGTH_MISMATCH_BETWEEN_LOCAL_AND_REMOTE _("Invalid file length. Cannot continue download %s: local %s, remote %s")
#endif // _D_MESSAGE_H_
