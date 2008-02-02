
#if @HAVE_VISIBILITY@ && BUILDING_LIBINTL
#define LIBINTL_DLL_EXPORTED __attribute__((__visibility__("default")))
#else
#define LIBINTL_DLL_EXPORTED
#endif
