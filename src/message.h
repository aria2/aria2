/* <!-- copyright */
/*
 * aria2 - a simple utility for downloading files faster
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
/* copyright --> */
#ifndef _D_MESSAGE_H_
#define _D_MESSAGE_H_

#include "common.h"

#define MSG_DOWNLOAD_COMPLETED "CUID#%d - The download for one segment completed successfully."
#define MSG_NO_SEGMENT_AVAILABLE "CUID#%d - No segment available."
#define MSG_CONNECTING_TO_SERVER "CUID#%d - Connecting to %s:%d"
#define MSG_SEGMENT_CHANGED "CUID#%d - The segment changed. We send the request again with new Range header."
#define MSG_REDIRECT "CUID#%d - Redirecting to %s"
#define MSG_SENDING_REQUEST "CUID#%d - Requesting:\n%s"
#define MSG_RECEIVE_RESPONSE "CUID#%d - Response received:\n%s"
#define MSG_DOWNLOAD_ABORTED "CUID#%d - Download aborted."
#define MSG_RESTARTING_DOWNLOAD "CUID#%d - Restarting the download."
#define MSG_MAX_TRY "CUID#%d - %d times attempted, but no success. Download aborted."
#define MSG_UNREGISTER_CUID "CUID#%d - Unregistering cuid from segmentManager."

#define MSG_SEGMENT_FILE_EXISTS "The segment file %s exists."
#define MSG_SEGMENT_FILE_DOES_NOT_EXIST "The segment file %s does not exist."
#define MSG_SAVING_SEGMENT_FILE "Saving the segment file %s"
#define MSG_SAVED_SEGMENT_FILE "The segment file was saved successfully."
#define MSG_LOADING_SEGMENT_FILE "Loading the segment file %s."
#define MSG_LOADED_SEGMENT_FILE "The segment file was loaded successfully."

#define EX_TIME_OUT "Timeout."
#define EX_INVALID_CHUNK_SIZE "Invalid chunk size."
#define EX_TOO_LARGE_CHUNK "Too large chunk. size = %d"
#define EX_INVALID_HEADER "Invalid header."
#define EX_INVALID_RESPONSE "Invalid response."
#define EX_NO_HEADER "No header found."
#define EX_NO_STATUS_HEADER "No status header."
#define EX_PROXY_CONNECTION_FAILED "Proxy connection failed."
#define EX_CONNECTION_FAILED "Connection failed."
#define EX_FILENAME_MISMATCH "The requested filename and the previously registered one are not same. %s != %s"
#define EX_BAD_STATUS "The response status is not successful. status = %d"
#define EX_TOO_LARGE_FILE "Too large file size. size = %lld"
#define EX_TRANSFER_ENCODING_NOT_SUPPORTED "Transfer encoding %s is not supported."
#define EX_SSL_INIT_FAILURE "SSL initialization failed."
#define EX_SIZE_MISMATCH "Size mismatch %lld != %lld"

#endif // _D_MESSAGE_H_
