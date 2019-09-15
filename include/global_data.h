#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include "libtimidity/timidity.h"

#include "doomstat.h"
#include "m_fixed.h"
#include "am_map.h"
#include "g_game.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "r_defs.h"
#include "i_sound.h"
#include "m_menu.h"
#include "p_spec.h"
#include "p_enemy.h"
#include "p_map.h"
#include "p_maputl.h"

#include "p_mobj.h"

#include "mmus2mid.h"


typedef struct globals_t
{
    #include "global_defs.h"
} globals_t;

void InitGlobals();

extern globals_t* _g;

#endif // GLOBAL_DATA_H
