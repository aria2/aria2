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
#include "MetalinkParserStateImpl.h"
#include "MetalinkParserStateV3Impl.h"
#include "MetalinkParserStateV4Impl.h"
#include "Metalinker.h"
#include "MetalinkEntry.h"

namespace aria2 {

MetalinkParserState* MetalinkParserStateMachine::_initialState =
  new InitialMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_skipTagState =
  new SkipTagMetalinkParserState();

MetalinkParserState* MetalinkParserStateMachine::_metalinkState =
  new MetalinkMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_filesState =
  new FilesMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_fileState =
  new FileMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_sizeState =
  new SizeMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_versionState =
  new VersionMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_languageState =
  new LanguageMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_osState =
  new OSMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_verificationState =
  new VerificationMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_hashState =
  new HashMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_piecesState =
  new PiecesMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_pieceHashState =
  new PieceHashMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_signatureState =
  new SignatureMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_resourcesState =
  new ResourcesMetalinkParserState();
MetalinkParserState* MetalinkParserStateMachine::_urlState =
  new URLMetalinkParserState();

MetalinkParserState* MetalinkParserStateMachine::_metalinkStateV4 =
  new MetalinkMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_fileStateV4 =
  new FileMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_sizeStateV4 =
  new SizeMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_versionStateV4 =
  new VersionMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_languageStateV4 =
  new LanguageMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_osStateV4 =
  new OSMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_hashStateV4 =
  new HashMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_piecesStateV4 =
  new PiecesMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_pieceHashStateV4 =
  new PieceHashMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_signatureStateV4 =
  new SignatureMetalinkParserStateV4();
MetalinkParserState* MetalinkParserStateMachine::_urlStateV4 =
  new URLMetalinkParserStateV4();

MetalinkParserStateMachine::MetalinkParserStateMachine():
  _ctrl(new MetalinkParserController())
{
  _stateStack.push(_initialState);
}

void MetalinkParserStateMachine::setMetalinkState()
{
  _stateStack.push(_metalinkState);
}

void MetalinkParserStateMachine::setFilesState()
{
  _stateStack.push(_filesState);
}

void MetalinkParserStateMachine::setFileState()
{
  _stateStack.push(_fileState);
}

void MetalinkParserStateMachine::setSizeState()
{
  _stateStack.push(_sizeState);
}

void MetalinkParserStateMachine::setVersionState()
{
  _stateStack.push(_versionState);
}

void MetalinkParserStateMachine::setLanguageState()
{
  _stateStack.push(_languageState);
}

void MetalinkParserStateMachine::setOSState()
{
  _stateStack.push(_osState);
}

void MetalinkParserStateMachine::setVerificationState()
{
  _stateStack.push(_verificationState);
}

void MetalinkParserStateMachine::setHashState()
{
  _stateStack.push(_hashState);
}

void MetalinkParserStateMachine::setPiecesState()
{
  _stateStack.push(_piecesState);
}

void MetalinkParserStateMachine::setPieceHashState()
{
  _stateStack.push(_pieceHashState);
}

void MetalinkParserStateMachine::setSignatureState()
{
  _stateStack.push(_signatureState);
}

void MetalinkParserStateMachine::setResourcesState()
{
  _stateStack.push(_resourcesState);
}

void MetalinkParserStateMachine::setURLState()
{
  _stateStack.push(_urlState);
}

void MetalinkParserStateMachine::setMetalinkStateV4()
{
  _stateStack.push(_metalinkStateV4);
}

void MetalinkParserStateMachine::setFileStateV4()
{
  _stateStack.push(_fileStateV4);
}

void MetalinkParserStateMachine::setSizeStateV4()
{
  _stateStack.push(_sizeStateV4);
}

void MetalinkParserStateMachine::setVersionStateV4()
{
  _stateStack.push(_versionStateV4);
}

void MetalinkParserStateMachine::setLanguageStateV4()
{
  _stateStack.push(_languageStateV4);
}

void MetalinkParserStateMachine::setOSStateV4()
{
  _stateStack.push(_osStateV4);
}

void MetalinkParserStateMachine::setHashStateV4()
{
  _stateStack.push(_hashStateV4);
}

void MetalinkParserStateMachine::setPiecesStateV4()
{
  _stateStack.push(_piecesStateV4);
}

void MetalinkParserStateMachine::setPieceHashStateV4()
{
  _stateStack.push(_pieceHashStateV4);
}

void MetalinkParserStateMachine::setSignatureStateV4()
{
  _stateStack.push(_signatureStateV4);
}

void MetalinkParserStateMachine::setURLStateV4()
{
  _stateStack.push(_urlStateV4);
}

void MetalinkParserStateMachine::setSkipTagState()
{
  _stateStack.push(_skipTagState);
}

bool MetalinkParserStateMachine::finished() const
{
  return _stateStack.top() == _initialState;
}

void MetalinkParserStateMachine::newEntryTransaction()
{
  _ctrl->newEntryTransaction();
}

void MetalinkParserStateMachine::setFileNameOfEntry(const std::string& filename)
{
  _ctrl->setFileNameOfEntry(filename);
}

void MetalinkParserStateMachine::setFileLengthOfEntry(uint64_t length)
{
  _ctrl->setFileLengthOfEntry(length);
}

void MetalinkParserStateMachine::setVersionOfEntry(const std::string& version)
{
  _ctrl->setVersionOfEntry(version);
}

void MetalinkParserStateMachine::setLanguageOfEntry(const std::string& language)
{
  _ctrl->setLanguageOfEntry(language);
}

void MetalinkParserStateMachine::setOSOfEntry(const std::string& os)
{
  _ctrl->setOSOfEntry(os);
}

void MetalinkParserStateMachine::setMaxConnectionsOfEntry(int maxConnections)
{
  _ctrl->setMaxConnectionsOfEntry(maxConnections);
}

void MetalinkParserStateMachine::commitEntryTransaction()
{
  _ctrl->commitEntryTransaction();
}

void MetalinkParserStateMachine::newResourceTransaction()
{
  _ctrl->newResourceTransaction();
}

void MetalinkParserStateMachine::setURLOfResource(const std::string& url)
{
  _ctrl->setURLOfResource(url);
}

void MetalinkParserStateMachine::setTypeOfResource(const std::string& type)
{
  _ctrl->setTypeOfResource(type);
}

void MetalinkParserStateMachine::setLocationOfResource
(const std::string& location)
{
  _ctrl->setLocationOfResource(location);
}

void MetalinkParserStateMachine::setPriorityOfResource(int priority)
{
  _ctrl->setPriorityOfResource(priority);
}

void MetalinkParserStateMachine::setMaxConnectionsOfResource(int maxConnections)
{
  _ctrl->setMaxConnectionsOfResource(maxConnections);
}

void MetalinkParserStateMachine::commitResourceTransaction()
{
  _ctrl->commitResourceTransaction();
}

void MetalinkParserStateMachine::cancelResourceTransaction()
{
  _ctrl->cancelResourceTransaction();
}

void MetalinkParserStateMachine::newChecksumTransaction()
{
  _ctrl->newChecksumTransaction();
}

void MetalinkParserStateMachine::setTypeOfChecksum(const std::string& type)
{
  _ctrl->setTypeOfChecksum(type);
}

void MetalinkParserStateMachine::setHashOfChecksum(const std::string& md)
{
  _ctrl->setHashOfChecksum(md);
}

void MetalinkParserStateMachine::commitChecksumTransaction()
{
  _ctrl->commitChecksumTransaction();
}

void MetalinkParserStateMachine::cancelChecksumTransaction()
{
  _ctrl->cancelChecksumTransaction();
}

void MetalinkParserStateMachine::newChunkChecksumTransactionV4()
{
  _ctrl->newChunkChecksumTransactionV4();
}

void MetalinkParserStateMachine::setLengthOfChunkChecksumV4(size_t length)
{
  _ctrl->setLengthOfChunkChecksumV4(length);
}

void MetalinkParserStateMachine::setTypeOfChunkChecksumV4
(const std::string& type)
{
  _ctrl->setTypeOfChunkChecksumV4(type);
}

void MetalinkParserStateMachine::addHashOfChunkChecksumV4(const std::string& md)
{
  _ctrl->addHashOfChunkChecksumV4(md);
}

void MetalinkParserStateMachine::commitChunkChecksumTransactionV4()
{
  _ctrl->commitChunkChecksumTransactionV4();
}

void MetalinkParserStateMachine::cancelChunkChecksumTransactionV4()
{
  _ctrl->cancelChunkChecksumTransactionV4();
}

void MetalinkParserStateMachine::newChunkChecksumTransaction()
{
  _ctrl->newChunkChecksumTransaction();
}

void MetalinkParserStateMachine::setLengthOfChunkChecksum(size_t length)
{
  _ctrl->setLengthOfChunkChecksum(length);
}

void MetalinkParserStateMachine::setTypeOfChunkChecksum(const std::string& type)
{
  _ctrl->setTypeOfChunkChecksum(type);
}

void MetalinkParserStateMachine::createNewHashOfChunkChecksum(size_t order)
{
  _ctrl->createNewHashOfChunkChecksum(order);
}

void MetalinkParserStateMachine::setMessageDigestOfChunkChecksum
(const std::string& md)
{
  _ctrl->setMessageDigestOfChunkChecksum(md);
}

void MetalinkParserStateMachine::addHashOfChunkChecksum()
{
  _ctrl->addHashOfChunkChecksum();
}

void MetalinkParserStateMachine::commitChunkChecksumTransaction()
{
  _ctrl->commitChunkChecksumTransaction();
}

void MetalinkParserStateMachine::cancelChunkChecksumTransaction()
{
  _ctrl->cancelChunkChecksumTransaction();
}

void MetalinkParserStateMachine::newSignatureTransaction()
{
  _ctrl->newSignatureTransaction();
}

void MetalinkParserStateMachine::setTypeOfSignature(const std::string& type)
{
  _ctrl->setTypeOfSignature(type);
}

void MetalinkParserStateMachine::setFileOfSignature(const std::string& file)
{
  _ctrl->setFileOfSignature(file);
}

void MetalinkParserStateMachine::setBodyOfSignature(const std::string& body)
{
  _ctrl->setBodyOfSignature(body);
}

void MetalinkParserStateMachine::commitSignatureTransaction()
{
  _ctrl->commitSignatureTransaction();
}

void MetalinkParserStateMachine::cancelSignatureTransaction()
{
  _ctrl->cancelSignatureTransaction();
}

void MetalinkParserStateMachine::beginElement
(const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::vector<XmlAttr>& attrs)
{
  _stateStack.top()->beginElement(this, localname, prefix, nsUri, attrs);
}
  
void MetalinkParserStateMachine::endElement
(const std::string& localname,
 const std::string& prefix,
 const std::string& nsUri,
 const std::string& characters)
{
  _stateStack.top()->endElement(this, localname, prefix, nsUri, characters);
  _stateStack.pop();
}

bool MetalinkParserStateMachine::needsCharactersBuffering() const
{
  return _stateStack.top()->needsCharactersBuffering();
}

} // namespace aria2
