#ifndef LIBRARIES_EXPAT_H
#define LIBRARIES_EXPAT_H

/*
** Copyright (c) 2001-2007 Expat maintainers.
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


/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack(2)
   #endif
#elif defined(__VBCC__)
   #pragma amiga-align
#endif

/****************************************************************************/


#include <stdlib.h>

#ifndef XMLCALL
#define XMLCALL
#endif

typedef char XML_Char;
typedef char XML_LChar;
typedef long XML_Index;
typedef unsigned long XML_Size;

struct XML_ParserStruct;
typedef struct XML_ParserStruct *XML_Parser;

typedef unsigned char XML_Bool;
#define XML_TRUE   ((XML_Bool) 1)
#define XML_FALSE  ((XML_Bool) 0)

enum XML_Status {
  XML_STATUS_ERROR = 0,
#define XML_STATUS_ERROR XML_STATUS_ERROR
  XML_STATUS_OK = 1,
#define XML_STATUS_OK XML_STATUS_OK
  XML_STATUS_SUSPENDED = 2,
#define XML_STATUS_SUSPENDED XML_STATUS_SUSPENDED
};

enum XML_Error {
  XML_ERROR_NONE,
  XML_ERROR_NO_MEMORY,
  XML_ERROR_SYNTAX,
  XML_ERROR_NO_ELEMENTS,
  XML_ERROR_INVALID_TOKEN,
  XML_ERROR_UNCLOSED_TOKEN,
  XML_ERROR_PARTIAL_CHAR,
  XML_ERROR_TAG_MISMATCH,
  XML_ERROR_DUPLICATE_ATTRIBUTE,
  XML_ERROR_JUNK_AFTER_DOC_ELEMENT,
  XML_ERROR_PARAM_ENTITY_REF,
  XML_ERROR_UNDEFINED_ENTITY,
  XML_ERROR_RECURSIVE_ENTITY_REF,
  XML_ERROR_ASYNC_ENTITY,
  XML_ERROR_BAD_CHAR_REF,
  XML_ERROR_BINARY_ENTITY_REF,
  XML_ERROR_ATTRIBUTE_EXTERNAL_ENTITY_REF,
  XML_ERROR_MISPLACED_XML_PI,
  XML_ERROR_UNKNOWN_ENCODING,
  XML_ERROR_INCORRECT_ENCODING,
  XML_ERROR_UNCLOSED_CDATA_SECTION,
  XML_ERROR_EXTERNAL_ENTITY_HANDLING,
  XML_ERROR_NOT_STANDALONE,
  XML_ERROR_UNEXPECTED_STATE,
  XML_ERROR_ENTITY_DECLARED_IN_PE,
  XML_ERROR_FEATURE_REQUIRES_XML_DTD,
  XML_ERROR_CANT_CHANGE_FEATURE_ONCE_PARSING,
  XML_ERROR_UNBOUND_PREFIX,
  XML_ERROR_UNDECLARING_PREFIX,
  XML_ERROR_INCOMPLETE_PE,
  XML_ERROR_XML_DECL,
  XML_ERROR_TEXT_DECL,
  XML_ERROR_PUBLICID,
  XML_ERROR_SUSPENDED,
  XML_ERROR_NOT_SUSPENDED,
  XML_ERROR_ABORTED,
  XML_ERROR_FINISHED,
  XML_ERROR_SUSPEND_PE,
  XML_ERROR_RESERVED_PREFIX_XML,
  XML_ERROR_RESERVED_PREFIX_XMLNS,
  XML_ERROR_RESERVED_NAMESPACE_URI
};

enum XML_Content_Type {
  XML_CTYPE_EMPTY = 1,
  XML_CTYPE_ANY,
  XML_CTYPE_MIXED,
  XML_CTYPE_NAME,
  XML_CTYPE_CHOICE,
  XML_CTYPE_SEQ
};

enum XML_Content_Quant {
  XML_CQUANT_NONE,
  XML_CQUANT_OPT,
  XML_CQUANT_REP,
  XML_CQUANT_PLUS
};

typedef struct XML_cp XML_Content;

struct XML_cp {
  enum XML_Content_Type         type;
  enum XML_Content_Quant        quant;
  XML_Char *                    name;
  unsigned int                  numchildren;
  XML_Content *                 children;
};


typedef void (*XML_ElementDeclHandler) (void *userData,
                                        const XML_Char *name,
                                        XML_Content *model);

void
XML_SetElementDeclHandler(XML_Parser parser,
                          XML_ElementDeclHandler eldecl);

typedef void (*XML_AttlistDeclHandler) (
                                    void            *userData,
                                    const XML_Char  *elname,
                                    const XML_Char  *attname,
                                    const XML_Char  *att_type,
                                    const XML_Char  *dflt,
                                    int              isrequired);

void
XML_SetAttlistDeclHandler(XML_Parser parser,
                          XML_AttlistDeclHandler attdecl);

typedef void (*XML_XmlDeclHandler) (void *userData,
                                    const XML_Char *version,
                                    const XML_Char *encoding,
                                    int             standalone);

void
XML_SetXmlDeclHandler(XML_Parser parser,
                      XML_XmlDeclHandler xmldecl);


typedef struct {
  void *(*malloc_fcn)(size_t size);
  void *(*realloc_fcn)(void *ptr, size_t size);
  void (*free_fcn)(void *ptr);
} XML_Memory_Handling_Suite;

XML_Parser
XML_ParserCreate(const XML_Char *encoding);

XML_Parser
XML_ParserCreateNS(const XML_Char *encoding, XML_Char namespaceSeparator);


XML_Parser
XML_ParserCreate_MM(const XML_Char *encoding,
                    const XML_Memory_Handling_Suite *memsuite,
                    const XML_Char *namespaceSeparator);

XML_Bool
XML_ParserReset(XML_Parser parser, const XML_Char *encoding);

typedef void (*XML_StartElementHandler) (void *userData,
                                         const XML_Char *name,
                                         const XML_Char **atts);

typedef void (*XML_EndElementHandler) (void *userData,
                                       const XML_Char *name);


typedef void (*XML_CharacterDataHandler) (void *userData,
                                          const XML_Char *s,
                                          int len);

typedef void (*XML_ProcessingInstructionHandler) (
                                                void *userData,
                                                const XML_Char *target,
                                                const XML_Char *data);

typedef void (*XML_CommentHandler) (void *userData,
                                    const XML_Char *data);

typedef void (*XML_StartCdataSectionHandler) (void *userData);
typedef void (*XML_EndCdataSectionHandler) (void *userData);

typedef void (*XML_DefaultHandler) (void *userData,
                                    const XML_Char *s,
                                    int len);

typedef void (*XML_StartDoctypeDeclHandler) (
                                            void *userData,
                                            const XML_Char *doctypeName,
                                            const XML_Char *sysid,
                                            const XML_Char *pubid,
                                            int has_internal_subset);

typedef void (*XML_EndDoctypeDeclHandler)(void *userData);

typedef void (*XML_EntityDeclHandler) (
                              void *userData,
                              const XML_Char *entityName,
                              int is_parameter_entity,
                              const XML_Char *value,
                              int value_length,
                              const XML_Char *base,
                              const XML_Char *systemId,
                              const XML_Char *publicId,
                              const XML_Char *notationName);

void
XML_SetEntityDeclHandler(XML_Parser parser,
                         XML_EntityDeclHandler handler);

typedef void (*XML_UnparsedEntityDeclHandler) (
                                    void *userData,
                                    const XML_Char *entityName,
                                    const XML_Char *base,
                                    const XML_Char *systemId,
                                    const XML_Char *publicId,
                                    const XML_Char *notationName);

typedef void (*XML_NotationDeclHandler) (
                                    void *userData,
                                    const XML_Char *notationName,
                                    const XML_Char *base,
                                    const XML_Char *systemId,
                                    const XML_Char *publicId);

typedef void (*XML_StartNamespaceDeclHandler) (
                                    void *userData,
                                    const XML_Char *prefix,
                                    const XML_Char *uri);

typedef void (*XML_EndNamespaceDeclHandler) (
                                    void *userData,
                                    const XML_Char *prefix);

typedef int (*XML_NotStandaloneHandler) (void *userData);

typedef int (*XML_ExternalEntityRefHandler) (
                                    XML_Parser parser,
                                    const XML_Char *context,
                                    const XML_Char *base,
                                    const XML_Char *systemId,
                                    const XML_Char *publicId);

typedef void (*XML_SkippedEntityHandler) (
                                    void *userData,
                                    const XML_Char *entityName,
                                    int is_parameter_entity);

typedef struct {
  int map[256];
  void *data;
  int (*convert)(void *data, const char *s);
  void (*release)(void *data);
} XML_Encoding;

typedef int (*XML_UnknownEncodingHandler) (
                                    void *encodingHandlerData,
                                    const XML_Char *name,
                                    XML_Encoding *info);

void
XML_SetElementHandler(XML_Parser parser,
                      XML_StartElementHandler start,
                      XML_EndElementHandler end);

void
XML_SetStartElementHandler(XML_Parser parser,
                           XML_StartElementHandler handler);

void
XML_SetEndElementHandler(XML_Parser parser,
                         XML_EndElementHandler handler);

void
XML_SetCharacterDataHandler(XML_Parser parser,
                            XML_CharacterDataHandler handler);

void
XML_SetProcessingInstructionHandler(XML_Parser parser,
                                    XML_ProcessingInstructionHandler handler);
void
XML_SetCommentHandler(XML_Parser parser,
                      XML_CommentHandler handler);

void
XML_SetCdataSectionHandler(XML_Parser parser,
                           XML_StartCdataSectionHandler start,
                           XML_EndCdataSectionHandler end);

void
XML_SetStartCdataSectionHandler(XML_Parser parser,
                                XML_StartCdataSectionHandler start);

void
XML_SetEndCdataSectionHandler(XML_Parser parser,
                              XML_EndCdataSectionHandler end);

void
XML_SetDefaultHandler(XML_Parser parser,
                      XML_DefaultHandler handler);

void
XML_SetDefaultHandlerExpand(XML_Parser parser,
                            XML_DefaultHandler handler);

void
XML_SetDoctypeDeclHandler(XML_Parser parser,
                          XML_StartDoctypeDeclHandler start,
                          XML_EndDoctypeDeclHandler end);

void
XML_SetStartDoctypeDeclHandler(XML_Parser parser,
                               XML_StartDoctypeDeclHandler start);

void
XML_SetEndDoctypeDeclHandler(XML_Parser parser,
                             XML_EndDoctypeDeclHandler end);

void
XML_SetUnparsedEntityDeclHandler(XML_Parser parser,
                                 XML_UnparsedEntityDeclHandler handler);

void
XML_SetNotationDeclHandler(XML_Parser parser,
                           XML_NotationDeclHandler handler);

void
XML_SetNamespaceDeclHandler(XML_Parser parser,
                            XML_StartNamespaceDeclHandler start,
                            XML_EndNamespaceDeclHandler end);

void
XML_SetStartNamespaceDeclHandler(XML_Parser parser,
                                 XML_StartNamespaceDeclHandler start);

void
XML_SetEndNamespaceDeclHandler(XML_Parser parser,
                               XML_EndNamespaceDeclHandler end);

void
XML_SetNotStandaloneHandler(XML_Parser parser,
                            XML_NotStandaloneHandler handler);

void
XML_SetExternalEntityRefHandler(XML_Parser parser,
                                XML_ExternalEntityRefHandler handler);

void
XML_SetExternalEntityRefHandlerArg(XML_Parser parser,
                                   void *arg);

void
XML_SetSkippedEntityHandler(XML_Parser parser,
                            XML_SkippedEntityHandler handler);

void
XML_SetUnknownEncodingHandler(XML_Parser parser,
                              XML_UnknownEncodingHandler handler,
                              void *encodingHandlerData);

void
XML_DefaultCurrent(XML_Parser parser);

void
XML_SetReturnNSTriplet(XML_Parser parser, int do_nst);

void
XML_SetUserData(XML_Parser parser, void *userData);

#define XML_GetUserData(parser) (*(void **)(parser))

enum XML_Status
XML_SetEncoding(XML_Parser parser, const XML_Char *encoding);

void
XML_UseParserAsHandlerArg(XML_Parser parser);

enum XML_Error
XML_UseForeignDTD(XML_Parser parser, XML_Bool useDTD);


enum XML_Status
XML_SetBase(XML_Parser parser, const XML_Char *base);

const XML_Char *
XML_GetBase(XML_Parser parser);

int
XML_GetSpecifiedAttributeCount(XML_Parser parser);

int
XML_GetIdAttributeIndex(XML_Parser parser);

enum XML_Status
XML_Parse(XML_Parser parser, const char *s, int len, int isFinal);

void *
XML_GetBuffer(XML_Parser parser, int len);

enum XML_Status
XML_ParseBuffer(XML_Parser parser, int len, int isFinal);

enum XML_Status
XML_StopParser(XML_Parser parser, XML_Bool resumable);

enum XML_Status
XML_ResumeParser(XML_Parser parser);

enum XML_Parsing {
  XML_INITIALIZED,
  XML_PARSING,
  XML_FINISHED,
  XML_SUSPENDED
};

typedef struct {
  enum XML_Parsing parsing;
  XML_Bool finalBuffer;
} XML_ParsingStatus;

void
XML_GetParsingStatus(XML_Parser parser, XML_ParsingStatus *status);

XML_Parser
XML_ExternalEntityParserCreate(XML_Parser parser,
                               const XML_Char *context,
                               const XML_Char *encoding);

enum XML_ParamEntityParsing {
  XML_PARAM_ENTITY_PARSING_NEVER,
  XML_PARAM_ENTITY_PARSING_UNLESS_STANDALONE,
  XML_PARAM_ENTITY_PARSING_ALWAYS
};

int
XML_SetParamEntityParsing(XML_Parser parser,
                          enum XML_ParamEntityParsing parsing);

enum XML_Error
XML_GetErrorCode(XML_Parser parser);

int XML_GetCurrentLineNumber(XML_Parser parser);
int XML_GetCurrentColumnNumber(XML_Parser parser);
long XML_GetCurrentByteIndex(XML_Parser parser);

int
XML_GetCurrentByteCount(XML_Parser parser);

const char *
XML_GetInputContext(XML_Parser parser,
                    int *offset,
                    int *size);

#define XML_GetErrorLineNumber   XML_GetCurrentLineNumber
#define XML_GetErrorColumnNumber XML_GetCurrentColumnNumber
#define XML_GetErrorByteIndex    XML_GetCurrentByteIndex

void
XML_FreeContentModel(XML_Parser parser, XML_Content *model);

void *
XML_MemMalloc(XML_Parser parser, size_t size);

void *
XML_MemRealloc(XML_Parser parser, void *ptr, size_t size);

void
XML_MemFree(XML_Parser parser, void *ptr);

void
XML_ParserFree(XML_Parser parser);

const XML_LChar *
XML_ErrorString(enum XML_Error code);

const XML_LChar *
XML_ExpatVersion(void);

typedef struct {
  int major;
  int minor;
  int micro;
} XML_Expat_Version;

XML_Expat_Version 
XML_ExpatVersionInfo(void);

enum XML_FeatureEnum {
  XML_FEATURE_END = 0,
  XML_FEATURE_UNICODE,
  XML_FEATURE_UNICODE_WCHAR_T,
  XML_FEATURE_DTD,
  XML_FEATURE_CONTEXT_BYTES,
  XML_FEATURE_MIN_SIZE,
  XML_FEATURE_SIZEOF_XML_CHAR,
  XML_FEATURE_SIZEOF_XML_LCHAR,
  XML_FEATURE_NS,
  XML_FEATURE_LARGE_SIZE
};

typedef struct {
  enum XML_FeatureEnum  feature;
  const XML_LChar       *name;
  long int              value;
} XML_Feature;

const XML_Feature *
XML_GetFeatureList(void);


#define XML_MAJOR_VERSION 2
#define XML_MINOR_VERSION 0
#define XML_MICRO_VERSION 1


/****************************************************************************/

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack()
   #endif
#elif defined(__VBCC__)
   #pragma default-align
#endif

#ifdef __cplusplus
}
#endif

/****************************************************************************/

#endif  /* EXPAT_EXPAT_H */
