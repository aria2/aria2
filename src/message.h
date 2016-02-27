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
#ifndef D_MESSAGE_H
#define D_MESSAGE_H

#include "common.h"

// clang-format off

#define MSG_SEGMENT_DOWNLOAD_COMPLETED                                  \
  "CUID#%" PRId64 " - The download for one segment completed successfully."
#define MSG_NO_SEGMENT_AVAILABLE "CUID#%" PRId64 " - No segment available."
#define MSG_CONNECTING_TO_SERVER "CUID#%" PRId64 " - Connecting to %s:%d"
#define MSG_REDIRECT "CUID#%" PRId64 " - Redirecting to %s"
#define MSG_SENDING_REQUEST "CUID#%" PRId64 " - Requesting:\n%s"
#define MSG_RECEIVE_RESPONSE "CUID#%" PRId64 " - Response received:\n%s"
#define MSG_DOWNLOAD_ABORTED "CUID#%" PRId64 " - Download aborted. URI=%s"
#define MSG_RESTARTING_DOWNLOAD "CUID#%" PRId64 " - Restarting the download. URI=%s"
#define MSG_TORRENT_DOWNLOAD_ABORTED "CUID#%" PRId64 " - Download aborted."
#define MSG_MAX_TRY                                                     \
  "CUID#%" PRId64 " - %d times attempted, but no success. Download aborted."
#define MSG_SEND_PEER_MESSAGE "CUID#%" PRId64 " - To: %s:%d %s"
#define MSG_RECEIVE_PEER_MESSAGE "CUID#%" PRId64 " - From: %s:%d %s"
#define MSG_GOT_NEW_PIECE "CUID#%" PRId64 " - we got new piece. index=%lu"
#define MSG_GOT_WRONG_PIECE "CUID#%" PRId64 " - we got wrong piece. index=%lu"
#define MSG_DOWNLOAD_NOT_COMPLETE "CUID#%" PRId64 " - Download not complete: %s"
#define MSG_DOWNLOAD_ALREADY_COMPLETED _("GID#%s - Download has already completed: %s")
#define MSG_RESOLVING_HOSTNAME "CUID#%" PRId64 " - Resolving hostname %s"
#define MSG_NAME_RESOLUTION_COMPLETE                    \
  "CUID#%" PRId64 " - Name resolution complete: %s -> %s"
#define MSG_NAME_RESOLUTION_FAILED                      \
  "CUID#%" PRId64 " - Name resolution for %s failed:%s"
#define MSG_DNS_CACHE_HIT "CUID#%" PRId64 " - DNS cache hit: %s -> %s"
#define MSG_CONNECTING_TO_PEER "CUID#%" PRId64 " - Connecting to the peer %s"
#define MSG_PIECE_RECEIVED                                              \
  "CUID#%" PRId64 " - Piece received. index=%lu, begin=%d, length=%d, offset=%" PRId64 "," \
  " blockIndex=%lu"
#define MSG_PIECE_BITFIELD "CUID#%" PRId64 " - Piece bitfield %s"
#define MSG_REJECT_PIECE_CHOKED                                         \
  "CUID#%" PRId64 " - Reject piece message in queue because the peer has been" \
  " choked. index=%lu, begin=%d, length=%d"
#define MSG_REJECT_PIECE_CANCEL                                         \
  "CUID#%" PRId64 " - Reject piece message in queue because cancel message received." \
  " index=%lu, begin=%d, length=%d"
#define MSG_FILE_VALIDATION_FAILURE                             \
  "CUID#%" PRId64 " - Exception caught while validating file integrity."
#define MSG_PEER_INTERESTED "CUID#%" PRId64 " - Interested in the peer"
#define MSG_PEER_NOT_INTERESTED "CUID#%" PRId64 " - Not interested in the peer"
#define MSG_DELETING_REQUEST_SLOT "CUID#%" PRId64 " - Deleting request slot" \
  " index=%lu, begin=%d, blockIndex=%lu"
#define MSG_DELETING_REQUEST_SLOT_CHOKED "CUID#%" PRId64 " - Deleting request slot" \
  " index=%lu, begin=%d, blockIndex=%lu because localhost got choked."
#define MSG_DELETING_REQUEST_SLOT_TIMEOUT "CUID#%" PRId64 " - Deleting request slot" \
  " index=%lu, begin=%d, blockIndex=%lu because of time out"
#define MSG_DELETING_REQUEST_SLOT_ACQUIRED "CUID#%" PRId64 " - Deleting request slot" \
  " index=%lu, begin=%d, blockIndex=%lu because the block has been acquired."
#define MSG_FAST_EXTENSION_ENABLED "CUID#%" PRId64 " - Fast extension enabled."
#define MSG_EXTENDED_MESSAGING_ENABLED "CUID#%" PRId64 " - Extended Messaging enabled."
#define MSG_FILE_ALLOCATION_FAILURE                             \
  "CUID#%" PRId64 " - Exception caught while allocating file space."
#define MSG_CONTENT_DISPOSITION_DETECTED                        \
  "CUID#%" PRId64 " - Content-Disposition detected. Use %s as filename"
#define MSG_PEER_BANNED "CUID#%" PRId64 " - Peer %s:%d banned."
#define MSG_LISTENING_PORT                                      \
  "CUID#%" PRId64 " - Using port %d for accepting new connections"
#define MSG_BIND_FAILURE "CUID#%" PRId64 " - An error occurred while binding port=%d"
#define MSG_INCOMING_PEER_CONNECTION                            \
  "CUID#%" PRId64 " - Incoming connection, adding new command CUID#%" PRId64 ""
#define MSG_ACCEPT_FAILURE "CUID#%" PRId64 " - Error in accepting connection"
#define MSG_TRACKER_RESPONSE_PROCESSING_FAILED                  \
  "CUID#%" PRId64 " - Error occurred while processing tracker response."
#define MSG_DHT_ENABLED_PEER "CUID#%" PRId64 " - The peer is DHT-enabled."
#define MSG_CONNECT_FAILED_AND_RETRY            \
  "CUID#%" PRId64 " - Could not to connect to %s:%u. Trying another address"

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
#define MSG_REMOVED_HAVE_ENTRY _("Removed %lu have entries.")
#define MSG_VALIDATING_FILE _("Validating file %s")
#define MSG_ALLOCATION_COMPLETED "%ld seconds to allocate %" PRId64 " byte(s)"
#define MSG_FILE_ALLOCATION_DISPATCH                    \
  "Dispatching FileAllocationCommand for CUID#%" PRId64 "."
#define MSG_METALINK_QUEUEING _("Metalink: Queueing %s for download.")
#define MSG_FILE_DOWNLOAD_COMPLETED _("Download complete: %s")
#define MSG_SEEDING_END _("Seeding is over.")
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
#define MSG_TOO_SMALL_PAYLOAD_SIZE _("Too small payload size for %s, size=%lu.")
#define MSG_REMOVED_DEFUNCT_CONTROL_FILE _("Removed the defunct control file %s because the download file %s doesn't exist.")
#define MSG_SHARE_RATIO_REPORT _("Your share ratio was %.1f, uploaded/downloaded=%sB/%sB")
#define MSG_MISSING_BT_INFO _("Missing %s in torrent metainfo.")
#define MSG_NEGATIVE_LENGTH_BT_INFO _("%s does not allow negative integer %" PRId64 "")
#define MSG_NULL_TRACKER_RESPONSE _("Tracker returned null data.")
#define MSG_WINSOCK_INIT_FAILD _("Windows socket library initialization failed")
#define MSG_TIME_HAS_PASSED _("%ld second(s) has passed. Stopping application.")
#define MSG_SIGNATURE_SAVED _("Saved signature as %s. Please note that aria2" \
                              " doesn't verify signatures.")
#define MSG_SIGNATURE_NOT_SAVED _("Saving signature as %s failed. Maybe file" \
                                  " already exists.")
#define MSG_OPENING_READABLE_SERVER_STAT_FILE_FAILED    \
  _("Failed to open ServerStat file %s for read.")
#define MSG_SERVER_STAT_LOADED _("ServerStat file %s loaded successfully.")
#define MSG_READING_SERVER_STAT_FILE_FAILED _("Failed to read ServerStat from" \
                                              " %s.")
#define MSG_OPENING_WRITABLE_SERVER_STAT_FILE_FAILED    \
  _("Failed to open ServerStat file %s for write.")
#define MSG_SERVER_STAT_SAVED _("ServerStat file %s saved successfully.")
#define MSG_WRITING_SERVER_STAT_FILE_FAILED _("Failed to write ServerStat to" \
                                              " %s.")
#define MSG_ESTABLISHING_CONNECTION_FAILED              \
  _("Failed to establish connection, cause: %s")
#define MSG_NETWORK_PROBLEM _("Network problem has occurred. cause:%s")
#define MSG_LOADING_SYSTEM_TRUSTED_CA_CERTS_FAILED                              \
  _("Failed to load trusted CA certificates from system. Cause: %s")
#define MSG_LOADING_TRUSTED_CA_CERT_FAILED                              \
  _("Failed to load trusted CA certificates from %s. Cause: %s")
#define MSG_CERT_VERIFICATION_FAILED                    \
  _("Certificate verification failed. Cause: %s See --ca-certificate and" \
    " --check-certificate option.")
#define MSG_NO_CERT_FOUND _("No certificate found.")
#define MSG_HOSTNAME_NOT_MATCH _("Hostname not match.")
#define MSG_NO_FILES_TO_DOWNLOAD _("No files to download.")
#define MSG_WARN_NO_CA_CERT                                             \
  _("You may encounter the certificate verification error with HTTPS server." \
    " See --ca-certificate and --check-certificate option.")
#define MSG_WARN_UNKNOWN_TLS_CONNECTION \
  _("aria2c had to connect to the other side using an unknown TLS protocol. " \
    "The integrity and confidentiality of the connection might be " \
    "compromised.\nPeer: %s")
#define MSG_WARN_OLD_TLS_CONNECTION \
  _("aria2c had to connect to the other side using an old and vulnerable TLS" \
    " protocol. The integrity and confidentiality of the connection might be" \
    " compromised.\nProtocol: %s, Peer: %s")
#define MSG_SHOW_FILES _("Printing the contents of file '%s'...")
#define MSG_NOT_TORRENT_METALINK _("This file is neither Torrent nor Metalink" \
                                   " file. Skipping.")
#define MSG_GID_NOT_PROVIDED "GID is not provided."
#define MSG_CANNOT_PARSE_XML_RPC_REQUEST "Failed to parse xml-rpc request."
#define MSG_GOOD_BYE_SEEDER "Client is in seed state: Good Bye Seeder;)"
#define MSG_NOT_FILE _("Is '%s' a file?")
#define MSG_INTERFACE_NOT_FOUND _("Failed to find given interface %s,"  \
                                  " cause: %s")
#define MSG_METADATA_SAVED _("Saved metadata as %s.")
#define MSG_METADATA_NOT_SAVED _("Saving metadata as %s failed. Maybe file" \
                                 " already exists.")
#define MSG_DIR_TRAVERSAL_DETECTED _("Detected directory traversal directive in %s")
#define MSG_HASH_CHECK_NOT_DONE                                         \
  "File has already been downloaded but hash check has not been done yet."
#define MSG_REMOVING_UNSELECTED_FILE _("GID#%s - Removing unselected file.")
#define MSG_FILE_REMOVED _("File %s removed.")
#define MSG_FILE_COULD_NOT_REMOVED _("File %s could not be removed.")

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
#define EX_TOO_LARGE_FILE "Too large file size. size=%" PRId64 ""
#define EX_TRANSFER_ENCODING_NOT_SUPPORTED _("Transfer encoding %s is not supported.")
#define EX_SSL_INIT_FAILURE _("SSL initialization failed: %s")
#define EX_SSL_IO_ERROR _("SSL I/O error")
#define EX_SSL_PROTOCOL_ERROR _("SSL protocol error")
#define EX_SSL_UNKNOWN_ERROR _("SSL unknown error %d")
#define EX_SSL_CONNECT_ERROR _("SSL initialization failed: OpenSSL connect error %d")
#define EX_SIZE_MISMATCH "Size mismatch Expected:%" PRId64 " Actual:%" PRId64 ""
#define EX_AUTH_FAILED _("Authorization failed.")
#define EX_GOT_EOF _("Got EOF from the server.")
#define EX_EOF_FROM_PEER _("Got EOF from peer.")
#define EX_MALFORMED_META_INFO _("Malformed meta info.")

#define EX_FILE_OPEN _("Failed to open the file %s, cause: %s")
#define EX_FILE_WRITE _("Failed to write into the file %s, cause: %s")
#define EX_FILE_READ _("Failed to read from the file %s, cause: %s")
#define EX_DATA_READ _("Failed to read data from disk.")
#define EX_FILE_SHA1SUM _("Failed to calculate SHA1 digest of or a part of the file %s, cause: %s")
#define EX_FILE_SEEK _("Failed to seek the file %s, cause: %s")
#define EX_FILE_OFFSET_OUT_OF_RANGE "The offset is out of range, offset=%" PRId64 ""
#define EX_NOT_DIRECTORY _("%s is not a directory.")
#define EX_MAKE_DIR _("Failed to make the directory %s, cause: %s")
#define EX_SEGMENT_FILE_WRITE "Failed to write into the segment file %s"
#define EX_SEGMENT_FILE_READ "Failed to read from the segment file %s"

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
#define EX_INVALID_PAYLOAD_SIZE                                 \
  _("Invalid payload size for %s, size=%lu. It should be %lu.")
#define EX_INVALID_BT_MESSAGE_ID _("Invalid ID=%d for %s. It should be %d.")
#define EX_INVALID_CHUNK_CHECKSUM "Chunk checksum validation failed. checksumIndex=%lu, offset=%" PRId64 ", expectedHash=%s, actualHash=%s"
#define EX_DOWNLOAD_ABORTED _("Download aborted.")
#define EX_DUPLICATE_FILE_DOWNLOAD _("File %s is being downloaded by other command.")
#define EX_INSUFFICIENT_CHECKSUM _("Insufficient checksums.")
#define EX_TRACKER_FAILURE _("Tracker returned failure reason: %s")
#define EX_FLOODING_DETECTED _("Flooding detected.")
#define EX_DROP_INACTIVE_CONNECTION \
  _("Drop connection because no request/piece messages were exchanged in a" \
    " certain period(%ld seconds).")
#define EX_INFOHASH_MISMATCH_IN_SEGFILE _("The infoHash in torrent file doesn't match to one in .aria2 file.")
#define EX_NO_SUCH_FILE_ENTRY _("No such file entry %s")
#define EX_TOO_SLOW_DOWNLOAD_SPEED _("Too slow Downloading speed: %d <= %d(B/s), host:%s")
#define EX_NO_HTTP_REQUEST_ENTRY_FOUND _("No HttpRequestEntry found.")
#define EX_LOCATION_HEADER_REQUIRED _("Got %d status, but no location header provided.")
#define EX_INVALID_RANGE_HEADER "Invalid range header. Request: %" PRId64 "-%" PRId64 "/%" PRId64 ", Response: %" PRId64 "-%" PRId64 "/%" PRId64 ""
#define EX_NO_RESULT_WITH_YOUR_PREFS _("No file matched with your preference.")
#define EX_EXCEPTION_CAUGHT _("Exception caught")
#define EX_TOO_LONG_PAYLOAD _("Max payload length exceeded or invalid. length = %u")
#define EX_FILE_LENGTH_MISMATCH_BETWEEN_LOCAL_AND_REMOTE _("Invalid file length. Cannot continue download %s: local %s, remote %s")

// clang-format on

#endif // D_MESSAGE_H
