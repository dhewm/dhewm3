
#define DEBUG 1
#include <aros/debug.h>

#include <proto/exec.h>

struct Library *Sys_AROS_InitNetworking(void)
{
	struct Library *socketBase;
    D(bug("[ADoom3] %s()\n", __func__));
	socketBase = OpenLibrary("bsdsocket.library", 0);
	D(bug("[ADoom3] %s: returning 0x%p\n", __func__, socketBase));
	return socketBase;
}
