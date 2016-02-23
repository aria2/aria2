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

#ifdef __USE_INLINE__
#undef __USE_INLINE__
#endif

#include "expat_68k.h"
#include <exec/emulation.h>
#include <proto/exec.h>
#include <stdarg.h>

static uint32 VARARGS68K call_68k_code (struct ExecIFace *IExec, void *code, int num_args, ...) {
	uint32 res = 0;

	va_list vargs;
	va_startlinear(vargs, num_args);
	uint32 *args = va_getlinearva(vargs, uint32 *);

	uint8 *stack = IExec->AllocVec(4096, MEMF_SHARED);
	if (stack) {
		uint32 *sp = (uint32 *)(stack + 4096);
		args += num_args;
		while (num_args--) {
			*--sp = *--args;
		}

		res = IExec->EmulateTags(code, ET_StackPtr, sp, TAG_END);
		IExec->FreeVec(stack);
	}

	va_end(vargs);

	return res;
}

void _68k_startelementhandler(void *userdata, const char *name, const char **attrs) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->startelementhandler, 3, p->handlerarg, name, attrs);
}

void _68k_endelementhandler(void *userdata, const char *name) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->endelementhandler, 2, p->handlerarg, name);
}

void _68k_chardatahandler(void *userdata, const char *s, int len) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->chardatahandler, 3, p->handlerarg, s, len);
}

void _68k_procinsthandler(void *userdata, const char *target, const char *data) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->procinsthandler, 3, p->handlerarg, target, data);
}

void _68k_commenthandler(void *userdata, const char *data) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->commenthandler, 2, p->handlerarg, data);
}

void _68k_startcdatahandler(void *userdata) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->startcdatahandler, 1, p->handlerarg);
}

void _68k_endcdatahandler(void *userdata) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->endcdatahandler, 1, p->handlerarg);
}

void _68k_defaulthandler(void *userdata, const char *s, int len) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->defaulthandler, 3, p->handlerarg, s, len);
}

void _68k_defaulthandlerexp(void *userdata, const char *s, int len) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->defaulthandlerexp, 3, p->handlerarg, s, len);
}

int _68k_extentrefhandler(XML_Parser parser, const char *context, const char *base,
	const char *sysid, const char *pubid)
{
	M68kXML_Parser p = XML_GetUserData(parser);
	void *arg = p->extenthandlerarg;
	return (int)call_68k_code(p->IExec, p->extentrefhandler, 5, arg ? arg : p, context, base, sysid, pubid);
}

int _68k_unknownenchandler(void *enchandlerdata, const char *name, XML_Encoding *info) {
	M68kXML_Parser p = enchandlerdata;
	return (int)call_68k_code(p->IExec, p->unknownenchandler, 3, p->enchandlerarg, name, info);
}

void _68k_startnamespacehandler(void *userdata, const char *prefix, const char *uri) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->startnamespacehandler, 3, p->handlerarg, prefix, uri);
}

void _68k_endnamespacehandler(void *userdata, const char *prefix) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->endnamespacehandler, 2, p->handlerarg, prefix);
}

void _68k_xmldeclhandler(void *userdata, const char *version, const char *encoding, int standalone) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->xmldeclhandler, 4, p->handlerarg, version, encoding, standalone);
}

void _68k_startdoctypehandler(void *userdata, const char *doctypename,
	const char *sysid, const char *pubid, int has_internal_subset)
{
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->startdoctypehandler, 5, p->handlerarg, doctypename, sysid, pubid, has_internal_subset);
}

void _68k_enddoctypehandler(void *userdata) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->enddoctypehandler, 1, p->handlerarg);
}

void _68k_elementdeclhandler(void *userdata, const char *name, XML_Content *model) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->elementdeclhandler, 3, p->handlerarg, name, model);
}

void _68k_attlistdeclhandler(void *userdata, const char *elname, const char *attname,
	const char *att_type, const char *dflt, int isrequired)
{
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->attlistdeclhandler, 6, p->handlerarg, elname, attname, att_type, dflt, isrequired);
}

void _68k_entitydeclhandler(void *userdata, const char *entityname, int is_param_entity,
	const char *value, int value_length, const char *base, const char *sysid, const char *pubid,
	const char *notationname)
{
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->entitydeclhandler, 9, p->handlerarg, entityname, is_param_entity,
		value, value_length, base, sysid, pubid, notationname);
}

void _68k_unparseddeclhandler(void *userdata, const char *entityname, const char *base,
	const char *sysid, const char *pubid, const char *notationname)
{
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->unparseddeclhandler, 6, p->handlerarg, entityname, base, sysid, pubid, notationname);
}

void _68k_notationdeclhandler(void *userdata, const char *notationname, const char *base,
	const char *sysid, const char *pubid)
{
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->notationdeclhandler, 5, p->handlerarg, notationname, base, sysid, pubid);
}

int _68k_notstandalonehandler(void *userdata) {
	M68kXML_Parser p = userdata;
	return (int)call_68k_code(p->IExec, p->notstandalonehandler, 1, p->handlerarg);
}

void _68k_skippedentityhandler(void *userdata, const char *entityname, int is_param_entity) {
	M68kXML_Parser p = userdata;
	call_68k_code(p->IExec, p->skippedentityhandler, 3, p->handlerarg, entityname, is_param_entity);
}
