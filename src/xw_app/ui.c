#include "xw_app/ui.h"

#include <stdio.h>
#include <string.h>

bool XwAppUi_Init(XwAppUi* ui, const char* font, char* error, size_t error_capacity) {
	if (!ui || !font || !font[0]) {
		if (error && error_capacity)
			snprintf(error, error_capacity, "invalid application UI configuration");
		return false;
	}
	memset(ui, 0, sizeof *ui);
	AeronUiTheme theme;
	if (!AeronUi_ThemeLoadYaml(&theme, Aeron_GetVfs(), AERON_VFS_ROOT_RESOURCE, "ui-theme.yaml")) {
		if (error && error_capacity)
			snprintf(error, error_capacity, "Cannot load UI theme RESOURCE/ui-theme.yaml.");
		return false;
	}
	AeronCommandBuffer* command = Aeron_AcquireCommandBuffer();
	if (!command) {
		if (error && error_capacity)
			snprintf(error, error_capacity, "UI font upload command-buffer acquisition failed");
		return false;
	}
	if (!AeronFontAtlas_LoadVfs(&ui->font, command, Aeron_GetVfs(), AERON_VFS_ROOT_RESOURCE, font,
								64u * 1024u * 1024u)) {
		Aeron_CancelCommandBuffer(command);
		if (error && error_capacity)
			snprintf(error, error_capacity,
					 "required UI font atlas RESOURCE/%s.{fnt,png} is missing, unreadable, or invalid", font);
		return false;
	}
	if (!Aeron_SubmitCommandBuffer(command)) {
		AeronFontAtlas_Release(&ui->font);
		if (error && error_capacity)
			snprintf(error, error_capacity, "UI font atlas upload submission failed");
		return false;
	}
	ui->context = AeronUi_Create(NULL);
	if (!ui->context) {
		AeronFontAtlas_Release(&ui->font);
		if (error && error_capacity)
			snprintf(error, error_capacity, "application UI context creation failed");
		return false;
	}
	AeronUi_SetTheme(ui->context, &theme);
	AeronUi_SetFonts(ui->context, &(AeronUiFontSet) {
									  .regular = &ui->font,
									  .title = &ui->font,
								  });
	return true;
}

void XwAppUi_Shutdown(XwAppUi* ui) {
	if (!ui)
		return;
	AeronUi_Destroy(ui->context);
	AeronFontAtlas_Release(&ui->font);
	memset(ui, 0, sizeof *ui);
}

AeronUiContext* XwAppUi_Context(XwAppUi* ui) { return ui ? ui->context : NULL; }
