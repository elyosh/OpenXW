#include "xw_dos94/frontend/dispatch.h"
#include "xw/audio/frontend_audio.h"
#include "xw/flight/fediskio.h"
#include "xw/flight/flight.h"
#include "xw/flight/flight_display.h"
#include "xw/frontend/scenes/assault.h"
#include "xw/frontend/scenes/bintro.h"
#include "xw/frontend/scenes/boom.h"
#include "xw/frontend/scenes/dfire.h"
#include "xw/frontend/scenes/dock.h"
#include "xw/frontend/scenes/ds_boom.h"
#include "xw/frontend/scenes/ds_done.h"
#include "xw/frontend/scenes/ds_land.h"
#include "xw/frontend/scenes/gov1.h"
#include "xw/frontend/scenes/hoth.h"
#include "xw/frontend/scenes/leave1.h"
#include "xw/frontend/scenes/leave2.h"
#include "xw/frontend/scenes/outpost.h"
#include "xw/frontend/scenes/plans.h"
#include "xw/frontend/scenes/probe.h"
#include "xw/frontend/scenes/sabotage.h"
#include "xw/frontend/scenes/tarkin.h"
#include "xw/frontend/scenes/yavin1.h"
#include "xw/frontend/scenes/yavin2.h"
#include "xw/frontend/scenes/yavin3.h"
#include "xw/frontend/shell_test.h"
#include "xw/frontend/shellext.h"
#include "xw_dos94/frontend/awards.h"
#include "xw_dos94/frontend/brief.h"
#include "xw_dos94/frontend/ceremony.h"
#include "xw_dos94/frontend/combat.h"
#include "xw_dos94/frontend/credits.h"
#include "xw_dos94/frontend/debrief.h"
#include "xw_dos94/frontend/filmview.h"
#include "xw_dos94/frontend/hangar3.h"
#include "xw_dos94/frontend/inflight.h"
#include "xw_dos94/frontend/intro.h"
#include "xw_dos94/frontend/pilot_log.h"
#include "xw_dos94/frontend/recovery_scenes.h"
#include "xw_dos94/frontend/register.h"
#include "xw_dos94/frontend/scenes.h"
#include "xw_dos94/frontend/tourdesk.h"
#include "xw_dos94/frontend/train.h"
#include "xw_dos94/frontend/training_transit.h"

typedef void (*ScenePlayer)(XwShellContext*);

/* These retained VGA renderers have the same DOS film roles, geometry and exits. */
static ScenePlayer shared_scene(int scene) {
	switch (scene) {
		case 5:
			return ShellTest_Show;
		case 140:
		case 142:
			return Sabotage_Sabotage;
		case 141:
			return Sabotage_MiddleScene;
		case 150:
		case 151:
		case 152:
		case 153:
		case 154:
		case 163:
		case 165:
		case 166:
		case 167:
		case 168:
			return DFire_Play;
		case 160:
			return DsDone_Play;
		case 162:
			return DsLand_Play;
		case 164:
			return Tarkin_Play;
		case 170:
			return DsBoom_Play;
		case 171:
			return Yavin1_Play;
		case 172:
			return Yavin2_Play;
		case 173:
			return Yavin3_Play;
		case 180:
		case 181:
		case 182:
		case 183:
		case 184:
		case 185:
			return Outpost_Play;
		case 186:
		case 187:
		case 189:
			return Plans_Play;
		case 190:
		case 191:
		case 192:
			return Assault_Play;
		case 400:
		case 401:
		case 402:
			return Dock_Play;
		case 410:
			return Leave1_Play;
		case 411:
			return Leave2_Play;
		case 420:
			return Gov1_Play;
		case 430:
		case 431:
		case 432:
			return Boom_Play;
		case 500:
		case 501:
		case 502:
			return Hoth_Play;
		case 510:
		case 511:
			return BIntro_BWingArrival;
		case 520:
		case 521:
		case 522:
			return Probe_Play;
		default:
			return NULL;
	}
}

/* DOS94 shell_Main dispatch table at 0x100086. */
static ScenePlayer dos_scene(XwShellSceneId scene) {
	ScenePlayer play = NULL;
	switch (scene) {
		case 6:
			play = Dos94_Logo640_Play;
			break;
		case 7:
		case 8:
			play = Dos94_title_Title;
			break;
		case 10:
			play = Dos94_Intro1_Opening;
			break;
		case 11:
			play = Dos94_Intro2_Attack;
			break;
		case 13:
			play = Dos94_Brdg1_Play;
			break;
		case 14:
			play = Dos94_Brdg2_Play;
			break;
		case 15:
			play = Dos94_Hangar1;
			break;
		case 16:
		case 260:
		case 261:
		case 262:
		case 266:
		case 270:
		case 271:
		case 272:
		case 276:
		case 281:
		case 283:
		case 284:
		case 285:
		case 320:
		case 321:
		case 322:
		case 323:
		case 330:
		case 331:
		case 332:
		case 333:
		case 340:
		case 341:
		case 342:
		case 343:
			play = Dos94_Hangar2;
			break;
		case 17:
		case 83:
		case 250:
		case 251:
		case 252:
		case 253:
		case 282:
		case 286:
		case 287:
		case 288:
			play = Dos94_Hangar3;
			break;
		case 20:
		case 21:
		case 22:
		case 23:
		case 27:
		case 310:
		case 311:
		case 312:
		case 313:
			play = Dos94_Battle;
			break;
		case 24:
			play = Dos94_XLogo_XLogo;
			break;
		case 25:
		case 29:
			play = Dos94_credits_Credits;
			break;
		case 26:
		case 28:
			play = Dos94_Register;
			break;
		case 30:
			play = Dos94_MainMenu;
			break;
		case 40:
		case 41:
		case 42:
		case 50:
		case 51:
		case 52:
		case 80:
		case 81:
			play = Dos94_TrainingTransit_PlayDeparture;
			break;
		case 43:
		case 53:
		case 263:
		case 264:
		case 265:
		case 267:
		case 273:
		case 274:
		case 275:
		case 277:
		case 290:
		case 291:
		case 292:
		case 296:
		case 300:
		case 301:
		case 302:
		case 306:
			play = Dos94_TrainingTransit_PlayFacilityFlight;
			break;
		case 45:
			play = Dos94_Train;
			break;
		case 60:
			play = Dos94_combat_Combat;
			break;
		case 70:
			play = Dos94_tourdesk_TourDesk;
			break;
		case 82:
		case 85:
		case 86:
		case 188:
			play = Dos94_Flet640_Play;
			break;
		case 90:
			play = Dos94_filmview_FilmView;
			break;
		case 95:
			play = Dos94_Blueprint;
			break;
		case 110:
		case 111:
		case 112:
			play = Dos94_Brief;
			break;
		case 115:
		case 116:
		case 117:
			play = Dos94_debrief_Debrief;
			break;
		case 120:
		case 122:
			play = Dos94_PilotSelection;
			break;
		case 125:
		case 126:
		case 130:
			play = Dos94_Uniform_Uniform;
			break;
		case 127:
		case 128:
		case 131:
			play = Dos94_PilotLog_Show;
			break;
		case 135:
		case 136:
		case 137:
			play = Dos94_AwardBox_AwardBox;
			break;
		case 161:
			play = Dos94_DsBay;
			break;
		case 220:
		case 221:
			play = Dos94_Rescue64_Play;
			break;
		case 222:
			play = Dos94_Tort640_Play;
			break;
		case 225:
			play = Dos94_Med640_Play;
			break;
		case 230:
		case 231:
			play = Dos94_Death640_Play;
			break;
		case 240:
		case 241:
		case 242:
		case 245:
		case 246:
		case 247:
			play = Dos94_Awards640_Play;
			break;
		case 255:
		case 256:
		case 257:
		case 258:
		case 293:
		case 294:
		case 295:
		case 297:
		case 303:
		case 304:
		case 305:
		case 307:
			play = Dos94_Landing;
			break;
		case 360:
		case 361:
		case 362:
			play = Dos94_InflightUI_Show;
			break;
		case 421:
		case 422:
		case 423:
		case 424:
		case 425:
			play = Dos94_Gov2_Play;
			break;
		default:
			play = shared_scene(scene);
			break;
	}
	return play;
}

int Dos94_OpenScene(XwShellSceneId scene, XwShellContext* shell) {
	ScenePlayer play = dos_scene(scene);
	if (!play)
		return 0;
	FlightDisplay_ClearBackAndAuxiliarySurfaces();
	FlightDisplay_Flip();
	switch (scene) {
		case 140:
			FrontendAudio_PlayFile("XwingCD\\music\\tour1s2.wav", 0);
			break;
		case 150:
			FrontendAudio_PlayFile("XwingCD\\music\\tour3s2.wav", 0);
			break;
		case 160:
			FrontendAudio_PlayFile("XwingCD\\music\\tour3s1.wav", 0);
			break;
		case 170:
			FrontendAudio_PlayFile("XwingCD\\music\\tour3s3.wav", 0);
			break;
		case 180:
			FrontendAudio_PlayFile("XwingCD\\music\\tour2s1.wav", 0);
			break;
		case 186:
			FrontendAudio_PlayFile("XwingCD\\music\\tour2s2.wav", 0);
			break;
		case 190:
			FrontendAudio_PlayFile("XwingCD\\music\\tour1s1.wav", 0);
			break;
		case 400:
			FrontendAudio_PlayFile("XwingCD\\music\\tour4s3.wav", 0);
			break;
		case 410:
			FrontendAudio_PlayFile("XwingCD\\music\\tour4s1.wav", 0);
			break;
		case 420:
			FrontendAudio_PlayFile("XwingCD\\music\\tour4s2.wav", 0);
			break;
		case 430:
			FrontendAudio_PlayFile("XwingCD\\music\\tour4s4.wav", 0);
			break;
		case 500:
			FrontendAudio_PlayFile("XwingCD\\music\\tour5s3.wav", 0);
			break;
		case 510:
			FrontendAudio_PlayFile("XwingCD\\music\\tour5s1.wav", 0);
			break;
		case 520:
			FrontendAudio_PlayFile("XwingCD\\music\\tour5s2.wav", 0);
			break;
		case 90:
			g_flightLoadingReplayFilm = 1;
			break;
		case 220:
			FrontendAudio_PlayFile("XwingCD\\music\\rescue.wav", 0);
			break;
		case 221:
			FrontendAudio_PlayFile("XwingCD\\music\\torture.wav", 0);
			break;
		case 230:
			FrontendAudio_PlayFile("XwingCD\\music\\death.wav", 0);
			break;
		case 240:
			FrontendAudio_PlayFile("XwingCD\\music\\recruits.wav", 0);
			break;
	}
	play(shell);
	return 1;
}

/* DOS94 0x101412, source/destination table at 0x116d7a. */
int16_t Dos94_ConvertTransition(int16_t scene, int16_t sudden) {
	static const int16_t redirects[][2] = {
		{ 40, 45 },   { 43, 30 },   { 50, 60 },   { 53, 30 },   { 80, 112 },  { 250, 117 },
		{ 251, 117 }, { 252, 117 }, { 253, 117 }, { 260, 2 },   { 261, 2 },   { 262, 2 },
		{ 266, 2 },   { 270, 3 },   { 271, 3 },   { 272, 3 },   { 276, 3 },   { 283, 4 },
		{ 284, 4 },   { 285, 4 },   { 281, 4 },   { 290, 115 }, { 291, 115 }, { 292, 115 },
		{ 296, 115 }, { 300, 116 }, { 301, 116 }, { 302, 116 }, { 306, 116 }, { 83, 30 }
	};
	for (size_t i = 0; i < sizeof redirects / sizeof redirects[0]; ++i) {
		if (redirects[i][0] != scene)
			continue;
		if (sudden)
			shellext_Sudden_Scene_End();
		return redirects[i][1];
	}
	return scene;
}
