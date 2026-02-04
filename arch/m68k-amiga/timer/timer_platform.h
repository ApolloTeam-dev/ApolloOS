#include <graphics/gfxbase.h>
#include <aros/debug.h>

void InitCustom(struct GfxBase *gfx);

#if APOLLO_DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

#if DEBUG
#define bug(x,args...)      kprintf(x ,##args)
#define debug(x,args...)    bug("[SAGASD] %s:%ld " x "\n", __func__, (unsigned long)__LINE__ ,##args)
#define DD(x) x
#else
#define DD(x)
#endif