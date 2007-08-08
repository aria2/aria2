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
#include "Xml2MetalinkProcessor.h"
#include "DlAbortEx.h"
#include "Util.h"
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

Xml2MetalinkProcessor::Xml2MetalinkProcessor():doc(0), context(0) {}

Xml2MetalinkProcessor::~Xml2MetalinkProcessor() {
  release();
}

void Xml2MetalinkProcessor::release() {
  if(context) {
    xmlXPathFreeContext(context);
    context = 0;
  }
  if(doc) {
    xmlFreeDoc(doc);
    doc = 0;
  }
}

MetalinkerHandle Xml2MetalinkProcessor::parseFile(const string& filename) {
  release();
  doc = xmlParseFile(filename.c_str());
  if(!doc) {
    throw new DlAbortEx("Cannot parse metalink file %s", filename.c_str());
  }
  context = xmlXPathNewContext(doc);
  if(!context) {
    throw new DlAbortEx("Cannot create new xpath context");
  }
  string defaultNamespace = "http://www.metalinker.org/";
  if(xmlXPathRegisterNs(context, (xmlChar*)"m",
			(xmlChar*)defaultNamespace.c_str()) != 0) {
    throw new DlAbortEx("Cannot register namespace %s",
			defaultNamespace.c_str());
  }
  
  string xpath = "/m:metalink/m:files/m:file";
  MetalinkerHandle metalinker(new Metalinker());
  for(int index = 1; 1; index++) {
    MetalinkEntryHandle entry = getEntry(xpath+"["+Util::itos(index)+"]");
    if(!entry.get()) {
      break;
    } else {
      metalinker->entries.push_back(entry);
    }
  }
  return metalinker;
}

MetalinkEntryHandle Xml2MetalinkProcessor::getEntry(const string& xpath) {
  xmlXPathObjectPtr result = xpathEvaluation(xpath);
  if(!result) {
    return 0;
  }
  xmlNodeSetPtr nodeSet = result->nodesetval;
  xmlNodePtr node = nodeSet->nodeTab[0];
  string filename = Util::trim(xmlAttribute(node, "name"));
  xmlXPathFreeObject(result);

  MetalinkEntryHandle entry(new MetalinkEntry());

  FileEntryHandle fileEntry = new FileEntry(filename, 0, 0);
  
  string sizeStr = Util::trim(xpathContent(xpath+"/m:size"));
  if(sizeStr == "") {
    fileEntry->setLength(0);
  } else {
    fileEntry->setLength(strtoll(sizeStr.c_str(), 0, 10));
  }
  entry->file = fileEntry;
  entry->version = Util::trim(xpathContent(xpath+"/m:version"));
  entry->language = Util::trim(xpathContent(xpath+"/m:language"));
  entry->os = Util::trim(xpathContent(xpath+"/m:os"));
#ifdef ENABLE_MESSAGE_DIGEST
  xmlXPathObjectPtr hashPathObj = xpathEvaluation(xpath+"/m:verification/m:hash");
  if(hashPathObj) {
    xmlNodeSetPtr nodeSet = hashPathObj->nodesetval;
    for(int32_t i = 0; i < nodeSet->nodeNr; ++i) {
      xmlNodePtr node = nodeSet->nodeTab[i];
      string algo = Util::trim(xmlAttribute(node, "type"));
      if(MessageDigestContext::supports(algo)) {
	entry->checksum = new Checksum(algo, Util::trim(xmlContent(node)));
	break;
      }
    }
  }
  xmlXPathFreeObject(hashPathObj);

  string piecesPath = xpath+"/m:verification/m:pieces";
  xmlXPathObjectPtr pieceHashPathObj = xpathEvaluation(piecesPath);
  if(pieceHashPathObj) {
    xmlNodeSetPtr nodeSet = pieceHashPathObj->nodesetval;
    for(int32_t i = 0; i < nodeSet->nodeNr; ++i) {
      xmlNodePtr node = nodeSet->nodeTab[i];
      string algo = Util::trim(xmlAttribute(node, "type"));
      if(MessageDigestContext::supports(algo)) {
	entry->chunkChecksum = getPieceHash(piecesPath+"[@type=\""+algo+"\"]",
					    entry->getLength());
	break;
      }
    }
  }
  xmlXPathFreeObject(pieceHashPathObj);
  /*
  string piecesPath = xpath+"/m:verification/m:pieces";
  string sha1PiecesPath = piecesPath+"[@type=\"sha1\"]";
  string md5PiecesPath = piecesPath+"[@type=\"md5\"]";
  if(xpathExists(sha1PiecesPath)) {
    entry->chunkChecksum = getPieceHash(sha1PiecesPath, entry->getLength());
  } else if(xpathExists(md5PiecesPath)) {
    entry->chunkChecksum = getPieceHash(md5PiecesPath, entry->getLength());
  }
  */
#endif // ENABLE_MESSAGE_DIGEST
  for(int index = 1; 1; index++) {
    MetalinkResourceHandle resource(getResource(xpath+"/m:resources/m:url["+Util::itos(index)+"]"));
    if(!resource.get()) {
      break;
    } else {
      entry->resources.push_back(resource);
    }
  }
  return entry;
}

#ifdef ENABLE_MESSAGE_DIGEST
ChunkChecksumHandle Xml2MetalinkProcessor::getPieceHash(const string& xpath,
								int64_t totalSize)
{
  xmlXPathObjectPtr result = xpathEvaluation(xpath);
  if(!result) {
    return 0;
  }
  xmlNodeSetPtr nodeSet = result->nodesetval;
  xmlNodePtr node = nodeSet->nodeTab[0];

  int64_t checksumLength = STRTOLL(Util::trim(xmlAttribute(node, "length")).c_str());
  string algoString = Util::trim(xmlAttribute(node, "type"));
  xmlXPathFreeObject(result);

  if(!MessageDigestContext::supports(algoString)) {
    // unknown checksum type
    return 0;
  }

  Strings checksums;
  int64_t numPiece = (totalSize+checksumLength-1)/checksumLength;
  for(int64_t i = 0; i < numPiece; ++i) {
    string pieceHash = Util::trim(xpathContent(xpath+"/m:hash[@piece=\""+Util::ullitos(i)+"\"]"));
    if(pieceHash == "") {
      throw new DlAbortEx("Piece hash missing. index=%d", i);
    }
    checksums.push_back(pieceHash);
  }
  return new ChunkChecksum(algoString, checksums, checksumLength);
}
#endif // ENABLE_MESSAGE_DIGEST

MetalinkResourceHandle Xml2MetalinkProcessor::getResource(const string& xpath) {
  xmlXPathObjectPtr result = xpathEvaluation(xpath);
  if(!result) {
    return 0;
  }
  MetalinkResourceHandle resource(new MetalinkResource());

  xmlNodeSetPtr nodeSet = result->nodesetval;
  xmlNodePtr node = nodeSet->nodeTab[0];
  string type = Util::trim(xmlAttribute(node, "type"));

  if(type == "ftp") {
    resource->type = MetalinkResource::TYPE_FTP;
  } else if(type == "http") {
    resource->type = MetalinkResource::TYPE_HTTP;
  } else if(type == "https") {
    resource->type = MetalinkResource::TYPE_HTTPS;
  } else if(type == "bittorrent") {
    resource->type = MetalinkResource::TYPE_BITTORRENT;
  } else {
    resource->type = MetalinkResource::TYPE_NOT_SUPPORTED;
  }
  string pref = Util::trim(xmlAttribute(node, "preference"));
  if(pref.empty()) {
    resource->preference = 100;
  } else {
    resource->preference = STRTOLL(pref.c_str());
  }
  resource->location = Util::trim(xmlAttribute(node, "location"));

  resource->url = Util::trim(xmlContent(node));

  xmlXPathFreeObject(result);

  return resource;
}

xmlXPathObjectPtr Xml2MetalinkProcessor::xpathEvaluation(const string& xpath) {
  xmlXPathObjectPtr result = xmlXPathEvalExpression((xmlChar*)xpath.c_str(),
						    context);
  if(!result) {
    throw new DlAbortEx("Cannot evaluate xpath %s", xpath.c_str());
  }
  if(xmlXPathNodeSetIsEmpty(result->nodesetval)) {
    xmlXPathFreeObject(result);
    return 0;
  }
  return result;
}

string Xml2MetalinkProcessor::xmlAttribute(xmlNodePtr node, const string& attrName) {
  xmlChar* temp = xmlGetNoNsProp(node, (xmlChar*)attrName.c_str());
  if(!temp) {
    return "";
  } else {
    string attr = (char*)temp;
    xmlFree(temp);
    return attr;
  }
}

string Xml2MetalinkProcessor::xmlContent(xmlNodePtr node) {
  xmlChar* temp = xmlNodeGetContent(node);
  if(!temp) {
    return "";
  } else {
    string content = (char*)temp;
    xmlFree(temp);
    return content;
  }
}

string Xml2MetalinkProcessor::xpathContent(const string& xpath) {
  xmlXPathObjectPtr result = xpathEvaluation(xpath);
  if(!result) {
    return "";
  }
  xmlNodeSetPtr nodeSet = result->nodesetval;
  xmlNodePtr node = nodeSet->nodeTab[0]->children;
  string content = (char*)node->content;
  xmlXPathFreeObject(result);
  return content;
}

bool Xml2MetalinkProcessor::xpathExists(const string& xpath) {
  xmlXPathObjectPtr result = xpathEvaluation(xpath);
  bool retval = true;
  if(!result) {
    retval = false;
  }
  xmlXPathFreeObject(result);
  return retval;
}
