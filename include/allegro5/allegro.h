/*         ______   ___    ___
 *        /\  _  \ /\_ \  /\_ \
 *        \ \ \L\ \\//\ \ \//\ \      __     __   _ __   ___
 *         \ \  __ \ \ \ \  \ \ \   /'__`\ /'_ `\/\`'__\/ __`\
 *          \ \ \/\ \ \_\ \_ \_\ \_/\  __//\ \L\ \ \ \//\ \L\ \
 *           \ \_\ \_\/\____\/\____\ \____\ \____ \ \_\\ \____/
 *            \/_/\/_/\/____/\/____/\/____/\/___L\ \/_/ \/___/
 *                                           /\____/
 *                                           \_/__/
 *
 *      Main header file for the entire Allegro library.
 *      (separate modules can be included from the allegro/ directory)
 *
 *      By Shawn Hargreaves.
 *
 *      Vincent Penquerc'h split the original allegro.h into separate headers.
 *
 *      See readme.txt for copyright information.
 */


#ifndef ALLEGRO_H
#define ALLEGRO_H


#include "allegro5/base.h"

//#include "allegro5/system.h"
#include "allegro5/system_new.h"
#include "allegro5/memory.h"
#include "allegro5/debug.h"

#include "allegro5/unicode.h"

#include "allegro5/altime.h"

#include "allegro5/events.h"

#include "allegro5/mouse.h"
#include "allegro5/timer.h"
#include "allegro5/keyboard.h"
#include "allegro5/joystick.h"

#include "allegro5/threads.h"

#include "allegro5/display_new.h"
#include "allegro5/bitmap_new.h"
#include "allegro5/color.h"

#include "allegro5/file.h"
#include "allegro5/lzss.h"

#ifdef ALLEGRO_CFG_OPENGL
   //temporary.... where's DECLARE_HANDLE?
   //#include "allegro5/a5_opengl.h"
#endif

#ifdef ALLEGRO_EXTRA_HEADER
   #include ALLEGRO_EXTRA_HEADER
#endif


#endif          /* ifndef ALLEGRO_H */


