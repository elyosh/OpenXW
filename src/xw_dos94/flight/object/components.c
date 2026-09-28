#include "xw_dos94/assets/models.h"
#include <string.h>

/* Native turret identity shared by creation, damage and gun rotation. */
uint16_t Dos94_corvetteguncomponent(bool second) {
	return Dos94Assets_Version() == XW_GAME_VERSION_93 ? (second ? 6 : 5) : (second ? 2 : 0);
}

/* DOS94 0x68FE49, create_createcraft: original component ordinals, not OPT meshes. */
void Dos94_create_initcomponents(CraftData* craft, uint16_t objectType) {
	memset(craft->componentState, 0, sizeof craft->componentState);
	memset(craft->meshRotation, 0, sizeof craft->meshRotation);
	memset(craft->componentHp, 0, sizeof craft->componentHp);
	memset(craft->componentHp, 0xff, 16);
	if (objectType == 16) {
		craft->componentHp[1] = 90;
		craft->componentHp[2] = 90;
	} else if (objectType == 15) {
		craft->componentHp[Dos94_corvetteguncomponent(false)] = 80;
		craft->componentHp[Dos94_corvetteguncomponent(true)] = 80;
	}
}

/* DOS94 0x68E77E, create_createhyperin. */
void Dos94_create_initclosedfoils(CraftData* craft, uint16_t objectType) {
	if (objectType == 1) {
		static const uint8_t rotations[4] = { 0xf8, 0x0c, 0x08, 0xf4 };
		memcpy(craft->meshRotation + 1, rotations, sizeof rotations);
		craft->sFoilState = XW_SFOIL_CLOSED;
	} else if (objectType == 118) {
		memset(craft->meshRotation + 1, 64, 5);
		craft->sFoilState = XW_SFOIL_CLOSED;
	}
}
