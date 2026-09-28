#ifndef XW_APP_SETUP_UI_H
#define XW_APP_SETUP_UI_H
#include "xw_app/setup.h"
XwSetupResult XwSetupUi_Recover(XwAppUi* ui, char* error, size_t capacity);
XwSetupResult XwSetupUi_Run(XwAppUi* ui, XwInstallationSet* installations, bool override93, bool override94,
							bool override98, char* error, size_t capacity);
#endif
