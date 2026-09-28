#include "xw/util/shared.h"

// GLOBAL: XW 0x4FA100
char g_sharedEmptyString[XW_SHARED_EMPTY_STRING_CAPACITY] = { 0 };

// FUNCTION: XW 0x46CBD0
void j_nullsub_2(void) { nullsub_SharedNoOp(); }

// FUNCTION: XW 0x485100
int Shared_ReturnZero32(void) { return 0; }

// FUNCTION: XW 0x49E560
int16_t Shared_ReturnZero(void) { return 0; }

// FUNCTION: XW 0x49E570
int16_t Shared_ReturnOne(void) { return 1; }

// FUNCTION: XW 0x49E5C0
void nullsub_SharedNoOp(void) {}
