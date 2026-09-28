#ifndef XW_APP_SETUP_H
#define XW_APP_SETUP_H
#include "xw_app/host_config.h"
#include "xw_app/installation.h"
#include "xw_app/ui.h"
#include "xw_runtime/config/settings.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef enum XwSetupResult {
	XW_SETUP_ERROR,
	XW_SETUP_SUCCESS,
	XW_SETUP_CANCELLED,
} XwSetupResult;

/* A NULL UI keeps installation checks windowless. Otherwise setup initializes
 * the application-owned UI for use throughout the rest of the session. */
XwSetupResult XwSetup_Run(const XwLaunchOptions* options, XwAppUi* ui, char* error, size_t capacity);
/* Installation VFS handles stay owned by setup until all runtime consumers stop. */
void XwSetup_Shutdown(void);
const char* XwSetup_InstallationFor(XwGameVersion version);
int XwSetup_CdMusicAvailable(void);
void XwSetup_SelectDefaults(const XwInstallationSet* installed, XwSettings* settings, bool only_unset);
bool XwSetup_ValidateSelection(const XwInstallationSet* installed, const XwSettings* settings, char* error,
							   size_t capacity);
bool XwSetup_CommitSettings(const XwSettings* settings, int save, char* error, size_t capacity);
#ifdef __cplusplus
}
#endif

#endif
