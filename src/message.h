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

#define MSG_DOWNLOAD_COMPLETED _("CUID#%d - The download for one segment completed successfully.")
#define MSG_NO_SEGMENT_AVAILABLE _("CUID#%d - No segment available.")
#define MSG_CONNECTING_TO_SERVER _("CUID#%d - Connecting to %s:%d")
#define MSG_SEGMENT_CHANGED _("CUID#%d - The segment changed. We send the request again with new Range header.")
#define MSG_REDIRECT _("CUID#%d - Redirecting to %s")
#define MSG_SENDING_REQUEST _("CUID#%d - Requesting:\n%s")
#define MSG_RECEIVE_RESPONSE _("CUID#%d - Response received:\n%s")
#define MSG_DOWNLOAD_ABORTED _("CUID#%d - Download aborted.")
#define MSG_RESTARTING_DOWNLOAD _("CUID#%d - Restarting the download.")
#define MSG_MAX_TRY _("CUID#%d - %d times attempted, but no success. Download aborted.")
#define MSG_UNREGISTER_CUID _("CUID#%d - Unregistering cuid from segmentManager.")

#define MSG_SEGMENT_FILE_EXISTS _("The segment file %s exists.")
#define MSG_SEGMENT_FILE_DOES_NOT_EXIST _("The segment file %s does not exist.")
#define MSG_SAVING_SEGMENT_FILE _("Saving the segment file %s")
#define MSG_SAVED_SEGMENT_FILE _("The segment file was saved successfully.")
#define MSG_LOADING_SEGMENT_FILE _("Loading the segment file %s.")
#define MSG_LOADED_SEGMENT_FILE _("The segment file was loaded successfully.")

#define EX_TIME_OUT _("Timeout.")
#define EX_INVALID_CHUNK_SIZE _("Invalid chunk size.")
#define EX_TOO_LARGE_CHUNK _("Too large chunk. size = %d")
#define EX_INVALID_HEADER _("Invalid header.")
#define EX_INVALID_RESPONSE _("Invalid response.")
#define EX_NO_HEADER _("No header found.")
#define EX_NO_STATUS_HEADER _("No status header.")
#define EX_PROXY_CONNECTION_FAILED _("Proxy connection failed.")
#define EX_CONNECTION_FAILED _("Connection failed.")
#define EX_FILENAME_MISMATCH _("The requested filename and the previously registered one are not same. %s != %s")
#define EX_BAD_STATUS _("The response status is not successful. status = %d")
#define EX_TOO_LARGE_FILE _("Too large file size. size = %lld")
#define EX_TRANSFER_ENCODING_NOT_SUPPORTED _("Transfer encoding %s is not supported.")
#define EX_SSL_INIT_FAILURE _("SSL initialization failed.")
#define EX_SIZE_MISMATCH _("Size mismatch %lld != %lld")
#define EX_GOT_EOF _("Got EOF from the server.")

#endif // _D_MESSAGE_H_
