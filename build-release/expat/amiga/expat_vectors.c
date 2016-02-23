/*
** Copyright (c) 2001-2009 Expat maintainers.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <exec/types.h>
#include <exec/exec.h>
#include <exec/interfaces.h>
#include <interfaces/expat.h>

extern uint32                _Expat_Obtain(struct ExpatIFace *);
extern uint32                _Expat_Release(struct ExpatIFace *);
extern XML_Parser            _Expat_XML_ParserCreate(struct ExpatIFace *, const XML_Char * encodingName);
extern XML_Parser            _Expat_XML_ParserCreateNS(struct ExpatIFace *, const XML_Char * encodingName, XML_Char nsSep);
extern XML_Parser            _Expat_XML_ParserCreate_MM(struct ExpatIFace *, const XML_Char * encoding, const XML_Memory_Handling_Suite * memsuite, const XML_Char * namespaceSeparator);
extern XML_Parser            _Expat_XML_ExternalEntityParserCreate(struct ExpatIFace *, XML_Parser parser, const XML_Char * context, const XML_Char * encoding);
extern void                  _Expat_XML_ParserFree(struct ExpatIFace *, XML_Parser parser);
extern enum XML_Status       _Expat_XML_Parse(struct ExpatIFace *, XML_Parser parser, const char * s, int len, int isFinal);
extern enum XML_Status       _Expat_XML_ParseBuffer(struct ExpatIFace *, XML_Parser parser, int len, int isFinal);
extern void *                _Expat_XML_GetBuffer(struct ExpatIFace *, XML_Parser parser, int len);
extern void                  _Expat_XML_SetStartElementHandler(struct ExpatIFace *, XML_Parser parser, XML_StartElementHandler start);
extern void                  _Expat_XML_SetEndElementHandler(struct ExpatIFace *, XML_Parser parser, XML_EndElementHandler end);
extern void                  _Expat_XML_SetElementHandler(struct ExpatIFace *, XML_Parser parser, XML_StartElementHandler start, XML_EndElementHandler end);
extern void                  _Expat_XML_SetCharacterDataHandler(struct ExpatIFace *, XML_Parser parser, XML_CharacterDataHandler handler);
extern void                  _Expat_XML_SetProcessingInstructionHandler(struct ExpatIFace *, XML_Parser parser, XML_ProcessingInstructionHandler handler);
extern void                  _Expat_XML_SetCommentHandler(struct ExpatIFace *, XML_Parser parser, XML_CommentHandler handler);
extern void                  _Expat_XML_SetStartCdataSectionHandler(struct ExpatIFace *, XML_Parser parser, XML_StartCdataSectionHandler start);
extern void                  _Expat_XML_SetEndCdataSectionHandler(struct ExpatIFace *, XML_Parser parser, XML_EndCdataSectionHandler end);
extern void                  _Expat_XML_SetCdataSectionHandler(struct ExpatIFace *, XML_Parser parser, XML_StartCdataSectionHandler start, XML_EndCdataSectionHandler end);
extern void                  _Expat_XML_SetDefaultHandler(struct ExpatIFace *, XML_Parser parser, XML_DefaultHandler handler);
extern void                  _Expat_XML_SetDefaultHandlerExpand(struct ExpatIFace *, XML_Parser parser, XML_DefaultHandler handler);
extern void                  _Expat_XML_SetExternalEntityRefHandler(struct ExpatIFace *, XML_Parser parser, XML_ExternalEntityRefHandler handler);
extern void                  _Expat_XML_SetExternalEntityRefHandlerArg(struct ExpatIFace *, XML_Parser parser, void * arg);
extern void                  _Expat_XML_SetUnknownEncodingHandler(struct ExpatIFace *, XML_Parser parser, XML_UnknownEncodingHandler handler, void * data);
extern void                  _Expat_XML_SetStartNamespaceDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_StartNamespaceDeclHandler start);
extern void                  _Expat_XML_SetEndNamespaceDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_EndNamespaceDeclHandler end);
extern void                  _Expat_XML_SetNamespaceDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_StartNamespaceDeclHandler start, XML_EndNamespaceDeclHandler end);
extern void                  _Expat_XML_SetXmlDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_XmlDeclHandler handler);
extern void                  _Expat_XML_SetStartDoctypeDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_StartDoctypeDeclHandler start);
extern void                  _Expat_XML_SetEndDoctypeDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_EndDoctypeDeclHandler end);
extern void                  _Expat_XML_SetDoctypeDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_StartDoctypeDeclHandler start, XML_EndDoctypeDeclHandler end);
extern void                  _Expat_XML_SetElementDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_ElementDeclHandler eldecl);
extern void                  _Expat_XML_SetAttlistDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_AttlistDeclHandler attdecl);
extern void                  _Expat_XML_SetEntityDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_EntityDeclHandler handler);
extern void                  _Expat_XML_SetUnparsedEntityDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_UnparsedEntityDeclHandler handler);
extern void                  _Expat_XML_SetNotationDeclHandler(struct ExpatIFace *, XML_Parser parser, XML_NotationDeclHandler handler);
extern void                  _Expat_XML_SetNotStandaloneHandler(struct ExpatIFace *, XML_Parser parser, XML_NotStandaloneHandler handler);
extern enum XML_Error        _Expat_XML_GetErrorCode(struct ExpatIFace *, XML_Parser parser);
extern const XML_LChar *     _Expat_XML_ErrorString(struct ExpatIFace *, enum XML_Error code);
extern long                  _Expat_XML_GetCurrentByteIndex(struct ExpatIFace *, XML_Parser parser);
extern int                   _Expat_XML_GetCurrentLineNumber(struct ExpatIFace *, XML_Parser parser);
extern int                   _Expat_XML_GetCurrentColumnNumber(struct ExpatIFace *, XML_Parser parser);
extern int                   _Expat_XML_GetCurrentByteCount(struct ExpatIFace *, XML_Parser parser);
extern const char *          _Expat_XML_GetInputContext(struct ExpatIFace *, XML_Parser parser, int * offset, int * size);
extern void                  _Expat_XML_SetUserData(struct ExpatIFace *, XML_Parser parser, void * userData);
extern void                  _Expat_XML_DefaultCurrent(struct ExpatIFace *, XML_Parser parser);
extern void                  _Expat_XML_UseParserAsHandlerArg(struct ExpatIFace *, XML_Parser parser);
extern enum XML_Status       _Expat_XML_SetBase(struct ExpatIFace *, XML_Parser parser, const XML_Char * base);
extern const XML_Char *      _Expat_XML_GetBase(struct ExpatIFace *, XML_Parser parser);
extern int                   _Expat_XML_GetSpecifiedAttributeCount(struct ExpatIFace *, XML_Parser parser);
extern int                   _Expat_XML_GetIdAttributeIndex(struct ExpatIFace *, XML_Parser parser);
extern enum XML_Status       _Expat_XML_SetEncoding(struct ExpatIFace *, XML_Parser parser, const XML_Char * encoding);
extern int                   _Expat_XML_SetParamEntityParsing(struct ExpatIFace *, XML_Parser parser, enum XML_ParamEntityParsing parsing);
extern void                  _Expat_XML_SetReturnNSTriplet(struct ExpatIFace *, XML_Parser parser, int do_nst);
extern const XML_LChar *     _Expat_XML_ExpatVersion(struct ExpatIFace *);
extern XML_Expat_Version     _Expat_XML_ExpatVersionInfo(struct ExpatIFace *);
extern XML_Bool              _Expat_XML_ParserReset(struct ExpatIFace *, XML_Parser parser, const XML_Char * encoding);
extern void                  _Expat_XML_SetSkippedEntityHandler(struct ExpatIFace *, XML_Parser parser, XML_SkippedEntityHandler handler);
extern enum XML_Error        _Expat_XML_UseForeignDTD(struct ExpatIFace *, XML_Parser parser, XML_Bool useDTD);
extern const XML_Feature *   _Expat_XML_GetFeatureList(struct ExpatIFace *);
extern enum XML_Status       _Expat_XML_StopParser(struct ExpatIFace *, XML_Parser parser, XML_Bool resumable);
extern enum XML_Status       _Expat_XML_ResumeParser(struct ExpatIFace *, XML_Parser parser);
extern void                  _Expat_XML_GetParsingStatus(struct ExpatIFace *, XML_Parser parser, XML_ParsingStatus * status);
extern void                  _Expat_XML_FreeContentModel(struct ExpatIFace *, XML_Parser parser, XML_Content * model);
extern void *                _Expat_XML_MemMalloc(struct ExpatIFace *, XML_Parser parser, size_t size);
extern void *                _Expat_XML_MemRealloc(struct ExpatIFace *, XML_Parser parser, void * ptr, size_t size);
extern void                  _Expat_XML_MemFree(struct ExpatIFace *, XML_Parser parser, void * ptr);


CONST APTR main_vectors[] =
{
    _Expat_Obtain,
    _Expat_Release,
    NULL,
    NULL,
    _Expat_XML_ParserCreate,
    _Expat_XML_ParserCreateNS,
    _Expat_XML_ParserCreate_MM,
    _Expat_XML_ExternalEntityParserCreate,
    _Expat_XML_ParserFree,
    _Expat_XML_Parse,
    _Expat_XML_ParseBuffer,
    _Expat_XML_GetBuffer,
    _Expat_XML_SetStartElementHandler,
    _Expat_XML_SetEndElementHandler,
    _Expat_XML_SetElementHandler,
    _Expat_XML_SetCharacterDataHandler,
    _Expat_XML_SetProcessingInstructionHandler,
    _Expat_XML_SetCommentHandler,
    _Expat_XML_SetStartCdataSectionHandler,
    _Expat_XML_SetEndCdataSectionHandler,
    _Expat_XML_SetCdataSectionHandler,
    _Expat_XML_SetDefaultHandler,
    _Expat_XML_SetDefaultHandlerExpand,
    _Expat_XML_SetExternalEntityRefHandler,
    _Expat_XML_SetExternalEntityRefHandlerArg,
    _Expat_XML_SetUnknownEncodingHandler,
    _Expat_XML_SetStartNamespaceDeclHandler,
    _Expat_XML_SetEndNamespaceDeclHandler,
    _Expat_XML_SetNamespaceDeclHandler,
    _Expat_XML_SetXmlDeclHandler,
    _Expat_XML_SetStartDoctypeDeclHandler,
    _Expat_XML_SetEndDoctypeDeclHandler,
    _Expat_XML_SetDoctypeDeclHandler,
    _Expat_XML_SetElementDeclHandler,
    _Expat_XML_SetAttlistDeclHandler,
    _Expat_XML_SetEntityDeclHandler,
    _Expat_XML_SetUnparsedEntityDeclHandler,
    _Expat_XML_SetNotationDeclHandler,
    _Expat_XML_SetNotStandaloneHandler,
    _Expat_XML_GetErrorCode,
    _Expat_XML_ErrorString,
    _Expat_XML_GetCurrentByteIndex,
    _Expat_XML_GetCurrentLineNumber,
    _Expat_XML_GetCurrentColumnNumber,
    _Expat_XML_GetCurrentByteCount,
    _Expat_XML_GetInputContext,
    _Expat_XML_SetUserData,
    _Expat_XML_DefaultCurrent,
    _Expat_XML_UseParserAsHandlerArg,
    _Expat_XML_SetBase,
    _Expat_XML_GetBase,
    _Expat_XML_GetSpecifiedAttributeCount,
    _Expat_XML_GetIdAttributeIndex,
    _Expat_XML_SetEncoding,
    _Expat_XML_SetParamEntityParsing,
    _Expat_XML_SetReturnNSTriplet,
    _Expat_XML_ExpatVersion,
    _Expat_XML_ExpatVersionInfo,
    _Expat_XML_ParserReset,
    _Expat_XML_SetSkippedEntityHandler,
    _Expat_XML_UseForeignDTD,
    _Expat_XML_GetFeatureList,
    _Expat_XML_StopParser,
    _Expat_XML_ResumeParser,
    _Expat_XML_GetParsingStatus,
    _Expat_XML_FreeContentModel,
    _Expat_XML_MemMalloc,
    _Expat_XML_MemRealloc,
    _Expat_XML_MemFree,
    (APTR)-1
};

uint32 _Expat_Obtain(struct ExpatIFace *Self)
{
	return ++Self->Data.RefCount;
}

uint32 _Expat_Release(struct ExpatIFace *Self)
{
	return --Self->Data.RefCount;
}

XML_Parser _Expat_XML_ParserCreate(struct ExpatIFace * Self, const XML_Char *encoding)
{
	return XML_ParserCreate(encoding);
}

XML_Parser _Expat_XML_ParserCreateNS(struct ExpatIFace * Self, const XML_Char *encoding, XML_Char nsSep)
{
	return XML_ParserCreateNS(encoding, nsSep);
}

XML_Parser _Expat_XML_ParserCreate_MM(struct ExpatIFace * Self, const XML_Char *encoding, const XML_Memory_Handling_Suite *memsuite, const XML_Char *namespaceSeparator)
{
	return XML_ParserCreate_MM(encoding, memsuite, namespaceSeparator);
}

XML_Parser _Expat_XML_ExternalEntityParserCreate(struct ExpatIFace * Self, XML_Parser parser, const XML_Char *context, const XML_Char *encoding)
{
	return XML_ExternalEntityParserCreate(parser, context, encoding);
}

void _Expat_XML_ParserFree(struct ExpatIFace *Self, XML_Parser parser)
{
	XML_ParserFree(parser);
}

enum XML_Status _Expat_XML_Parse(struct ExpatIFace * Self, XML_Parser parser, const char * s, int len, int isFinal)
{
	return XML_Parse(parser, s, len, isFinal);
}

enum XML_Status _Expat_XML_ParseBuffer(struct ExpatIFace * Self, XML_Parser parser, int len, int isFinal)
{
	return XML_ParseBuffer(parser, len, isFinal);
}

void * _Expat_XML_GetBuffer(struct ExpatIFace * Self, XML_Parser parser, int len)
{
	return XML_GetBuffer(parser, len);
}

void _Expat_XML_SetStartElementHandler(struct ExpatIFace * Self, XML_Parser parser, XML_StartElementHandler start)
{
	XML_SetStartElementHandler(parser, start);
}

void _Expat_XML_SetEndElementHandler(struct ExpatIFace * Self, XML_Parser parser, XML_EndElementHandler end)
{
	XML_SetEndElementHandler(parser, end);
}

void _Expat_XML_SetElementHandler(struct ExpatIFace * Self, XML_Parser parser, XML_StartElementHandler start, XML_EndElementHandler end)
{
	XML_SetElementHandler(parser, start, end);
}

void _Expat_XML_SetCharacterDataHandler(struct ExpatIFace * Self, XML_Parser parser, XML_CharacterDataHandler handler)
{
	XML_SetCharacterDataHandler(parser, handler);
}

void _Expat_XML_SetProcessingInstructionHandler(struct ExpatIFace * Self, XML_Parser parser, XML_ProcessingInstructionHandler handler)
{
	XML_SetProcessingInstructionHandler(parser, handler);
}

void _Expat_XML_SetCommentHandler(struct ExpatIFace * Self, XML_Parser parser, XML_CommentHandler handler)
{
	XML_SetCommentHandler(parser, handler);
}

void _Expat_XML_SetStartCdataSectionHandler(struct ExpatIFace * Self, XML_Parser parser, XML_StartCdataSectionHandler start)
{
	XML_SetStartCdataSectionHandler(parser, start);
}

void _Expat_XML_SetEndCdataSectionHandler(struct ExpatIFace * Self, XML_Parser parser, XML_EndCdataSectionHandler end)
{
	XML_SetEndCdataSectionHandler(parser, end);
}

void _Expat_XML_SetCdataSectionHandler(struct ExpatIFace * Self, XML_Parser parser, XML_StartCdataSectionHandler start, XML_EndCdataSectionHandler end)
{
	XML_SetCdataSectionHandler(parser, start, end);
}

void _Expat_XML_SetDefaultHandler(struct ExpatIFace * Self, XML_Parser parser, XML_DefaultHandler handler)
{
	XML_SetDefaultHandler(parser, handler);
}

void _Expat_XML_SetDefaultHandlerExpand(struct ExpatIFace * Self, XML_Parser parser, XML_DefaultHandler handler)
{
	XML_SetDefaultHandlerExpand(parser, handler);
}

void _Expat_XML_SetExternalEntityRefHandler(struct ExpatIFace * Self, XML_Parser parser, XML_ExternalEntityRefHandler handler)
{
	XML_SetExternalEntityRefHandler(parser, handler);
}

void _Expat_XML_SetExternalEntityRefHandlerArg(struct ExpatIFace * Self, XML_Parser parser, void * arg)
{
	XML_SetExternalEntityRefHandlerArg(parser, arg);
}

void _Expat_XML_SetUnknownEncodingHandler(struct ExpatIFace * Self, XML_Parser parser, XML_UnknownEncodingHandler handler, void * data)
{
	XML_SetUnknownEncodingHandler(parser, handler, data);
}

void _Expat_XML_SetStartNamespaceDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_StartNamespaceDeclHandler start)
{
	XML_SetStartNamespaceDeclHandler(parser, start);
}

void _Expat_XML_SetEndNamespaceDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_EndNamespaceDeclHandler end)
{
	XML_SetEndNamespaceDeclHandler(parser, end);
}

void _Expat_XML_SetNamespaceDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_StartNamespaceDeclHandler start, XML_EndNamespaceDeclHandler end)
{
	XML_SetNamespaceDeclHandler(parser, start, end);
}

void _Expat_XML_SetXmlDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_XmlDeclHandler handler)
{
	XML_SetXmlDeclHandler(parser, handler);
}

void _Expat_XML_SetStartDoctypeDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_StartDoctypeDeclHandler start)
{
	XML_SetStartDoctypeDeclHandler(parser, start);
}

void _Expat_XML_SetEndDoctypeDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_EndDoctypeDeclHandler end)
{
	XML_SetEndDoctypeDeclHandler(parser, end);
}

void _Expat_XML_SetDoctypeDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_StartDoctypeDeclHandler start, XML_EndDoctypeDeclHandler end)
{
	XML_SetDoctypeDeclHandler(parser, start, end);
}

void _Expat_XML_SetElementDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_ElementDeclHandler eldecl)
{
	XML_SetElementDeclHandler(parser, eldecl);
}

void _Expat_XML_SetAttlistDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_AttlistDeclHandler attdecl)
{
	XML_SetAttlistDeclHandler(parser, attdecl);
}

void _Expat_XML_SetEntityDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_EntityDeclHandler handler)
{
	XML_SetEntityDeclHandler(parser, handler);
}

void _Expat_XML_SetUnparsedEntityDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_UnparsedEntityDeclHandler handler)
{
	XML_SetUnparsedEntityDeclHandler(parser, handler);
}

void _Expat_XML_SetNotationDeclHandler(struct ExpatIFace * Self, XML_Parser parser, XML_NotationDeclHandler handler)
{
	XML_SetNotationDeclHandler(parser, handler);
}

void _Expat_XML_SetNotStandaloneHandler(struct ExpatIFace * Self, XML_Parser parser, XML_NotStandaloneHandler handler)
{
	XML_SetNotStandaloneHandler(parser, handler);
}

enum XML_Error _Expat_XML_GetErrorCode(struct ExpatIFace * Self, XML_Parser parser)
{
	return XML_GetErrorCode(parser);
}

const XML_LChar * _Expat_XML_ErrorString(struct ExpatIFace * Self, enum XML_Error code)
{
	return XML_ErrorString(code);
}

long _Expat_XML_GetCurrentByteIndex(struct ExpatIFace * Self, XML_Parser parser)
{
	return XML_GetCurrentByteIndex(parser);
}

int _Expat_XML_GetCurrentLineNumber(struct ExpatIFace * Self, XML_Parser parser)
{
	return XML_GetCurrentLineNumber(parser);
}

int _Expat_XML_GetCurrentColumnNumber(struct ExpatIFace * Self, XML_Parser parser)
{
	return XML_GetCurrentColumnNumber(parser);
}

int _Expat_XML_GetCurrentByteCount(struct ExpatIFace * Self, XML_Parser parser)
{
	return XML_GetCurrentByteCount(parser);
}

const char * _Expat_XML_GetInputContext(struct ExpatIFace * Self, XML_Parser parser, int * offset, int * size)
{
	return XML_GetInputContext(parser, offset, size);
}

void _Expat_XML_SetUserData(struct ExpatIFace * Self, XML_Parser parser, void * userData)
{
	XML_SetUserData(parser, userData);
}

void _Expat_XML_DefaultCurrent(struct ExpatIFace * Self, XML_Parser parser)
{
	XML_DefaultCurrent(parser);
}

void _Expat_XML_UseParserAsHandlerArg(struct ExpatIFace * Self, XML_Parser parser)
{
	XML_UseParserAsHandlerArg(parser);
}

enum XML_Status _Expat_XML_SetBase(struct ExpatIFace * Self, XML_Parser parser, const XML_Char *p)
{
	return XML_SetBase(parser, p);
}

const XML_Char * _Expat_XML_GetBase(struct ExpatIFace * Self, XML_Parser parser)
{
	return XML_GetBase(parser);
}

int _Expat_XML_GetSpecifiedAttributeCount(struct ExpatIFace * Self, XML_Parser parser)
{
	return XML_GetSpecifiedAttributeCount(parser);
}

int _Expat_XML_GetIdAttributeIndex(struct ExpatIFace * Self, XML_Parser parser)
{
	return XML_GetIdAttributeIndex(parser);
}

enum XML_Status _Expat_XML_SetEncoding(struct ExpatIFace * Self, XML_Parser parser, const XML_Char *encoding)
{
	return XML_SetEncoding(parser, encoding);
}

int _Expat_XML_SetParamEntityParsing(struct ExpatIFace * Self, XML_Parser parser, enum XML_ParamEntityParsing parsing)
{
	return XML_SetParamEntityParsing(parser, parsing);
}

void _Expat_XML_SetReturnNSTriplet(struct ExpatIFace * Self, XML_Parser parser, int do_nst)
{
	XML_SetReturnNSTriplet(parser, do_nst);
}

const XML_LChar * _Expat_XML_ExpatVersion(struct ExpatIFace * Self)
{
	return XML_ExpatVersion();
}

XML_Expat_Version _Expat_XML_ExpatVersionInfo(struct ExpatIFace * Self)
{
	return XML_ExpatVersionInfo();
}

XML_Bool _Expat_XML_ParserReset(struct ExpatIFace * Self, XML_Parser parser, const XML_Char *encoding)
{
	return XML_ParserReset(parser, encoding);
}

void _Expat_XML_SetSkippedEntityHandler(struct ExpatIFace * Self, XML_Parser parser, XML_SkippedEntityHandler handler)
{
	XML_SetSkippedEntityHandler(parser, handler);
}

enum XML_Error _Expat_XML_UseForeignDTD(struct ExpatIFace * Self, XML_Parser parser, XML_Bool useDTD)
{
	return XML_UseForeignDTD(parser, useDTD);
}

const XML_Feature * _Expat_XML_GetFeatureList(struct ExpatIFace * Self)
{
	return XML_GetFeatureList();
}

enum XML_Status _Expat_XML_StopParser(struct ExpatIFace * Self, XML_Parser parser, XML_Bool resumable)
{
	return XML_StopParser(parser, resumable);
}

enum XML_Status _Expat_XML_ResumeParser(struct ExpatIFace * Self, XML_Parser parser)
{
	return XML_ResumeParser(parser);
}

void _Expat_XML_GetParsingStatus(struct ExpatIFace * Self, XML_Parser parser, XML_ParsingStatus * status)
{
	XML_GetParsingStatus(parser, status);
}

void _Expat_XML_FreeContentModel(struct ExpatIFace * Self, XML_Parser parser, XML_Content * model)
{
	XML_FreeContentModel(parser, model);
}

void * _Expat_XML_MemMalloc(struct ExpatIFace * Self, XML_Parser parser, size_t size)
{
	return XML_MemMalloc(parser, size);
}

void * _Expat_XML_MemRealloc(struct ExpatIFace * Self, XML_Parser parser, void * ptr, size_t size)
{
	XML_MemRealloc(parser, ptr, size);
}

void _Expat_XML_MemFree(struct ExpatIFace * Self, XML_Parser parser, void * ptr)
{
	XML_MemFree(parser, ptr);
}
