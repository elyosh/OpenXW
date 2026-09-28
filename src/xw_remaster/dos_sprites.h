#ifndef XW_REMASTER_DOS_SPRITES_H
#define XW_REMASTER_DOS_SPRITES_H
#include "xw_remaster/dos_ship.h"
#include <aeron/scene/billboard.h>
#include <aeron/scene/runtime_atlas.h>
bool XwDosSprites_Begin(AeronSampleCount samples);
/* Project a shared sky quad and queue its palette-filtered DOS draw. */
bool XwDosSprites_Backdrop(const XwRenderView* view, const AeronRuntimeAtlasPage* page,
						   const AeronSpriteRect* rect, const XwRenderSource* remap,
						   const AeronSceneBillboardDesc* backdrop);
bool XwDosSprites_Effects(const XwRenderSnapshot* s, const XwRenderView* view,
						  const XwDosSelection* selection);
bool XwDosSprites_Upload(AeronCommandBuffer* cmd);
void XwDosSprites_Draw(AeronRenderPass* pass, bool sky);
void XwDosSprites_Shutdown(void);
#endif
