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

#define __NOLIBBASE__
#define __NOGLOBALIFACE__

#include <dos/dos.h>
#include <proto/exec.h>

#include "expat_base.h"


#define LIBNAME		"expat.library"
#define LIBPRI		0
#define VERSION		53
#define REVISION	1
#define VSTRING		"expat.library 53.1 (7.8.2009)"  /* dd.mm.yyyy */


static const char* __attribute__((used)) verstag = "\0$VER: " VSTRING;


struct Interface *INewlib = 0;


struct ExpatBase * libInit(struct ExpatBase *libBase, BPTR seglist, struct ExecIFace *ISys);
uint32 libObtain (struct LibraryManagerInterface *Self);
uint32 libRelease (struct LibraryManagerInterface *Self);
struct ExpatBase *libOpen (struct LibraryManagerInterface *Self, uint32 version);
BPTR libClose (struct LibraryManagerInterface *Self);
BPTR libExpunge (struct LibraryManagerInterface *Self);
struct Interface *openInterface(struct ExecIFace *IExec, CONST_STRPTR libName, uint32 libVer);
void closeInterface(struct ExecIFace *IExec, struct Interface *iface);


static APTR lib_manager_vectors[] = {
	libObtain,
	libRelease,
	NULL,
	NULL,
	libOpen,
	libClose,
	libExpunge,
	NULL,
	(APTR)-1,
};


static struct TagItem lib_managerTags[] = {
	{ MIT_Name, (uint32)"__library" },
	{ MIT_VectorTable, (uint32)lib_manager_vectors },
	{ MIT_Version, 1 },
	{ TAG_END, 0 }
};


extern void *main_vectors[];

static struct TagItem lib_mainTags[] = {
	{ MIT_Name, (uint32)"main" },
	{ MIT_VectorTable, (uint32)main_vectors },
	{ MIT_Version, 1 },
	{ TAG_END, 0 }
};


static APTR libInterfaces[] = {
	lib_managerTags,
	lib_mainTags,
	NULL
};


extern void *VecTable68K[];

static struct TagItem libCreateTags[] = {
	{ CLT_DataSize, sizeof(struct ExpatBase) },
	{ CLT_InitFunc, (uint32)libInit },
	{ CLT_Interfaces, (uint32)libInterfaces },
	{ CLT_Vector68K, (uint32)VecTable68K },
	{ TAG_END, 0 }
};


static struct Resident __attribute__((used)) lib_res = {
	RTC_MATCHWORD,	// rt_MatchWord
	&lib_res,		// rt_MatchTag
	&lib_res+1,		// rt_EndSkip
	RTF_NATIVE | RTF_AUTOINIT,	// rt_Flags
	VERSION,		// rt_Version
	NT_LIBRARY,		// rt_Type
	LIBPRI,			// rt_Pri
	LIBNAME,		// rt_Name
	VSTRING,		// rt_IdString
	libCreateTags	// rt_Init
};


int32 _start()
{
	return RETURN_FAIL;
}


struct ExpatBase *libInit(struct ExpatBase *libBase, BPTR seglist, struct ExecIFace *iexec)
{
	libBase->libNode.lib_Node.ln_Type = NT_LIBRARY;
	libBase->libNode.lib_Node.ln_Pri = LIBPRI;
	libBase->libNode.lib_Node.ln_Name = LIBNAME;
	libBase->libNode.lib_Flags = LIBF_SUMUSED|LIBF_CHANGED;
	libBase->libNode.lib_Version = VERSION;
	libBase->libNode.lib_Revision = REVISION;
	libBase->libNode.lib_IdString = VSTRING;

	libBase->SegList = seglist;

	libBase->IExec = iexec;
	INewlib        = openInterface(iexec, "newlib.library", 0);

	if ( INewlib != 0 )  {
		return libBase;
	}

	closeInterface(iexec, INewlib);
	INewlib = 0;

	iexec->DeleteLibrary(&libBase->libNode);

	return NULL;
}


uint32 libObtain( struct LibraryManagerInterface *Self )
{
	++Self->Data.RefCount;
	return Self->Data.RefCount;
}


uint32 libRelease( struct LibraryManagerInterface *Self )
{
	--Self->Data.RefCount;
	return Self->Data.RefCount;
}


struct ExpatBase *libOpen( struct LibraryManagerInterface *Self, uint32 version )
{
	struct ExpatBase *libBase;

	libBase = (struct ExpatBase *)Self->Data.LibBase;

	++libBase->libNode.lib_OpenCnt;
	libBase->libNode.lib_Flags &= ~LIBF_DELEXP;

	return libBase;
}


BPTR libClose( struct LibraryManagerInterface *Self )
{
	struct ExpatBase *libBase;

	libBase = (struct ExpatBase *)Self->Data.LibBase;

	--libBase->libNode.lib_OpenCnt;
	if ( libBase->libNode.lib_OpenCnt ) {
		return 0;
	}

	if ( libBase->libNode.lib_Flags & LIBF_DELEXP ) {
		return (BPTR)Self->LibExpunge();
	}
	else {
		return ZERO;
	}
}


BPTR libExpunge( struct LibraryManagerInterface *Self )
{
	struct ExpatBase *libBase = (struct ExpatBase *)Self->Data.LibBase;
	BPTR result = ZERO;

	if (libBase->libNode.lib_OpenCnt == 0) {
		libBase->IExec->Remove(&libBase->libNode.lib_Node);

		result = libBase->SegList;

		closeInterface(libBase->IExec, INewlib);
		INewlib = 0;

		libBase->IExec->DeleteLibrary(&libBase->libNode);
	}
	else {
		libBase->libNode.lib_Flags |= LIBF_DELEXP;
	}

	return result;
}


struct Interface *openInterface(struct ExecIFace *IExec, CONST_STRPTR libName, uint32 libVer)
{
	struct Library *base = IExec->OpenLibrary(libName, libVer);
	struct Interface *iface = IExec->GetInterface(base, "main", 1, 0);
	if (iface == 0) {
		IExec->CloseLibrary(base);
	}

	return iface;
}


void closeInterface(struct ExecIFace *IExec, struct Interface *iface)
{
	if (iface != 0)
	{
		struct Library *base = iface->Data.LibBase;
		IExec->DropInterface(iface);
		IExec->CloseLibrary(base);
	}
}
