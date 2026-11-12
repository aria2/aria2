/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "UTMetadataPostDownloadHandler.h"
#include "bittorrent_helper.h"
#include "RequestGroup.h"
#include "download_helper.h"
#include "RecoverableException.h"
#include "A2STR.h"
#include "DownloadContext.h"
#include "Logger.h"
#include "LogFactory.h"
#include "util.h"
#include "a2functional.h"
#include "DiskAdaptor.h"
#include "PieceStorage.h"
#include "bencode2.h"
#include "message.h"
#include "prefs.h"
#include "Option.h"
#include "fmt.h"
#include "RequestGroupMan.h"
#include "TorrentAttribute.h"
#include "FileEntry.h"
#include "error_code.h"
#include "ValueBaseBencodeParser.h"
#include "MetadataInfo.h"
#include "util.h"

namespace aria2 {

namespace {

// Check if a file extension is considered executable
bool isExecutableExtension(const std::string& path) {
  std::string lowerPath = path;
  std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(),
                 ::tolower);
  
  static const std::vector<std::string> executableExtensions = {
    ".exe", ".sh", ".bat", ".cmd", ".com", ".scr", ".pif", ".vbs", ".js",
    ".ps1", ".msi", ".dll", ".bin", ".app", ".command", ".csh", ".ksh",
    ".zsh", ".bash", ".pl", ".py", ".rb", ".cgi"
  };
  
  for (const auto& ext : executableExtensions) {
    if (lowerPath.size() >= ext.size() &&
        lowerPath.compare(lowerPath.size() - ext.size(), ext.size(), ext) == 0) {
      return true;
    }
  }
  return false;
}

// Extract filename from magnet link URI
std::string extractFileNameFromUri(const std::string& uri) {
  // Look for dn= parameter in magnet link
  std::string dn = "dn=";
  size_t pos = uri.find(dn);
  if (pos != std::string::npos) {
    pos += dn.size();
    size_t end = uri.find('&', pos);
    if (end == std::string::npos) {
      end = uri.size();
    }
    std::string fileName = uri.substr(pos, end - pos);
    // URL decode using percentDecode
    return util::percentDecode(fileName.begin(), fileName.end());
  }
  return "";
}

} // namespace

namespace {
class Criteria : public RequestGroupCriteria {
public:
  virtual bool match(const RequestGroup* requestGroup) const CXX11_OVERRIDE
  {
    auto& dctx = requestGroup->getDownloadContext();
    if (dctx->hasAttribute(CTX_ATTR_BT)) {
      if (bittorrent::getTorrentAttrs(dctx)->metadata.empty()) {
        return true;
      }
    }
    return false;
  }
};
} // namespace

UTMetadataPostDownloadHandler::UTMetadataPostDownloadHandler()
{
  setCriteria(make_unique<Criteria>());
}

void UTMetadataPostDownloadHandler::getNextRequestGroups(
    std::vector<std::shared_ptr<RequestGroup>>& groups,
    RequestGroup* requestGroup) const
{
  auto& dctx = requestGroup->getDownloadContext();
  auto attrs = bittorrent::getTorrentAttrs(dctx);
  std::string metadata =
      util::toString(requestGroup->getPieceStorage()->getDiskAdaptor());

  // Check metadata size limit to prevent memory exhaustion attacks
  int64_t metadataMaxSize =
      requestGroup->getOption()->getAsInt(PREF_BT_METADATA_MAX_SIZE);
  if (metadataMaxSize > 0 &&
      static_cast<int64_t>(metadata.size()) > metadataMaxSize) {
    throw DL_ABORT_EX2(
        fmt(MSG_DHT_METADATA_SIZE_LIMIT, metadata.size(), metadataMaxSize),
        error_code::BITTORRENT_PARSE_ERROR);
  }

  std::string torrent = bittorrent::metadata2Torrent(metadata, attrs);

  if (requestGroup->getOption()->getAsBool(PREF_BT_SAVE_METADATA)) {
    std::string filename =
        util::applyDir(requestGroup->getOption()->get(PREF_DIR),
                       util::toHex(attrs->infoHash) + ".torrent");
    if (util::saveAs(filename, torrent)) {
      A2_LOG_NOTICE(fmt(MSG_METADATA_SAVED, filename.c_str()));
    }
    else {
      A2_LOG_NOTICE(fmt(MSG_METADATA_NOT_SAVED, filename.c_str()));
    }
  }
  if (!requestGroup->getOption()->getAsBool(PREF_BT_METADATA_ONLY)) {
    // If web seeds from DHT metadata are disabled, remove them from the
    // torrent data before creating the download. This is the default for
    // security reasons because DHT metadata is untrusted.
    std::string processedTorrent = torrent;
    bool dhtWebSeedEnabled =
        requestGroup->getOption()->getAsBool(PREF_BT_ENABLE_DHT_WEB_SEED);
    bool webSeedEnabled =
        requestGroup->getOption()->getAsBool(PREF_BT_ENABLE_WEB_SEED);
    
    // Parse the torrent to apply security checks
    bittorrent::ValueBaseBencodeParser parser;
    ssize_t error;
    auto root = parser.parseFinal(processedTorrent.c_str(),
                                  processedTorrent.size(), error);
    
    if (root && error >= 0) {
      Dict* rootDict = downcast<Dict>(root);
      if (rootDict) {
        // Check 1: Disable web seeds if --bt-enable-web-seed=false
        if (!webSeedEnabled) {
          if (rootDict->containsKey("url-list")) {
            A2_LOG_NOTICE(MSG_WEB_SEED_DISABLED);
            rootDict->removeKey("url-list");
          }
        }
        // Check 2: Strip DHT web seeds if --bt-enable-dht-web-seed=false
        else if (!dhtWebSeedEnabled) {
          if (rootDict->containsKey("url-list")) {
            A2_LOG_NOTICE(MSG_DHT_WEB_SEED_IGNORED);
            rootDict->removeKey("url-list");
          }
        }
        
        // Get expected filename from magnet link if available
        std::string expectedFileName;
        if (requestGroup->getMetadataInfo()) {
          expectedFileName = extractFileNameFromUri(
              requestGroup->getMetadataInfo()->getUri());
        }
        
        // Check 3: Warn about executable files
        if (requestGroup->getOption()->getAsBool(PREF_BT_WARN_EXECUTABLE_FILES)) {
          if (rootDict->containsKey("info")) {
            Dict* infoDict = downcast<Dict>(rootDict->get("info"));
            if (infoDict) {
              // Check single file torrents
              if (infoDict->containsKey("name") && infoDict->containsKey("length")) {
                ValueBase* nameVal = infoDict->get("name");
                if (nameVal) {
                  String* nameStr = downcast<String>(nameVal);
                  if (nameStr) {
                    std::string fileName = nameStr->s();
                    if (isExecutableExtension(fileName)) {
                      A2_LOG_NOTICE(fmt(MSG_WARN_EXECUTABLE_FILE, fileName.c_str()));
                    }
                  }
                }
              }
              // Check multi-file torrents
              else if (infoDict->containsKey("files")) {
                List* filesList = downcast<List>(infoDict->get("files"));
                if (filesList) {
                  std::string dirName;
                  if (infoDict->containsKey("name")) {
                    ValueBase* nameVal = infoDict->get("name");
                    if (nameVal) {
                      String* nameStr = downcast<String>(nameVal);
                      if (nameStr) {
                        dirName = nameStr->s();
                      }
                    }
                  }
                  for (auto it = filesList->begin(); it != filesList->end(); ++it) {
                    Dict* fileDict = downcast<Dict>((*it).get());
                    if (fileDict && fileDict->containsKey("path")) {
                      List* pathList = downcast<List>(fileDict->get("path"));
                      if (pathList && !pathList->empty()) {
                        // Get the last element of the path (filename)
                        auto lastIt = std::prev(pathList->end());
                        String* fileNameStr = downcast<String>(lastIt->get());
                        if (fileNameStr) {
                          std::string fileName = fileNameStr->s();
                          if (isExecutableExtension(fileName)) {
                            std::string fullPath = dirName.empty() ? fileName :
                                                  dirName + "/" + fileName;
                            A2_LOG_NOTICE(fmt(MSG_WARN_EXECUTABLE_FILE, 
                                             fullPath.c_str()));
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
        
        // Check 4: Warn about file count mismatch
        if (requestGroup->getOption()->getAsBool(PREF_BT_WARN_FILE_COUNT)) {
          if (!expectedFileName.empty()) {
            Dict* infoDict = downcast<Dict>(rootDict->get("info"));
            if (infoDict && infoDict->containsKey("files")) {
              List* filesList = downcast<List>(infoDict->get("files"));
              if (filesList && filesList->size() > 1) {
                A2_LOG_NOTICE(fmt(MSG_WARN_FILE_COUNT, filesList->size()));
              }
            }
          }
        }
        
        // Check 5: Require single file if --bt-require-single-file=true
        if (requestGroup->getOption()->getAsBool(PREF_BT_REQUIRE_SINGLE_FILE)) {
          if (!expectedFileName.empty()) {
            Dict* infoDict = downcast<Dict>(rootDict->get("info"));
            if (infoDict && infoDict->containsKey("files")) {
              throw DL_ABORT_EX2(MSG_ABORT_MULTI_FILE,
                                error_code::BITTORRENT_PARSE_ERROR);
            }
          }
        }
        
        // Re-encode the modified torrent
        processedTorrent = bencode2::encode(rootDict);
      }
    }

    std::vector<std::shared_ptr<RequestGroup>> newRgs;
    // Don't adjust announce URI because it has been done when
    // RequestGroup is created with magnet URI.
    createRequestGroupForBitTorrent(newRgs, requestGroup->getOption(),
                                    std::vector<std::string>(), A2STR::NIL,
                                    processedTorrent, false);
    requestGroup->followedBy(newRgs.begin(), newRgs.end());
    for (auto& rg : newRgs) {
      rg->following(requestGroup->getGID());
    }
    if (requestGroup->getMetadataInfo()) {
      setMetadataInfo(newRgs.begin(), newRgs.end(),
                      requestGroup->getMetadataInfo());
    }
    auto rgman = requestGroup->getRequestGroupMan();

    if (rgman && rgman->getKeepRunning() &&
        requestGroup->getOption()->getAsBool(PREF_PAUSE_METADATA)) {
      for (auto& rg : newRgs) {
        rg->setPauseRequested(true);
      }
    }

    groups.insert(groups.end(), newRgs.begin(), newRgs.end());
  }
}

} // namespace aria2
