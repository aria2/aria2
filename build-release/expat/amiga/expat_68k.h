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

#ifndef EXPAT_68K_H
#define EXPAT_68K_H

#ifndef LIBRARIES_EXPAT_H
#include <libraries/expat.h>
#endif

typedef struct M68kXML_ParserStruct {
	XML_Parser p;
	struct ExecIFace *IExec;
	void *handlerarg;
	void *extenthandlerarg;
	void *enchandlerarg;
	void *startelementhandler;
	void *endelementhandler;
	void *chardatahandler;
	void *procinsthandler;
	void *commenthandler;
	void *startcdatahandler;
	void *endcdatahandler;
	void *defaulthandler;
	void *defaulthandlerexp;
	void *extentrefhandler;
	void *unknownenchandler;
	void *startnamespacehandler;
	void *endnamespacehandler;
	void *xmldeclhandler;
	void *startdoctypehandler;
	void *enddoctypehandler;
	void *elementdeclhandler;
	void *attlistdeclhandler;
	void *entitydeclhandler;
	void *unparseddeclhandler;
	void *notationdeclhandler;
	void *notstandalonehandler;
	void *skippedentityhandler;
} *M68kXML_Parser;

/* expat_68k_handler_stubs.c */
void _68k_startelementhandler(void *userdata, const char *name, const char **attrs);
void _68k_endelementhandler(void *userdata, const char *name);
void _68k_chardatahandler(void *userdata, const char *s, int len);
void _68k_procinsthandler(void *userdata, const char *target, const char *data);
void _68k_commenthandler(void *userdata, const char *data);
void _68k_startcdatahandler(void *userdata);
void _68k_endcdatahandler(void *userdata);
void _68k_defaulthandler(void *userdata, const char *s, int len);
void _68k_defaulthandlerexp(void *userdata, const char *s, int len);
int _68k_extentrefhandler(XML_Parser parser, const char *context, const char *base,
	const char *sysid, const char *pubid);
int _68k_unknownenchandler(void *enchandlerdata, const char *name, XML_Encoding *info);
void _68k_startnamespacehandler(void *userdata, const char *prefix, const char *uri);
void _68k_endnamespacehandler(void *userdata, const char *prefix);
void _68k_xmldeclhandler(void *userdata, const char *version, const char *encoding, int standalone);
void _68k_startdoctypehandler(void *userdata, const char *doctypename,
	const char *sysid, const char *pubid, int has_internal_subset);
void _68k_enddoctypehandler(void *userdata);
void _68k_elementdeclhandler(void *userdata, const char *name, XML_Content *model);
void _68k_attlistdeclhandler(void *userdata, const char *elname, const char *attname,
	const char *att_type, const char *dflt, int isrequired);
void _68k_entitydeclhandler(void *userdata, const char *entityname, int is_param_entity,
	const char *value, int value_length, const char *base, const char *sysid, const char *pubid,
	const char *notationname);
void _68k_unparseddeclhandler(void *userdata, const char *entityname, const char *base,
	const char *sysid, const char *pubid, const char *notationname);
void _68k_notationdeclhandler(void *userdata, const char *notationname, const char *base,
	const char *sysid, const char *pubid);
int _68k_notstandalonehandler(void *userdata);
void _68k_skippedentityhandler(void *userdata, const char *entityname, int is_param_entity);

#endif
