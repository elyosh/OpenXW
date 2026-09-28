/* DOS93 B-Wing resident catalogue; original contracts are in DOS93_BWING.EXE.i64. */
#include "xw_dos94/assets/shared_model_tables.h"

/* DOS93 0x4d2ba. */
static const uint16_t Dos93_xWingComponent1States[] = { 1, 65280, 5, 65282, 65534, 65284 };

/* DOS93 0x4d2c6. */
static const uint16_t Dos93_xWingComponent2States[] = { 2, 65280, 6, 65282, 65534, 65284 };

/* DOS93 0x4d2d2. */
static const uint16_t Dos93_xWingComponent3States[] = { 3, 65280, 7, 65282, 65534, 65284 };

/* DOS93 0x4d2de. */
static const uint16_t Dos93_xWingComponent4States[] = { 4, 65280, 8, 65282, 65534, 65284 };

/* DOS93 0x4d524: native references replace the resident near pointers. */
static const Dos94Component Dos93_xWingComponents[] = {
	{ Dos94_sharedPolygonComponent0States, 4, 5, 1, 256, 0, 0, 0 },
	{ Dos93_xWingComponent1States, 6, 0, 0, 256, 0, 0, 0 },
	{ Dos93_xWingComponent2States, 6, 0, 0, 256, 0, 0, 0 },
	{ Dos93_xWingComponent3States, 6, 0, 0, 256, 0, 0, 0 },
	{ Dos93_xWingComponent4States, 6, 0, 0, 256, 0, 0, 0 },
	{ Dos94_craftHitBitmapStates, 25, 0, 0, 256, 0, 0, 0 },
};

/* DOS93 0x4D5C0, 0x4D608 and 0x4D638: identical three-part craft descriptors. */
static const Dos94Component Dos93_sharedThreePartCraftComponents[] = {
	{ Dos94_sharedPolygonComponent0States, 4, 3, 1, 256, 0, 0, 0 },
	{ Dos93_xWingComponent1States, 6, 0, 0, 256, 0, 0, 0 },
	{ Dos93_xWingComponent2States, 6, 0, 0, 256, 0, 0, 0 },
	{ Dos94_craftHitBitmapStates, 25, 0, 0, 256, 0, 0, 0 },
};

/* DOS93 0x4d668: native references replace the resident near pointers. */
static const Dos94Component Dos93_shuttleComponents[] = {
	{ Dos94_sharedPolygonComponent0States, 4, 4, 1, 256, 0, 0, 0 },
	{ Dos93_xWingComponent1States, 6, 0, 0, 256, 0, 0, 0 },
	{ Dos93_xWingComponent2States, 6, 0, 0, 256, 0, 0, 0 },
	{ Dos93_xWingComponent3States, 6, 0, 0, 256, 0, 0, 0 },
	{ Dos94_craftHitBitmapStates, 25, 0, 0, 256, 0, 0, 0 },
};

/* DOS93 0x4d488: native references replace the resident near pointers. */
static const Dos94Component Dos93_corvetteComponents[] = {
	{ Dos94_corvetteComponent0States, 4, 0, 0, 256, 0, 0, 0 },
	{ Dos94_corvetteComponent1States, 4, 0, 0, 256, 0, 0, 0 },
	{ Dos94_corvetteComponent2States, 4, 0, 0, 256, 0, 0, 0 },
	{ Dos94_corvetteComponent3States, 4, 0, 0, 256, 0, 0, 0 },
	{ Dos94_corvetteComponent4States, 4, 0, 0, 256, 0, 0, 0 },
	{ Dos94_corvetteComponent5States, 4, 0, 0, 256, 0, 0, 0 },
	{ Dos94_corvetteComponent6States, 4, 0, 0, 256, 0, 0, 0 },
};

/* DOS93 0x6aa755: descriptor counts are independent of file component counts. */
const Dos94ModelMetadata Dos93_modelTemplates[DOS94_MODEL_COUNT] = {
	{ 0, 0, 0, 0, 0, NULL, 0, 0, NULL, 0 },                                                  /* 0 */
	{ 0, 1, 0, 0, 508, Dos93_xWingComponents, 6, 5, NULL, 1 },                               /* 1 */
	{ 0, 1, 0, 0, 651, Dos93_sharedThreePartCraftComponents, 4, 3, NULL, 1 },                /* 2 */
	{ 0, 1, 0, 0, 391, Dos94_tugContainerComponents, 2, 1, NULL, 1 },                        /* 3 */
	{ 0, 1, 0, 0, 325, Dos93_sharedThreePartCraftComponents, 4, 3, NULL, 1 },                /* 4 */
	{ 0, 1, 0, 0, 391, Dos93_sharedThreePartCraftComponents, 4, 3, NULL, 1 },                /* 5 */
	{ 0, 1, 0, 0, 356, Dos93_sharedThreePartCraftComponents, 4, 3, NULL, 1 },                /* 6 */
	{ 0, 33, 0, 0, 620, Dos93_sharedThreePartCraftComponents, 4, 3, NULL, 1 },               /* 7 */
	{ 0, 33, 0, 1, 600, Dos94_tugContainerComponents, 2, 1, NULL, 1 },                       /* 8 */
	{ 0, 33, 0, 1, 930, Dos93_shuttleComponents, 5, 4, NULL, 1 },                            /* 9 */
	{ 0, 33, 0, 2, 186, Dos94_tugContainerComponents, 2, 1, NULL, 1 },                       /* 10 */
	{ 0, 1, 0, 3, 3100, Dos94_tugContainerComponents, 2, 1, NULL, 1 },                       /* 11 */
	{ 0, 33, 0, 3, 4960, Dos93_sharedThreePartCraftComponents, 4, 3, NULL, 1 },              /* 12 */
	{ 0, 33, 0, 4, 58000, Dos94_calamariCruiserComponents, 2, 2, NULL, 1 },                  /* 13 */
	{ 0, 33, 0, 4, 15000, Dos94_nebulonFrigateComponents, 4, 4, NULL, 1 },                   /* 14 */
	{ 0, 33, 0, 3, 6200, Dos93_corvetteComponents, 7, 7, NULL, 1 },                          /* 15 */
	{ 0, 33, 0, 4, 64000, Dos94_starDestroyerComponents, 13, 13, NULL, 1 },                  /* 16 */
	{ 0, 1, 0, 0, 356, Dos93_sharedThreePartCraftComponents, 4, 3, NULL, 1 },                /* 17 */
	{ 0, 0, 1, 6, 2048, NULL, 0, 0, NULL, 0 },                                               /* 18 */
	{ 0, 0, 1, 6, 2048, NULL, 0, 0, NULL, 0 },                                               /* 19 */
	{ 0, 0, 1, 6, 2048, NULL, 0, 0, NULL, 0 },                                               /* 20 */
	{ 0, 0, 1, 6, 2048, NULL, 0, 0, NULL, 0 },                                               /* 21 */
	{ 0, 0, 1, 6, 2048, NULL, 0, 0, NULL, 0 },                                               /* 22 */
	{ 0, 0, 1, 6, 2048, NULL, 0, 0, NULL, 0 },                                               /* 23 */
	{ 0, 0, 1, 6, 2048, NULL, 0, 0, NULL, 1 },                                               /* 24 */
	{ 0, 0, 1, 6, 2048, NULL, 0, 0, NULL, 1 },                                               /* 25 */
	{ 0, 33, 1, 7, 500, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 1 },              /* 26 */
	{ 0, 0, 1, 7, 500, NULL, 0, 0, NULL, 1 },                                                /* 27 */
	{ 0, 33, 1, 7, 500, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 1 },              /* 28 */
	{ 0, 0, 1, 7, 500, NULL, 0, 0, NULL, 1 },                                                /* 29 */
	{ 0, 33, 2, 8, 300, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 1 },              /* 30 */
	{ 0, 1, 2, 8, 250, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 1 },               /* 31 */
	{ 0, 33, 2, 8, 200, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 1 },              /* 32 */
	{ 0, 33, 3, 9, 6000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },             /* 33 */
	{ 0, 33, 3, 9, 6000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },             /* 34 */
	{ 0, 33, 3, 9, 6000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },             /* 35 */
	{ 0, 33, 3, 9, 6000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },             /* 36 */
	{ 0, 33, 3, 9, 6000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },             /* 37 */
	{ 0, 33, 3, 9, 6000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },             /* 38 */
	{ 0, 0, 3, 9, 480, NULL, 0, 0, NULL, 0 },                                                /* 39 */
	{ 0, 0, 3, 9, 480, NULL, 0, 0, NULL, 0 },                                                /* 40 */
	{ 0, 0, 3, 9, 480, NULL, 0, 0, NULL, 0 },                                                /* 41 */
	{ 0, 0, 3, 9, 480, NULL, 0, 0, NULL, 0 },                                                /* 42 */
	{ 0, 0, 3, 10, 480, Dos94_detachedComponentComponents, 2, 0, NULL, 0 },                  /* 43 */
	{ 0, 10, 3, 10, 512, Dos94_debris1Components, 1, 0, Dos94_debris1Palette, 0 },           /* 44 */
	{ 0, 10, 3, 10, 512, Dos94_debris2Components, 1, 0, Dos94_debris2Palette, 0 },           /* 45 */
	{ 0, 10, 3, 10, 512, Dos94_debris3Components, 1, 0, Dos94_debris3Palette, 0 },           /* 46 */
	{ 0, 10, 3, 10, 512, Dos94_debris4Components, 1, 0, Dos94_debris4Palette, 0 },           /* 47 */
	{ 0, 0, 3, 10, 480, NULL, 0, 0, NULL, 0 },                                               /* 48 */
	{ 0, 2, 4, 11, 480, NULL, 0, 0, Dos94_planet1Palette, 0 },                               /* 49 */
	{ 0, 2, 4, 11, 480, NULL, 0, 0, Dos94_planet2Palette, 0 },                               /* 50 */
	{ 0, 2, 4, 11, 480, NULL, 0, 0, Dos94_planet3Palette, 0 },                               /* 51 */
	{ 0, 2, 4, 11, 480, NULL, 0, 0, Dos94_planet4Palette, 0 },                               /* 52 */
	{ 0, 2, 4, 11, 480, NULL, 0, 0, Dos94_planet5Palette, 0 },                               /* 53 */
	{ 0, 2, 4, 11, 480, NULL, 0, 0, Dos94_planet6Palette, 0 },                               /* 54 */
	{ 0, 2, 4, 11, 480, NULL, 0, 0, Dos94_planet7Palette, 0 },                               /* 55 */
	{ 0, 2, 4, 11, 480, NULL, 0, 0, Dos94_planet8Palette, 0 },                               /* 56 */
	{ 0, 10, 4, 12, 480, NULL, 0, 0, Dos94_galaxyPalette, 0 },                               /* 57 */
	{ 0, 10, 4, 12, 480, NULL, 0, 0, Dos94_cluster1Palette, 0 },                             /* 58 */
	{ 0, 10, 4, 12, 480, NULL, 0, 0, Dos94_cluster2Palette, 0 },                             /* 59 */
	{ 0, 10, 4, 12, 480, NULL, 0, 0, Dos94_cluster3Palette, 0 },                             /* 60 */
	{ 0, 10, 4, 12, 480, NULL, 0, 0, Dos94_star1Palette, 0 },                                /* 61 */
	{ 0, 10, 4, 12, 480, NULL, 0, 0, Dos94_star2Palette, 0 },                                /* 62 */
	{ 0, 2, 4, 11, 480, NULL, 0, 0, Dos94_deathStarPalette, 0 },                             /* 63 */
	{ 0, 10, 5, 13, 3328, Dos94_explosionComponents, 1, 0, Dos94_explosionPalette, 0 },      /* 64 */
	{ 0, 10, 5, 13, 3328, Dos94_smallExplosionComponents, 1, 0, Dos94_explosionPalette, 0 }, /* 65 */
	{ 1, 64, 5, 13, 3328, Dos94_explosionComponents, 1, 0, Dos94_explosionPalette, 0 },      /* 66 */
	{ 1, 64, 5, 13, 3328, Dos94_explosionComponents, 1, 0, Dos94_explosionPalette, 0 },      /* 67 */
	{ 0, 10, 5, 13, 1664, Dos94_surfaceImpactComponents, 1, 0, Dos94_explosionPalette, 0 },  /* 68 */
	{ 0, 10, 5, 13, 1664, Dos94_surfaceImpactComponents, 1, 0, Dos94_sparkPalette, 0 },      /* 69 */
	{ 0, 10, 5, 13, 1280, Dos94_emberVariant0Components, 1, 0, Dos94_sparkPalette, 0 },      /* 70 */
	{ 0, 10, 5, 13, 1280, Dos94_emberVariant1Components, 1, 0, Dos94_emberPalette, 0 },      /* 71 */
	{ 0, 10, 5, 13, 1664, NULL, 0, 0, Dos94_explosionPalette, 0 },                           /* 72 */
	{ 0, 10, 5, 13, 1664, NULL, 0, 0, Dos94_sparkPalette, 0 },                               /* 73 */
	{ 0, 137, 6, 14, 2000, NULL, 0, 0, NULL, 0 },                                            /* 74 */
	{ 0, 137, 6, 14, 2000, NULL, 0, 0, NULL, 0 },                                            /* 75 */
	{ 0, 137, 6, 14, 2000, NULL, 0, 0, NULL, 0 },                                            /* 76 */
	{ 0, 137, 6, 14, 4000, NULL, 0, 0, NULL, 0 },                                            /* 77 */
	{ 0, 137, 6, 14, 4000, NULL, 0, 0, NULL, 0 },                                            /* 78 */
	{ 0, 137, 6, 14, 4000, NULL, 0, 0, NULL, 0 },                                            /* 79 */
	{ 0, 137, 6, 14, 8000, NULL, 0, 0, NULL, 0 },                                            /* 80 */
	{ 0, 137, 6, 14, 10000, NULL, 0, 0, NULL, 0 },                                           /* 81 */
	{ 0, 137, 6, 14, 4500, NULL, 0, 0, NULL, 0 },                                            /* 82 */
	{ 0, 137, 6, 14, 10000, NULL, 0, 0, NULL, 0 },                                           /* 83 */
	{ 0, 137, 6, 14, 12000, NULL, 0, 0, NULL, 0 },                                           /* 84 */
	{ 0, 137, 6, 14, 10000, NULL, 0, 0, NULL, 0 },                                           /* 85 */
	{ 0, 137, 6, 14, 12000, NULL, 0, 0, NULL, 0 },                                           /* 86 */
	{ 0, 137, 6, 14, 10000, NULL, 0, 0, NULL, 0 },                                           /* 87 */
	{ 0, 0, 6, 14, 15000, NULL, 0, 0, NULL, 0 },                                             /* 88 */
	{ 0, 137, 6, 14, 64000, NULL, 0, 0, NULL, 0 },                                           /* 89 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 90 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 91 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 92 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 93 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 94 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 95 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 96 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 97 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 98 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 99 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 100 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 101 */
	{ 0, 137, 6, 14, 4096, NULL, 0, 0, NULL, 0 },                                            /* 102 */
	{ 0, 137, 6, 14, 8192, NULL, 0, 0, NULL, 0 },                                            /* 103 */
	{ 0, 137, 6, 14, 8192, NULL, 0, 0, NULL, 0 },                                            /* 104 */
	{ 0, 137, 6, 14, 8192, NULL, 0, 0, NULL, 0 },                                            /* 105 */
	{ 0, 137, 6, 14, 8192, NULL, 0, 0, NULL, 0 },                                            /* 106 */
	{ 0, 137, 6, 14, 8192, NULL, 0, 0, NULL, 0 },                                            /* 107 */
	{ 0, 137, 6, 14, 8192, NULL, 0, 0, NULL, 0 },                                            /* 108 */
	{ 0, 73, 6, 14, 5000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },            /* 109 */
	{ 0, 73, 6, 14, 16000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },           /* 110 */
	{ 0, 73, 6, 14, 16000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },           /* 111 */
	{ 0, 73, 6, 14, 16000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },           /* 112 */
	{ 0, 73, 6, 14, 16000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },           /* 113 */
	{ 0, 73, 6, 14, 16000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },           /* 114 */
	{ 0, 73, 6, 14, 16000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },           /* 115 */
	{ 0, 73, 6, 14, 16000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },           /* 116 */
	{ 0, 73, 6, 14, 16000, Dos94_sharedSingleComponentComponents, 1, 1, NULL, 0 },           /* 117 */
	{ 0, 1, 0, 0, 680, Dos94_bWingComponents, 7, 6, NULL, 1 },                               /* 118 */
	{ 1, 16, 0, 4, 64000, Dos94_starDestroyerComponents, 13, 13, NULL, 1 },                  /* 119 */
};
