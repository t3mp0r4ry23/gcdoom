/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2006 by
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
 *  DOOM graphics stuff for SDL
 *
 *-----------------------------------------------------------------------------
 */
#define PI 3.14159265

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "SDL2/SDL.h"

#include "m_argv.h"
#include "doomstat.h"
#include "doomdef.h"
#include "doomtype.h"
#include "v_video.h"
#include "r_draw.h"
#include "d_main.h"
#include "d_event.h"
#include "i_joy.h"
#include "i_video.h"
#include "z_zone.h"
#include "s_sound.h"
#include "sounds.h"
#include "w_wad.h"
#include "st_stuff.h"
#include "lprintf.h"
//#include <wiiuse/wpad.h>
#include <ogc/pad.h>
#include <math.h>

int gl_colorbuffer_bits=16;
int gl_depthbuffer_bits=16;

extern void M_QuitDOOM(int choice);
#ifdef DISABLE_DOUBLEBUFFER
int use_doublebuffer = 0;
#else
int use_doublebuffer = 1; // Included not to break m_misc, but not relevant to SDL
#endif
int use_fullscreen;
int desired_fullscreen;
static SDL_Surface *screen;
static SDL_Window *window;

////////////////////////////////////////////////////////////////////////////
// Input code
int             leds_always_off = 0; // Expected by m_misc, not relevant

// Mouse handling
extern int     usemouse;        // config file var
static bool grabMouse;       // internal var
static int mouse_currently_grabbed;

extern int chosenController;
/////////////////////////////////////////////////////////////////////////////////
// Keyboard handling

//
//  Translates the key currently in key
//

static int I_TranslateKey(SDL_Keysym* key)
{
  int rc = 0;

  switch (key->sym) {
  case SDLK_LEFT: rc = KEYD_LEFTARROW;  break;
  case SDLK_RIGHT:  rc = KEYD_RIGHTARROW; break;
  case SDLK_DOWN: rc = KEYD_DOWNARROW;  break;
  case SDLK_UP:   rc = KEYD_UPARROW;  break;
  case SDLK_ESCAPE: rc = KEYD_ESCAPE; break;
  case SDLK_RETURN: rc = KEYD_ENTER;  break;
  case SDLK_TAB:  rc = KEYD_TAB;    break;
  case SDLK_F1:   rc = KEYD_F1;   break;
  case SDLK_F2:   rc = KEYD_F2;   break;
  case SDLK_F3:   rc = KEYD_F3;   break;
  case SDLK_F4:   rc = KEYD_F4;   break;
  case SDLK_F5:   rc = KEYD_F5;   break;
  case SDLK_F6:   rc = KEYD_F6;   break;
  case SDLK_F7:   rc = KEYD_F7;   break;
  case SDLK_F8:   rc = KEYD_F8;   break;
  case SDLK_F9:   rc = KEYD_F9;   break;
  case SDLK_F10:  rc = KEYD_F10;    break;
  case SDLK_F11:  rc = KEYD_F11;    break;
  case SDLK_F12:  rc = KEYD_F12;    break;
  case SDLK_BACKSPACE:  rc = KEYD_BACKSPACE;  break;
  case SDLK_DELETE: rc = KEYD_DEL;  break;
  case SDLK_INSERT: rc = KEYD_INSERT; break;
  case SDLK_PAGEUP: rc = KEYD_PAGEUP; break;
  case SDLK_PAGEDOWN: rc = KEYD_PAGEDOWN; break;
  case SDLK_HOME: rc = KEYD_HOME; break;
  case SDLK_END:  rc = KEYD_END;  break;
  case SDLK_PAUSE:  rc = KEYD_PAUSE;  break;
  case SDLK_EQUALS: rc = KEYD_EQUALS; break;
  case SDLK_MINUS:  rc = KEYD_MINUS;  break;
  case SDLK_KP_0:  rc = KEYD_KEYPAD0;  break;
  case SDLK_KP_1:  rc = KEYD_KEYPAD1;  break;
  case SDLK_KP_2:  rc = KEYD_KEYPAD2;  break;
  case SDLK_KP_3:  rc = KEYD_KEYPAD3;  break;
  case SDLK_KP_4:  rc = KEYD_KEYPAD4;  break;
  case SDLK_KP_5:  rc = KEYD_KEYPAD5;  break;
  case SDLK_KP_6:  rc = KEYD_KEYPAD6;  break;
  case SDLK_KP_7:  rc = KEYD_KEYPAD7;  break;
  case SDLK_KP_8:  rc = KEYD_KEYPAD8;  break;
  case SDLK_KP_9:  rc = KEYD_KEYPAD9;  break;
  case SDLK_KP__PLUS:  rc = KEYD_KEYPADPLUS; break;
  case SDLK_KP__MINUS: rc = KEYD_KEYPADMINUS;  break;
  case SDLK_KP__DIVIDE:  rc = KEYD_KEYPADDIVIDE; break;
  case SDLK_KP__MULTIPLY: rc = KEYD_KEYPADMULTIPLY; break;
  case SDLK_KP__ENTER: rc = KEYD_KEYPADENTER;  break;
  case SDLK_KP__PERIOD:  rc = KEYD_KEYPADPERIOD; break;
  case SDLK_LSHIFT:
  case SDLK_RSHIFT: rc = KEYD_RSHIFT; break;
  case SDLK_LCTRL:
  case SDLK_RCTRL:  rc = KEYD_RCTRL;  break;
  case SDLK_LALT:
  case SDLK_RALT:  rc = KEYD_RALT;   break;
  case SDLK_CAPSLOCK: rc = KEYD_CAPSLOCK; break;
  default:    rc = key->sym;    break;
  }

  return rc;

}

/////////////////////////////////////////////////////////////////////////////////
// Main input code

/* cph - pulled out common button code logic */
static int I_SDLtoDoomMouseState(Uint8 buttonstate)
{
  return 0
      | (buttonstate & SDL_BUTTON(1) ? 1 : 0)
      | (buttonstate & SDL_BUTTON(2) ? 2 : 0)
      | (buttonstate & SDL_BUTTON(3) ? 4 : 0);
}

static void I_GetEvent(SDL_Event *Event)
{
  event_t event;

  switch (Event->type) {
	case SDL_KEYDOWN:
	if (chosenController == -1)
		chosenController = 2;
	if (chosenController != 2)
		break;
    event.type = ev_keydown;
    event.data1 = I_TranslateKey(&Event->key.keysym);
    D_PostEvent(&event);
    break;

  case SDL_KEYUP:
 	if (chosenController != 2)
		break;
    event.type = ev_keyup;
    event.data1 = I_TranslateKey(&Event->key.keysym);
    D_PostEvent(&event);
	break;
  case SDL_MOUSEBUTTONDOWN:
  case SDL_MOUSEBUTTONUP:
  /*if (MOUSE_IsConnected())
  {
    event.type = ev_mouse;
    event.data1 = I_SDLtoDoomMouseState(SDL_GetMouseState(NULL, NULL));
    event.data2 = event.data3 = 0;
    D_PostEvent(&event);
  }*/
  break;

  case SDL_MOUSEMOTION:
  /*if (MOUSE_IsConnected()) {
    event.type = ev_mouse;
    event.data1 = I_SDLtoDoomMouseState(Event->motion.state);
    event.data2 = Event->motion.xrel << 5;
    event.data3 = -Event->motion.yrel << 5;
    D_PostEvent(&event);
  }*/
  break;


  case SDL_QUIT:
    S_StartSound(NULL, sfx_swtchn);
    M_QuitDOOM(0);

  default:
    break;
  }
}


//
// I_StartTic
//

void I_StartTic (void)
{
  SDL_Event Event;
    int should_be_grabbed = grabMouse &&
      !(paused || (gamestate != GS_LEVEL) || demoplayback);

  while ( SDL_PollEvent(&Event) )
  {
    I_GetEvent(&Event);
  }

  I_PollJoystick();
}

//
// I_StartFrame
//
void I_StartFrame (void)
{
}

//
// I_InitInputs
//

static void I_InitInputs(void)
{
//  // check if the user wants to grab the mouse
//  grabMouse = M_CheckParm("-nomouse") ? false : usemouse ? true : false;
//  // e6y: fix for turn-snapping bug on fullscreen in software mode
//  if (!M_CheckParm("-nomouse"))
//    SDL_WarpMouse((unsigned short)(SCREENWIDTH/2), (unsigned short)(SCREENHEIGHT/2));

  I_InitJoystick();
}
/////////////////////////////////////////////////////////////////////////////

// I_SkipFrame
//
// Returns true if it thinks we can afford to skip this frame

inline static bool I_SkipFrame(void)
{
  static int frameno;

  frameno++;
  switch (gamestate) {
  case GS_LEVEL:
    if (!paused)
      return false;
  default:
    // Skip odd frames
    return (frameno & 1) ? true : false;
  }
}

///////////////////////////////////////////////////////////
// Palette stuff.
//
static void I_UploadNewPalette(int pal)
{
  // This is used to replace the current 256 colour cmap with a new one
  // Used by 256 colour PseudoColor modes

  // Array of SDL_Color structs used for setting the 256-colour palette
  static SDL_Color* colours;
  static int cachedgamma;
  static size_t num_pals;

  if (V_GetMode() == VID_MODEGL)
    return;

  if ((colours == NULL) || (cachedgamma != usegamma)) {
    int pplump = W_GetNumForName("PLAYPAL");
    int gtlump = (W_CheckNumForName)("GAMMATBL",ns_prboom);
    register const byte * palette = W_CacheLumpNum(pplump);
    register const byte * const gtable = (const byte *)W_CacheLumpNum(gtlump) + 256*(cachedgamma = usegamma);
    register int i;

    num_pals = W_LumpLength(pplump) / (3*256);
    num_pals *= 256;

    if (!colours) {
      // First call - allocate and prepare colour array
      colours = malloc(sizeof(*colours)*num_pals);
    }

    // set the colormap entries
    for (i=0 ; (size_t)i<num_pals ; i++) {
      colours[i].r = gtable[palette[0]];
      colours[i].g = gtable[palette[1]];
      colours[i].b = gtable[palette[2]];
      palette += 3;
    }

    W_UnlockLumpNum(pplump);
    W_UnlockLumpNum(gtlump);
    num_pals/=256;
  }

#ifdef RANGECHECK
  if ((size_t)pal >= num_pals)
    I_Error("I_UploadNewPalette: Palette number out of range (%d>=%d)",
      pal, num_pals);
#endif

  // store the colors to the current display
  // SDL_SetColors(SDL_GetVideoSurface(), colours+256*pal, 0, 256);
  SDL_SetPaletteColors(SDL_GetWindowSurface(window)->format->palette, colours+256*pal, 0, 256);
}

//////////////////////////////////////////////////////////////////////////////
// Graphics API

void I_ShutdownGraphics(void)
{
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit (void)
{
}

//
// I_FinishUpdate
//
static int newpal = 0;
#define NO_PALETTE_CHANGE 1000

void I_FinishUpdate (void)
{
  if (I_SkipFrame()) return;

//#ifdef MONITOR_VISIBILITY
//  if (!(SDL_GetAppState()&SDL_APPACTIVE)) {
//    return;
//  }
//#endif

#ifdef GL_DOOM
  if (V_GetMode() == VID_MODEGL) {
    // proff 04/05/2000: swap OpenGL buffers
    gld_Finish();
    return;
  }
#endif
  if (SDL_MUSTLOCK(screen)) {
      int h;
      byte *src;
      byte *dest;

      if (SDL_LockSurface(screen) < 0) {
        lprintf(LO_INFO,"I_FinishUpdate: %s\n", SDL_GetError());
        return;
      }
      dest=screen->pixels;
      src=screens[0].data;
      h=screen->h;
      for (; h>0; h--)
      {
        memcpy(dest,src,SCREENWIDTH);
        dest+=screen->pitch;
        src+=screens[0].pitch;
      }
      SDL_UnlockSurface(screen);
  }
  /* Update the display buffer (flipping video pages if supported)
   * If we need to change palette, that implicitely does a flip */
  if (newpal != NO_PALETTE_CHANGE) {
    I_UploadNewPalette(newpal);
    newpal = NO_PALETTE_CHANGE;
  }
  SDL_UpdateWindowSurface(window);
}

//
// I_ReadScreen
//
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
void I_ReadScreen (screeninfo_t *dest)
{
  int h;
  byte *srcofs = screens[0].data;
  byte *dstofs = dest->data;
  int width, height;
  width = min(screens[0].width, dest->width);
  height = min(screens[0].height, dest->height);
  for (h=height; h>0; h--) {
    memcpy(dstofs, srcofs, width);
    srcofs += screens[0].pitch;
    dstofs += dest->pitch;
  }
}

//
// I_SetPalette
//
void I_SetPalette (int pal)
{
  newpal = pal;
}

// I_PreInitGraphics

static void I_ShutdownSDL(void)
{
  SDL_Quit();
  return;
}

void I_PreInitGraphics(void)
{
  // Initialize SDL
  //unsigned int flags = 0;
  //if (!(M_CheckParm("-nodraw") && M_CheckParm("-nosound")))
  //  flags = SDL_INIT_VIDEO;
//#ifdef _DEBUG
  //flags |= SDL_INIT_NOPARACHUTE;
//#endif
  if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
    I_Error("Could not initialize SDL [%s]", SDL_GetError());
  }

  atexit(I_ShutdownSDL);
}

// e6y
// GLBoom use this function for trying to set the closest supported resolution if the requested mode can't be set correctly.
// For example glboom.exe -geom 1025x768 -nowindow will set 1024x768.
// It should be used only for fullscreen modes.
static void I_ClosestResolution (int *width, int *height, int flags)
{
  /*SDL_Rect **modes;
  int twidth, theight;
  int cwidth = 0, cheight = 0;
//  int iteration;
  int i;
  unsigned int closest = UINT_MAX;
  unsigned int dist;

  modes = SDL_ListModes(NULL, SDL_INIT_EVERYTHING);

  //for (iteration = 0; iteration < 2; iteration++)
  {
    for(i=0; modes[i]; ++i)
    {
      twidth = modes[i]->w;
      theight = modes[i]->h;

      if (twidth > MAX_SCREENWIDTH || theight> MAX_SCREENHEIGHT)
        continue;
      
      if (twidth == *width && theight == *height)
        return;

      //if (iteration == 0 && (twidth < *width || theight < *height))
      //  continue;

      dist = (twidth - *width) * (twidth - *width) + 
             (theight - *height) * (theight - *height);

      if (dist < closest)
      {
        closest = dist;
        cwidth = twidth;
        cheight = theight;
      }
    }
    if (closest != 4294967295u)
    {
      *width = cwidth;
      *height = cheight;
      return;
    }
  }*/
  return;
}  

// CPhipps -
// I_CalculateRes
// Calculates the screen resolution, possibly using the supplied guide
void I_CalculateRes(unsigned int width, unsigned int height)
{
  // e6y: how about 1680x1050?
  /*
  SCREENWIDTH = (width+3) & ~3;
  SCREENHEIGHT = (height+3) & ~3;
  */

// e6y
// GLBoom will try to set the closest supported resolution 
// if the requested mode can't be set correctly.
// For example glboom.exe -geom 1025x768 -nowindow will set 1024x768.
// It affects only fullscreen modes.
  if (V_GetMode() == VID_MODEGL) {
    if ( desired_fullscreen )
    {
      I_ClosestResolution(&width, &height, SDL_WINDOW_OPENGL|SDL_WINDOW_FULLSCREEN);
    }
    SCREENWIDTH = width;
    SCREENHEIGHT = height;
    SCREENPITCH = SCREENWIDTH;
  }
  // swc - disabled this, because it didn't compute the screenwidth correctly on the wii
  /* else {
    SCREENWIDTH = (width+15) & ~15;
    SCREENHEIGHT = height;
    if (!(SCREENWIDTH % 1024)) {
      SCREENPITCH = SCREENWIDTH+32;
    } else {
      SCREENPITCH = SCREENWIDTH;
    }
  }*/
}

// CPhipps -
// I_SetRes
// Sets the screen resolution
void I_SetRes(void)
{
  int i;

  I_CalculateRes(SCREENWIDTH, SCREENHEIGHT);

  // set first three to standard values
  for (i=0; i<3; i++) {
    screens[i].width = SCREENWIDTH;
    screens[i].height = SCREENHEIGHT;
    screens[i].pitch = SCREENPITCH;
  }

  // statusbar
  screens[4].width = SCREENWIDTH;
  screens[4].height = (ST_SCALED_HEIGHT+1);
  screens[4].pitch = SCREENPITCH;

  lprintf(LO_INFO,"I_SetRes: Using resolution %dx%d\n", SCREENWIDTH, SCREENHEIGHT);
}

void I_InitGraphics(void)
{
  char titlebuffer[2048];
  static int    firsttime=1;

  if (firsttime)
  {
    firsttime = 0;

    atexit(I_ShutdownGraphics);
    lprintf(LO_INFO, "I_InitGraphics: %dx%d\n", SCREENWIDTH, SCREENHEIGHT);

    /* Set the video mode */
    I_UpdateVideoMode();

    /* Setup the window title and create the window*/
    strcpy(titlebuffer,PACKAGE);
    strcat(titlebuffer," ");
    strcat(titlebuffer,VERSION);
    window = SDL_CreateWindow(titlebuffer, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREENWIDTH, SCREENHEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);

    /* Initialize the input system */
    I_InitInputs();
  }
}

void I_UpdateVideoMode(void)
{
  int init_flags;
  int i;
  video_mode_t mode;

  lprintf(LO_INFO, "I_UpdateVideoMode: %dx%d (%s)\n", SCREENWIDTH, SCREENHEIGHT, desired_fullscreen ? "fullscreen" : "nofullscreen");

  mode = default_videomode;
  if ((i=M_CheckParm("-vidmode")) && i<myargc-1) {
    /*if (!stricmp(myargv[i+1],"16")) {
      mode = VID_MODE16;
    } else if (!stricmp(myargv[i+1],"32")) {
      mode = VID_MODE32;
    } else*/ if (!stricmp(myargv[i+1],"gl")) {
      mode = VID_MODEGL;
    } else {
      mode = VID_MODE8;
    }
  }
  V_InitMode(mode);

  V_FreeScreens();

  I_SetRes();

  // Initialize SDL with this graphics mode
  /*if (V_GetMode() == VID_MODEGL) {
    init_flags = SDL_OPENGL;
  } else {
    if (use_doublebuffer)
      init_flags = SDL_DOUBLEBUF;
    else
      init_flags = SDL_SWSURFACE;
#ifndef _DEBUG
    init_flags |= SDL_HWPALETTE;
#endif*/
  }

  //init_flags = SDL_DOUBLEBUF;

  //if ( desired_fullscreen )
    //init_flags |= SDL_FULLSCREEN;

  /*if (V_GetMode() == VID_MODEGL) {
    SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, 0 );
    SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    SDL_GL_SetAttribute( SDL_GL_BUFFER_SIZE, gl_colorbuffer_bits );
    SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, gl_depthbuffer_bits );
    screen = SDL_GetWindowSurface(SCREENWIDTH, SCREENHEIGHT, gl_colorbuffer_bits, init_flags);
  } else {*/
  screen = SDL_GetWindowSurface(window);
  //}

  if(screen == NULL) {
    I_Error("Couldn't set %dx%d video mode [%s]", SCREENWIDTH, SCREENHEIGHT, SDL_GetError());
  }

  lprintf(LO_INFO, "I_UpdateVideoMode: 0x%x, %s, %s\n", init_flags, screen->pixels ? "SDL buffer" : "own buffer", SDL_MUSTLOCK(screen) ? "lock-and-copy": "direct access");

  mouse_currently_grabbed = false;

  // Get the info needed to render to the display
  if (!SDL_MUSTLOCK(screen))
  {
    screens[0].not_on_heap = true;
    screens[0].data = (unsigned char *) (screen->pixels);
    screens[0].pitch = screen->pitch;
  }
  else
  {
    screens[0].not_on_heap = false;
  }

  V_AllocScreens();

  // Hide pointer while over this window
  SDL_ShowCursor(0);

  R_InitBuffer(SCREENWIDTH, SCREENHEIGHT);

  if (V_GetMode() == VID_MODEGL) {
    int temp;
    lprintf(LO_INFO,"SDL OpenGL PixelFormat:\n");
    SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_RED_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_GREEN_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_BLUE_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_STENCIL_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_RED_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_RED_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_GREEN_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_GREEN_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_BLUE_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_BLUE_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_ACCUM_ALPHA_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_ACCUM_ALPHA_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &temp );
    lprintf(LO_INFO,"    SDL_GL_DOUBLEBUFFER: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_BUFFER_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_BUFFER_SIZE: %i\n",temp);
    SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &temp );
    lprintf(LO_INFO,"    SDL_GL_DEPTH_SIZE: %i\n",temp);
#ifdef GL_DOOM
    gld_Init(SCREENWIDTH, SCREENHEIGHT);
#endif
  }
}



