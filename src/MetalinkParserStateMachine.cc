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
#include "MetalinkParserStateMachine.h"

#include <sstream>
#include <iterator>

#include "MetalinkParserStateImpl.h"
#include "MetalinkParserStateV3Impl.h"
#include "MetalinkParserStateV4Impl.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"
#include "a2functional.h"

namespace aria2 {

MetalinkParserState* MetalinkParserStateMachine::initialState_ =
    new InitialMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::skipTagState_ =
    new SkipTagMetalinkParserState();

MetalinkParserState* MetalinkParserStateMachine::metalinkState_ =
    new MetalinkMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::filesState_ =
    new FilesMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::fileState_ =
    new FileMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::sizeState_ =
    new SizeMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::versionState_ =
    new VersionMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::languageState_ =
    new LanguageMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::osState_ =
    new OSMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::verificationState_ =
    new VerificationMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::hashState_ =
    new HashMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::piecesState_ =
    new PiecesMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::pieceHashState_ =
    new PieceHashMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::signatureState_ =
    new SignatureMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::resourcesState_ =
    new ResourcesMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::urlState_ =
    new URLMetalinkParserState();

MetalinkParserState* MetalinkParserStateMachine::metalinkStateV4_ =
    new MetalinkMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::fileStateV4_ =
    new FileMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::sizeStateV4_ =
    new SizeMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::versionStateV4_ =
    new VersionMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::languageStateV4_ =
    new LanguageMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::osStateV4_ =
    new OSMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::hashStateV4_ =
    new HashMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::piecesStateV4_ =
    new PiecesMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::pieceHashStateV4_ =
    new PieceHashMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::signatureStateV4_ =
    new SignatureMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::urlStateV4_ =
    new URLMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::metaurlStateV4_ =
    new MetaurlMetalinkParserStateV4();

MetalinkParserStateMachine::MetalinkParserStateMachine()
    : ctrl_{make_unique<MetalinkParserController>()}
{
  stateStack_.push(initialState_);
}

MetalinkParserStateMachine::~MetalinkParserStateMachine() = default;

void MetalinkParserStateMachine::reset()
{
  ctrl_->reset();
  errors_.clear();
  while (!stateStack_.empty()) {
    stateStack_.pop();
  }
  stateStack_.push(initialState_);
}

void MetalinkParserStateMachine::setMetalinkState()
{
  stateStack_.push(metalinkState_);
}

void MetalinkParserStateMachine::setFilesState()
{
  stateStack_.push(filesState_);
}

void MetalinkParserStateMachine::setFileState()
{
  stateStack_.push(fileState_);
}

void MetalinkParserStateMachine::setSizeState()
{
  stateStack_.push(sizeState_);
}

void MetalinkParserStateMachine::setVersionState()
{
  stateStack_.push(versionState_);
}

void MetalinkParserStateMachine::setLanguageState()
{
  stateStack_.push(languageState_);
}

void MetalinkParserStateMachine::setOSState() { stateStack_.push(osState_); }

void MetalinkParserStateMachine::setVerificationState()
{
  stateStack_.push(verificationState_);
}

void MetalinkParserStateMachine::setHashState()
{
  stateStack_.push(hashState_);
}

void MetalinkParserStateMachine::setPiecesState()
{
  stateStack_.push(piecesState_);
}

void MetalinkParserStateMachine::setPieceHashState()
{
  stateStack_.push(pieceHashState_);
}

void MetalinkParserStateMachine::setSignatureState()
{
  stateStack_.push(signatureState_);
}

void MetalinkParserStateMachine::setResourcesState()
{
  stateStack_.push(resourcesState_);
}

void MetalinkParserStateMachine::setURLState() { stateStack_.push(urlState_); }

void MetalinkParserStateMachine::setMetalinkStateV4()
{
  stateStack_.push(metalinkStateV4_);
}

void MetalinkParserStateMachine::setFileStateV4()
{
  stateStack_.push(fileStateV4_);
}

void MetalinkParserStateMachine::setSizeStateV4()
{
  stateStack_.push(sizeStateV4_);
}

void MetalinkParserStateMachine::setVersionStateV4()
{
  stateStack_.push(versionStateV4_);
}

void MetalinkParserStateMachine::setLanguageStateV4()
{
  stateStack_.push(languageStateV4_);
}

void MetalinkParserStateMachine::setOSStateV4()
{
  stateStack_.push(osStateV4_);
}

void MetalinkParserStateMachine::setHashStateV4()
{
  stateStack_.push(hashStateV4_);
}

void MetalinkParserStateMachine::setPiecesStateV4()
{
  stateStack_.push(piecesStateV4_);
}

void MetalinkParserStateMachine::setPieceHashStateV4()
{
  stateStack_.push(pieceHashStateV4_);
}

void MetalinkParserStateMachine::setSignatureStateV4()
{
  stateStack_.push(signatureStateV4_);
}

void MetalinkParserStateMachine::setURLStateV4()
{
  stateStack_.push(urlStateV4_);
}

void MetalinkParserStateMachine::setMetaurlStateV4()
{
  stateStack_.push(metaurlStateV4_);
}

void MetalinkParserStateMachine::setSkipTagState()
{
  stateStack_.push(skipTagState_);
}

bool MetalinkParserStateMachine::finished() const
{
  return stateStack_.top() == initialState_;
}

void MetalinkParserStateMachine::newEntryTransaction()
{
  ctrl_->newEntryTransaction();
}

void MetalinkParserStateMachine::setFileNameOfEntry(std::string filename)
{
  ctrl_->setFileNameOfEntry(std::move(filename));
}

void MetalinkParserStateMachine::setFileLengthOfEntry(int64_t length)
{
  ctrl_->setFileLengthOfEntry(length);
}

void MetalinkParserStateMachine::setVersionOfEntry(std::string version)
{
  ctrl_->setVersionOfEntry(std::move(version));
}

void MetalinkParserStateMachine::setLanguageOfEntry(std::string language)
{
  ctrl_->setLanguageOfEntry(std::move(language));
}

void MetalinkParserStateMachine::setOSOfEntry(std::string os)
{
  ctrl_->setOSOfEntry(std::move(os));
}

void MetalinkParserStateMachine::setMaxConnectionsOfEntry(int maxConnections)
{
  ctrl_->setMaxConnectionsOfEntry(maxConnections);
}

void MetalinkParserStateMachine::commitEntryTransaction()
{
  ctrl_->commitEntryTransaction();
}

void MetalinkParserStateMachine::cancelEntryTransaction()
{
  ctrl_->cancelEntryTransaction();
}

void MetalinkParserStateMachine::newResourceTransaction()
{
  ctrl_->newResourceTransaction();
}

void MetalinkParserStateMachine::setURLOfResource(std::string url)
{
  ctrl_->setURLOfResource(std::move(url));
}

void MetalinkParserStateMachine::setTypeOfResource(std::string type)
{
  ctrl_->setTypeOfResource(std::move(type));
}

void MetalinkParserStateMachine::setLocationOfResource(std::string location)
{
  ctrl_->setLocationOfResource(std::move(location));
}

void MetalinkParserStateMachine::setPriorityOfResource(int priority)
{
  ctrl_->setPriorityOfResource(priority);
}

void MetalinkParserStateMachine::setMaxConnectionsOfResource(int maxConnections)
{
  ctrl_->setMaxConnectionsOfResource(maxConnections);
}

void MetalinkParserStateMachine::commitResourceTransaction()
{
  ctrl_->commitResourceTransaction();
}

void MetalinkParserStateMachine::cancelResourceTransaction()
{
  ctrl_->cancelResourceTransaction();
}

void MetalinkParserStateMachine::newChecksumTransaction()
{
  ctrl_->newChecksumTransaction();
}

void MetalinkParserStateMachine::setTypeOfChecksum(std::string type)
{
  ctrl_->setTypeOfChecksum(std::move(type));
}

void MetalinkParserStateMachine::setHashOfChecksum(std::string md)
{
  ctrl_->setHashOfChecksum(std::move(md));
}

void MetalinkParserStateMachine::commitChecksumTransaction()
{
  ctrl_->commitChecksumTransaction();
}

void MetalinkParserStateMachine::cancelChecksumTransaction()
{
  ctrl_->cancelChecksumTransaction();
}

void MetalinkParserStateMachine::newChunkChecksumTransactionV4()
{
  ctrl_->newChunkChecksumTransactionV4();
}

void MetalinkParserStateMachine::setLengthOfChunkChecksumV4(size_t length)
{
  ctrl_->setLengthOfChunkChecksumV4(length);
}

void MetalinkParserStateMachine::setTypeOfChunkChecksumV4(std::string type)
{
  ctrl_->setTypeOfChunkChecksumV4(std::move(type));
}

void MetalinkParserStateMachine::addHashOfChunkChecksumV4(std::string md)
{
  ctrl_->addHashOfChunkChecksumV4(std::move(md));
}

void MetalinkParserStateMachine::commitChunkChecksumTransactionV4()
{
  ctrl_->commitChunkChecksumTransactionV4();
}

void MetalinkParserStateMachine::cancelChunkChecksumTransactionV4()
{
  ctrl_->cancelChunkChecksumTransactionV4();
}

void MetalinkParserStateMachine::newChunkChecksumTransaction()
{
  ctrl_->newChunkChecksumTransaction();
}

void MetalinkParserStateMachine::setLengthOfChunkChecksum(size_t length)
{
  ctrl_->setLengthOfChunkChecksum(length);
}

void MetalinkParserStateMachine::setTypeOfChunkChecksum(std::string type)
{
  ctrl_->setTypeOfChunkChecksum(std::move(type));
}

void MetalinkParserStateMachine::createNewHashOfChunkChecksum(size_t order)
{
  ctrl_->createNewHashOfChunkChecksum(order);
}

void MetalinkParserStateMachine::setMessageDigestOfChunkChecksum(std::string md)
{
  ctrl_->setMessageDigestOfChunkChecksum(std::move(md));
}

void MetalinkParserStateMachine::addHashOfChunkChecksum()
{
  ctrl_->addHashOfChunkChecksum();
}

void MetalinkParserStateMachine::commitChunkChecksumTransaction()
{
  ctrl_->commitChunkChecksumTransaction();
}

void MetalinkParserStateMachine::cancelChunkChecksumTransaction()
{
  ctrl_->cancelChunkChecksumTransaction();
}

void MetalinkParserStateMachine::newSignatureTransaction()
{
  ctrl_->newSignatureTransaction();
}

void MetalinkParserStateMachine::setTypeOfSignature(std::string type)
{
  ctrl_->setTypeOfSignature(std::move(type));
}

void MetalinkParserStateMachine::setFileOfSignature(std::string file)
{
  ctrl_->setFileOfSignature(std::move(file));
}

void MetalinkParserStateMachine::setBodyOfSignature(std::string body)
{
  ctrl_->setBodyOfSignature(std::move(body));
}

void MetalinkParserStateMachine::commitSignatureTransaction()
{
  ctrl_->commitSignatureTransaction();
}

void MetalinkParserStateMachine::cancelSignatureTransaction()
{
  ctrl_->cancelSignatureTransaction();
}

void MetalinkParserStateMachine::newMetaurlTransaction()
{
  ctrl_->newMetaurlTransaction();
}

void MetalinkParserStateMachine::setURLOfMetaurl(std::string url)
{
  ctrl_->setURLOfMetaurl(std::move(url));
}

void MetalinkParserStateMachine::setMediatypeOfMetaurl(std::string mediatype)
{
  ctrl_->setMediatypeOfMetaurl(std::move(mediatype));
}

void MetalinkParserStateMachine::setPriorityOfMetaurl(int priority)
{
  ctrl_->setPriorityOfMetaurl(priority);
}

void MetalinkParserStateMachine::setNameOfMetaurl(std::string name)
{
  ctrl_->setNameOfMetaurl(std::move(name));
}

void MetalinkParserStateMachine::commitMetaurlTransaction()
{
  ctrl_->commitMetaurlTransaction();
}

void MetalinkParserStateMachine::cancelMetaurlTransaction()
{
  ctrl_->cancelMetaurlTransaction();
}

void MetalinkParserStateMachine::beginElement(const char* localname,
                                              const char* prefix,
                                              const char* nsUri,
                                              const std::vector<XmlAttr>& attrs)
{
  stateStack_.top()->beginElement(this, localname, prefix, nsUri, attrs);
}

void MetalinkParserStateMachine::endElement(const char* localname,
                                            const char* prefix,
                                            const char* nsUri,
                                            std::string characters)
{
  stateStack_.top()->endElement(this, localname, prefix, nsUri,
                                std::move(characters));
  stateStack_.pop();
}

bool MetalinkParserStateMachine::needsCharactersBuffering() const
{
  return stateStack_.top()->needsCharactersBuffering();
}

void MetalinkParserStateMachine::logError(std::string log)
{
  if (errors_.size() < 10) {
    errors_.push_back(std::move(log));
  }
}

std::string MetalinkParserStateMachine::getErrorString() const
{
  std::stringstream error;
  error << "Specification violation: ";
  std::copy(errors_.begin(), errors_.end(),
            std::ostream_iterator<std::string>(error, ", "));
  return error.str();
}

std::unique_ptr<Metalinker> MetalinkParserStateMachine::getResult()
{
  return ctrl_->getResult();
}

void MetalinkParserStateMachine::setBaseUri(std::string uri)
{
  ctrl_->setBaseUri(std::move(uri));
}

} // namespace aria2
