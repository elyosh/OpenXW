#ifndef XW_FRONTEND_SHELL_TEST_H
#define XW_FRONTEND_SHELL_TEST_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>
#include <xw/frontend/shell.h>

enum {
	SHELL_TEST_CHOICE_EXIT = 0,
	SHELL_TEST_CHOICE_MISSION = 1,
	SHELL_TEST_CHOICE_EMPIRE_ASSAULT = 2,
	SHELL_TEST_CHOICE_DESTROY_STAR_DESTROYER = 3,
	SHELL_TEST_CHOICE_SEND_PLANS = 4,
	SHELL_TEST_CHOICE_RECOVER_PLANS = 5,
	SHELL_TEST_CHOICE_DEATH_STAR_COMPLETED = 6,
	SHELL_TEST_CHOICE_DEATH_STAR_FIRE = 7,
	SHELL_TEST_CHOICE_DEATH_STAR_DESTRUCTION = 8,
	SHELL_TEST_SCENE_LABEL_BYTES = 127
};

enum {
	SHELL_TEST_OUTCOME_EXIT = 0,
	SHELL_TEST_OUTCOME_LAND = 1,
	SHELL_TEST_OUTCOME_RESCUE = 2,
	SHELL_TEST_OUTCOME_CAPTURE = 3,
	SHELL_TEST_OUTCOME_DEATH = 4,
	SHELL_TEST_OUTCOME_LABEL_BYTES = 27
};

extern char g_shellTestOutcomeLabels[SHELL_TEST_OUTCOME_LABEL_BYTES];
extern char g_shellTestSceneLabels[SHELL_TEST_SCENE_LABEL_BYTES];

struct XwShellContext;
/* Declarations follow ascending original IDB address. */

/* 0x47DFA0 */
XwShellSceneResult ShellTest_Show(struct XwShellContext* shell);

/* 0x47E010 */
void ShellTest_UpdateProvingGroundsExit(int unusedTime);

/* 0x47E0A0 */
void ShellTest_UpdateCombatExit(int unusedTime);

/* 0x47E130 */
void ShellTest_UpdateMissionOutcome(int unusedTime);

/* 0x47E2E0 */
void ShellTest_UpdateSceneSelection(int unusedTime);

#ifdef __cplusplus
}
#endif

#endif
