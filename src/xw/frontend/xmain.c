#include "xw/frontend/xmain.h"

// FUNCTION: XW 0x47DF80
XwShellSceneResult xmain_main(XwShellSceneId scene, struct XwLegacyMemoryConfig* memory) {
#ifdef XW_MODERN
	shell_Shell(scene, memory);
#else
	return shell_Shell(scene, memory);
#endif
}
