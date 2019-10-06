/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Rendering main loop and setup functions,
 *       utility functions (BSP, geometry, trigonometry).
 *      See tables.c, too.
 *
 *-----------------------------------------------------------------------------*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doomstat.h"
#include "d_net.h"
#include "w_wad.h"
#include "r_main.h"
#include "r_things.h"
#include "r_plane.h"
#include "r_bsp.h"
#include "r_draw.h"
#include "m_bbox.h"
#include "r_sky.h"
#include "v_video.h"
#include "lprintf.h"
#include "st_stuff.h"
#include "i_main.h"
#include "i_system.h"
#include "g_game.h"

#include "global_data.h"

// Fineangles in the SCREENWIDTH wide window.
#define FIELDOFVIEW 2048

const int viewheight = SCREENHEIGHT-ST_SCALED_HEIGHT;
const int viewheightfrac = (SCREENHEIGHT-ST_SCALED_HEIGHT)<<FRACBITS;
const int centery = (SCREENHEIGHT-ST_SCALED_HEIGHT)/2;
const int centerx = SCREENWIDTH/2;
const int centerxfrac = (SCREENWIDTH/2) << FRACBITS;
const int centeryfrac = ((SCREENHEIGHT-ST_SCALED_HEIGHT)/2) << FRACBITS;

const fixed_t projection = (SCREENWIDTH/2) << FRACBITS;
const fixed_t projectiony = ((SCREENHEIGHT * (SCREENWIDTH/2) * 320) / 200) / SCREENWIDTH * FRACUNIT;

const fixed_t pspritescale = FRACUNIT*SCREENWIDTH/320;
const fixed_t pspriteiscale = FRACUNIT*320/SCREENWIDTH;

const fixed_t pspriteyscale = (((SCREENHEIGHT*SCREENWIDTH)/SCREENWIDTH) << FRACBITS) / 200;

const angle_t clipangle = 537395200; //xtoviewangle[0];





angle_t R_PointToAngle2(fixed_t viewx, fixed_t viewy, fixed_t x, fixed_t y)
{
  return (y -= viewy, (x -= viewx) || y) ?
    x >= 0 ?
      y >= 0 ?
        (x > y) ? tantoangle[SlopeDiv(y,x)] :                      // octant 0
                ANG90-1-tantoangle[SlopeDiv(x,y)] :                // octant 1
        x > (y = -y) ? 0-tantoangle[SlopeDiv(y,x)] :                // octant 8
                       ANG270+tantoangle[SlopeDiv(x,y)] :          // octant 7
      y >= 0 ? (x = -x) > y ? ANG180-1-tantoangle[SlopeDiv(y,x)] : // octant 3
                            ANG90 + tantoangle[SlopeDiv(x,y)] :    // octant 2
        (x = -x) > (y = -y) ? ANG180+tantoangle[ SlopeDiv(y,x)] :  // octant 4
                              ANG270-1-tantoangle[SlopeDiv(x,y)] : // octant 5
    0;
}


//
// R_Init
//

void R_Init (void)
{
  lprintf(LO_INFO, "R_LoadTrigTables\n");
  R_LoadTrigTables();
  lprintf(LO_INFO, "R_InitData\n");
  R_InitData();
  lprintf(LO_INFO, "R_Init: R_InitPlanes\n");
  R_InitPlanes();
  lprintf(LO_INFO, "R_InitSkyMap\n");
  R_InitSkyMap();

  R_InitBuffer();
}

//
// R_PointInSubsector
//
// killough 5/2/98: reformatted, cleaned up

subsector_t *R_PointInSubsector(fixed_t x, fixed_t y)
{
  int nodenum = _g->numnodes-1;

  // special case for trivial maps (single subsector, no nodes)
  if (_g->numnodes == 0)
    return _g->subsectors;

  while (!(nodenum & NF_SUBSECTOR))
    nodenum = _g->nodes[nodenum].children[R_PointOnSide(x, y, _g->nodes+nodenum)];
  return &_g->subsectors[nodenum & ~NF_SUBSECTOR];
}

//
// R_SetupFrame
//

void R_SetupFrame (player_t *player)
{
  _g->viewplayer = player;

  _g->viewx = player->mo->x;
  _g->viewy = player->mo->y;
  _g->viewz = player->viewz;
  _g->viewangle = player->mo->angle;

  _g->extralight = player->extralight;

  _g->viewsin = finesine[_g->viewangle>>ANGLETOFINESHIFT];
  _g->viewcos = finecosine[_g->viewangle>>ANGLETOFINESHIFT];

  _g->fullcolormap = &_g->colormaps[0];

  if (player->fixedcolormap)
    {
      _g->fixedcolormap = _g->fullcolormap   // killough 3/20/98: use fullcolormap
        + player->fixedcolormap*256*sizeof(lighttable_t);
    }
  else
    _g->fixedcolormap = 0;

  _g->validcount++;
}

//
// R_RenderView
//
void R_RenderPlayerView (player_t* player)
{
    R_SetupFrame (player);

    // Clear buffers.
    R_ClearClipSegs ();
    R_ClearDrawSegs ();
    R_ClearPlanes ();
    R_ClearSprites ();

    // The head node is the last node output.
    R_RenderBSPNode (_g->numnodes-1);

    R_DrawPlanes ();

    R_DrawMasked ();
}
