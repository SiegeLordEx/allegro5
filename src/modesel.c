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
 *      GUI routines.
 *
 *      The graphics mode selection dialog.
 *
 *      By Shawn Hargreaves.
 *
 *      Rewritten by Henrik Stokseth.
 *
 *      See readme.txt for copyright information.
 */


#include <string.h>
#include <stdio.h>

#include "allegro.h"
#include "allegro/aintern.h"


static AL_CONST char *gfx_mode_getter(int index, int *list_size);
static AL_CONST char *gfx_card_getter(int index, int *list_size);
static AL_CONST char *gfx_depth_getter(int index, int *list_size);


#define ALL_BPP(w, h) { w, h, { TRUE, TRUE, TRUE, TRUE, TRUE }}

#define BPP_08    0
#define BPP_15    1
#define BPP_16    2
#define BPP_24    3
#define BPP_32    4
#define BPP_TOTAL 5

#define GFX_TITLE          1
#define GFX_OK             2
#define GFX_CANCEL         3
#define GFX_DRIVERLIST     4
#define GFX_MODELIST       5
#define GFX_DEPTHLIST      6


typedef struct MODE_LIST {
   int  w, h;
   char bpp[5];
} MODE_LIST;

typedef struct DRIVER_LIST {
   int       id;
   char      *name;
   MODE_LIST *mode_list;
   int       mode_count;
} DRIVER_LIST;

static MODE_LIST default_mode_list[] =
{
   ALL_BPP(320,  200 ),
   ALL_BPP(320,  240 ),
   ALL_BPP(640,  400 ),
   ALL_BPP(640,  480 ),
   ALL_BPP(800,  600 ),
   ALL_BPP(1024, 768 ),
   ALL_BPP(1280, 1024),
   ALL_BPP(1600, 1200),
   ALL_BPP(80,   80  ),
   ALL_BPP(160,  120 ),
   ALL_BPP(256,  200 ),
   ALL_BPP(256,  224 ),
   ALL_BPP(256,  240 ),
   ALL_BPP(256,  256 ),
   ALL_BPP(320,  100 ),
   ALL_BPP(320,  350 ),
   ALL_BPP(320,  400 ),
   ALL_BPP(320,  480 ),
   ALL_BPP(320,  600 ),
   ALL_BPP(360,  200 ),
   ALL_BPP(360,  240 ),
   ALL_BPP(360,  270 ),
   ALL_BPP(360,  360 ),
   ALL_BPP(360,  400 ),
   ALL_BPP(360,  480 ),
   ALL_BPP(360,  600 ),
   ALL_BPP(376,  282 ),
   ALL_BPP(376,  308 ),
   ALL_BPP(376,  564 ),
   ALL_BPP(400,  150 ),
   ALL_BPP(400,  300 ),
   ALL_BPP(400,  600 ),
   ALL_BPP(0,    0   )
};



static DRIVER_LIST *driver_list;
static int driver_count;

static char mode_string[20];
static DIALOG *what_dialog;



/* d_listchange_proc
 *
 *  Stores the current driver in d1 and graphics mode in d2;
 *  if a new driver is selected in the listbox, it changes the
 *  w/h and cdepth listboxes so that they redraw and they
 *  lose their selections. likewise if a new w/h is selected the
 *  cdepth listbox is updated.
 */
int d_listchange_proc(int msg, DIALOG* d, int c)
{
    if(msg != MSG_IDLE) return D_O_K;

    if(what_dialog[GFX_DRIVERLIST].d1 != d->d1) {
       d->d1 = what_dialog[GFX_DRIVERLIST].d1;
       d->d2 = what_dialog[GFX_MODELIST].d1;
       what_dialog[GFX_MODELIST].d1 = 0;
       what_dialog[GFX_MODELIST].d2 = 0;
       what_dialog[GFX_MODELIST].proc(MSG_DRAW, what_dialog + GFX_MODELIST, 0);
       what_dialog[GFX_DEPTHLIST].d1 = 0;
       what_dialog[GFX_DEPTHLIST].proc(MSG_DRAW, what_dialog + GFX_DEPTHLIST, 0);
    }

    if(what_dialog[GFX_MODELIST].d1 != d->d2) {
       d->d2 = what_dialog[GFX_MODELIST].d1;
       what_dialog[GFX_DEPTHLIST].d1 = 0;
       what_dialog[GFX_DEPTHLIST].proc(MSG_DRAW, what_dialog + GFX_DEPTHLIST, 0);
    }

    return D_O_K;
}



static DIALOG gfx_mode_dialog[] =
{
   /* (dialog proc)        (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)                     (dp2) (dp3) */
   { _gui_shadow_box_proc, 0,    0,    313,  159,  0,    0,    0,    0,       0,    0,    NULL,                    NULL, NULL  },
   { _gui_ctext_proc,      156,  8,    1,    1,    0,    0,    0,    0,       0,    0,    NULL,                    NULL, NULL  },
   { _gui_button_proc,     196,  105,  101,  17,   0,    0,    0,    D_EXIT,  0,    0,    NULL,                    NULL, NULL  },
   { _gui_button_proc,     196,  127,  101,  17,   0,    0,    27,   D_EXIT,  0,    0,    NULL,                    NULL, NULL  },
   { _gui_list_proc,       16,   28,   165,  116,  0,    0,    0,    D_EXIT,  0,    0,    (void*)gfx_card_getter,  NULL, NULL  },
   { _gui_list_proc,       196,  28,   101,  68,   0,    0,    0,    D_EXIT,  3,    0,    (void*)gfx_mode_getter,  NULL, NULL  },
   { d_listchange_proc,    0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    0,                       0,    0     },
   { d_yield_proc,         0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,                    NULL, NULL  },
   { NULL,                 0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,                    NULL, NULL  }
};



static DIALOG gfx_mode_ex_dialog[] =
{
   /* (dialog proc)        (x)   (y)   (w)   (h)   (fg)  (bg)  (key) (flags)  (d1)  (d2)  (dp)                     (dp2) (dp3) */
   { _gui_shadow_box_proc, 0,    0,    313,  159,  0,    0,    0,    0,       0,    0,    NULL,                    NULL, NULL  },
   { _gui_ctext_proc,      156,  8,    1,    1,    0,    0,    0,    0,       0,    0,    NULL,                    NULL, NULL  },
   { _gui_button_proc,     196,  105,  101,  17,   0,    0,    0,    D_EXIT,  0,    0,    NULL,                    NULL, NULL  },
   { _gui_button_proc,     196,  127,  101,  17,   0,    0,    27,   D_EXIT,  0,    0,    NULL,                    NULL, NULL  },
   { _gui_list_proc,       16,   28,   165,  68,   0,    0,    0,    D_EXIT,  0,    0,    (void*)gfx_card_getter,  NULL, NULL  },
   { _gui_list_proc,       196,  28,   101,  68,   0,    0,    0,    D_EXIT,  3,    0,    (void*)gfx_mode_getter,  NULL, NULL  },
   { _gui_list_proc,       16,   105,  165,  44,   0,    0,    0,    D_EXIT,  0,    0,    (void*)gfx_depth_getter, NULL, NULL  },
   { d_listchange_proc,    0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    0,                       0,    0     },
   { d_yield_proc,         0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,                    NULL, NULL  },
   { NULL,                 0,    0,    0,    0,    0,    0,    0,    0,       0,    0,    NULL,                    NULL, NULL  }
};



/* create_mode_list:
 *  Create a mode list table. Returns: -1 on failure.
 */
static int create_mode_list(DRIVER_LIST *driver_list_entry)
{
   MODE_LIST *temp_mode_list;
   GFX_MODE_LIST *gfx_mode_entry;
   int ret, mode, res_exist;

   /* autodetect drivers? */
   if ((driver_list_entry->id == GFX_AUTODETECT) || (driver_list_entry->id == GFX_AUTODETECT_WINDOWED) ||
       (driver_list_entry->id == GFX_AUTODETECT_FULLSCREEN) || (driver_list_entry->id == GFX_SAFE)) {
      driver_list_entry->mode_list = default_mode_list;
      driver_list_entry->mode_count = sizeof(default_mode_list) / sizeof(MODE_LIST) - 1;
      return 0;
   }

   ret = get_gfx_mode_list(driver_list_entry->id);

   /* driver supports fetch_mode_list? */
   if (ret == 0) {
      temp_mode_list = NULL;
      driver_list_entry->mode_count = 0;

      /* build mode list */
      for (gfx_mode_entry = gfx_mode_list; gfx_mode_entry->width; gfx_mode_entry++) {
         res_exist = FALSE;

         /* resolution already there? */
         for(mode = 0; mode < driver_list_entry->mode_count; mode++) {
            if ((gfx_mode_entry->width == temp_mode_list[mode].w) &&
                (gfx_mode_entry->height == temp_mode_list[mode].h)) {
               res_exist = TRUE;
               switch(gfx_mode_entry->bpp) {
                  case 8 : temp_mode_list[mode].bpp[BPP_08] = TRUE; break;
                  case 15: temp_mode_list[mode].bpp[BPP_15] = TRUE; break;
                  case 16: temp_mode_list[mode].bpp[BPP_16] = TRUE; break;
                  case 24: temp_mode_list[mode].bpp[BPP_24] = TRUE; break;
                  case 32: temp_mode_list[mode].bpp[BPP_32] = TRUE; break;
               }
               break;
            }
         }

         /* add resolution. */
         if (!res_exist) {
            driver_list_entry->mode_count++;
            temp_mode_list = realloc(temp_mode_list, sizeof(MODE_LIST) * (driver_list_entry->mode_count));
            if (!temp_mode_list) return -1;
            mode = driver_list_entry->mode_count - 1;

            temp_mode_list[mode].w = gfx_mode_entry->width;
            temp_mode_list[mode].h = gfx_mode_entry->height;
            temp_mode_list[mode].bpp[BPP_08] = FALSE;
            temp_mode_list[mode].bpp[BPP_15] = FALSE;
            temp_mode_list[mode].bpp[BPP_16] = FALSE;
            temp_mode_list[mode].bpp[BPP_24] = FALSE;
            temp_mode_list[mode].bpp[BPP_32] = FALSE;

            switch(gfx_mode_entry->bpp) {
               case 8 : temp_mode_list[mode].bpp[BPP_08] = TRUE; break;
               case 15: temp_mode_list[mode].bpp[BPP_15] = TRUE; break;
               case 16: temp_mode_list[mode].bpp[BPP_16] = TRUE; break;
               case 24: temp_mode_list[mode].bpp[BPP_24] = TRUE; break;
               case 32: temp_mode_list[mode].bpp[BPP_32] = TRUE; break;
            }
         }
      }

      /* terminate mode list */
      temp_mode_list = realloc(temp_mode_list, sizeof(MODE_LIST) * (driver_list_entry->mode_count + 1));
      if (!temp_mode_list) return -1;
      temp_mode_list[driver_list_entry->mode_count].w = 0;
      temp_mode_list[driver_list_entry->mode_count].h = 0;

      driver_list_entry->mode_list = temp_mode_list;
   }

   /* driver doesn't support fetch_mode_list() or the call failed? */
   else {
      driver_list_entry->mode_count = sizeof(default_mode_list) / sizeof(MODE_LIST) - 1;
      driver_list_entry->mode_list = default_mode_list;
   }

   destroy_gfx_mode_list();

   return 0;
}



/* create_driver_list:
 *  Fills the list of video cards with info about the available drivers. Returns: -1 on failure;
 */
static int create_driver_list()
{
   _DRIVER_INFO *driver_info;
   GFX_DRIVER   *gfx_driver;

   if (system_driver->gfx_drivers)
      driver_info = system_driver->gfx_drivers();
   else
      driver_info = _gfx_driver_list;

   driver_list = malloc(sizeof(DRIVER_LIST) * 3);
   if (!driver_list) return -1;

   driver_list[0].id   = GFX_AUTODETECT;
   driver_list[0].name = "Autodetect";
   create_mode_list(&driver_list[0]);

   driver_list[1].id   = GFX_AUTODETECT_FULLSCREEN;
   driver_list[1].name = "Autodetect fullscreen";
   create_mode_list(&driver_list[1]);

   driver_list[2].id   = GFX_AUTODETECT_WINDOWED;
   driver_list[2].name = "Autodetect windowed";
   create_mode_list(&driver_list[2]);

   driver_count = 0;

   while(driver_info[driver_count].driver) {
      driver_list = realloc(driver_list, sizeof(DRIVER_LIST) * (driver_count + 4));
      if (!driver_list) return -1;
      driver_list[driver_count+3].id   = driver_info[driver_count].id;
      gfx_driver = driver_info[driver_count].driver;
      driver_list[driver_count+3].name = (char *) gfx_driver->ascii_name;
      create_mode_list(&driver_list[driver_count+3]);
      driver_count++;
   }

   driver_count += 3;

   return 0;
}



/* destroy_driver_list:
 *  Removed dynamically allocated memory used by driver and mode lists.
 */
static void destroy_driver_list()
{
   int driver;

   for (driver=0; driver < driver_count; driver++)
      if (driver_list[driver].mode_list != default_mode_list) free(driver_list[driver].mode_list);
   free(driver_list);
}



/* gfx_card_getter:
 *  Listbox data getter routine for the graphics card list.
 */
static AL_CONST char *gfx_card_getter(int index, int *list_size)
{
   if (index < 0) {
      if (list_size)
	 *list_size = driver_count;
      return NULL;
   }

   return driver_list[index].name;
}



/* gfx_mode_getter:
 *  Listbox data getter routine for the graphics mode list.
 */
static AL_CONST char *gfx_mode_getter(int index, int *list_size)
{
   int entry;

   entry = what_dialog[GFX_DRIVERLIST].d1;

   if (index < 0) {
      if (list_size) {
	   *list_size = driver_list[entry].mode_count;
         return NULL;
      }
   }

   sprintf(mode_string, "%ix%i", driver_list[entry].mode_list[index].w, driver_list[entry].mode_list[index].h);
   return mode_string;
}



/* gfx_depth_getter:
 *  Listbox data getter routine for the color depth list.
 */
static AL_CONST char *gfx_depth_getter(int index, int *list_size)
{
   MODE_LIST *mode;
   int bpp_count, card_entry, mode_entry, bpp_entry;

   card_entry = what_dialog[GFX_DRIVERLIST].d1;
   mode_entry = what_dialog[GFX_MODELIST].d1;
   mode = &driver_list[card_entry].mode_list[mode_entry];

   if (index < 0) {
      if (list_size) {
         bpp_count = 0;
         if(mode->bpp[BPP_08]) bpp_count++;
         if(mode->bpp[BPP_15]) bpp_count++;
         if(mode->bpp[BPP_16]) bpp_count++;
         if(mode->bpp[BPP_24]) bpp_count++;
         if(mode->bpp[BPP_32]) bpp_count++;
         *list_size = bpp_count;
         return NULL;
      }
   }

   bpp_entry = 0;
   bpp_count = 0;
   while (bpp_count < index) {
      bpp_entry++;
      if (mode->bpp[bpp_entry]) bpp_count++;
   }

   switch (bpp_entry) {
      case BPP_08: sprintf(mode_string, " 8 bpp (256 color)"); break;
      case BPP_15: sprintf(mode_string, "15 bpp (32K color)"); break;
      case BPP_16: sprintf(mode_string, "16 bpp (64K color)"); break;
      case BPP_24: sprintf(mode_string, "24 bpp (16M color)"); break;
      case BPP_32: sprintf(mode_string, "32 bpp (16M color)"); break;
   }

   return mode_string;
   
}



/* gfx_mode_select:
 *  Displays the Allegro graphics mode selection dialog, which allows the
 *  user to select a screen mode and graphics card. Stores the selection
 *  in the three variables, and returns zero if it was closed with the 
 *  Cancel button, or non-zero if it was OK'd.
 */
int gfx_mode_select(int *card, int *w, int *h)
{
   int what_driver, what_mode, ret;

   clear_keybuf();

   while (gui_mouse_b());

   what_dialog = gfx_mode_dialog;

   gfx_mode_dialog[GFX_TITLE].dp = (void*)get_config_text("Graphics Mode");
   gfx_mode_dialog[GFX_OK].dp = (void*)get_config_text("OK");
   gfx_mode_dialog[GFX_CANCEL].dp = (void*)get_config_text("Cancel");

   create_driver_list();

   centre_dialog(gfx_mode_dialog);
   set_dialog_color(gfx_mode_dialog, gui_fg_color, gui_bg_color);
   ret = popup_dialog(gfx_mode_dialog, GFX_DRIVERLIST);

   what_driver = gfx_mode_dialog[GFX_DRIVERLIST].d1;
   what_mode = gfx_mode_dialog[GFX_MODELIST].d1;

   *card = driver_list[what_driver].id;

   *w = driver_list[what_driver].mode_list[what_mode].w;
   *h = driver_list[what_driver].mode_list[what_mode].h;

   destroy_driver_list();

   if (ret == GFX_CANCEL)
      return FALSE;
   else 
      return TRUE;
}



/* gfx_mode_select_ex:
 *  Extended version of the graphics mode selection dialog, which allows the 
 *  user to select the color depth as well as the resolution and hardware 
 *  driver. This version of the function reads the initial values from the 
 *  parameters when it activates, so you can specify the default values.
 */
int gfx_mode_select_ex(int *card, int *w, int *h, int *color_depth)
{
   int i, j, ret, what_driver, what_mode, what_bpp;

   clear_keybuf();

   while (gui_mouse_b());

   what_dialog = gfx_mode_ex_dialog;

   create_driver_list();

   gfx_mode_ex_dialog[GFX_DRIVERLIST].d1 = 0;

   for (i=0; i<driver_count; i++) {
      if (driver_list[i].id == *card) {
         gfx_mode_ex_dialog[GFX_DRIVERLIST].d1 = i;
         break;
      }
   }

   what_driver = i;
   if (what_driver == driver_count) what_driver = GFX_AUTODETECT;

   for (i=0; driver_list[what_driver].mode_list[i].w; i++) {
      if ((driver_list[what_driver].mode_list[i].w == *w) && (driver_list[what_driver].mode_list[i].h == *h)) {
         gfx_mode_ex_dialog[GFX_MODELIST].d1 = i;
         break; 
      }
   }

   what_mode = i;
   what_bpp = -1;

   for (i=0; i < BPP_TOTAL; i++) {
      if (driver_list[what_driver].mode_list[what_mode].bpp[i]) {
         what_bpp++;
         switch (*color_depth) {
            case  8: if (i == BPP_08) gfx_mode_ex_dialog[GFX_DEPTHLIST].d1 = what_bpp; break;
            case 15: if (i == BPP_15) gfx_mode_ex_dialog[GFX_DEPTHLIST].d1 = what_bpp; break;
            case 16: if (i == BPP_16) gfx_mode_ex_dialog[GFX_DEPTHLIST].d1 = what_bpp; break;
            case 24: if (i == BPP_24) gfx_mode_ex_dialog[GFX_DEPTHLIST].d1 = what_bpp; break;
            case 32: if (i == BPP_32) gfx_mode_ex_dialog[GFX_DEPTHLIST].d1 = what_bpp; break;
         }
      }
   }

   gfx_mode_ex_dialog[GFX_TITLE].dp = (void*)get_config_text("Graphics Mode");
   gfx_mode_ex_dialog[GFX_OK].dp = (void*)get_config_text("OK");
   gfx_mode_ex_dialog[GFX_CANCEL].dp = (void*)get_config_text("Cancel");

   centre_dialog(gfx_mode_ex_dialog);
   set_dialog_color(gfx_mode_ex_dialog, gui_fg_color, gui_bg_color);

   ret = popup_dialog(gfx_mode_ex_dialog, GFX_DRIVERLIST);

   what_driver = gfx_mode_ex_dialog[GFX_DRIVERLIST].d1;
   what_mode = gfx_mode_ex_dialog[GFX_MODELIST].d1;
   what_bpp = gfx_mode_ex_dialog[GFX_DEPTHLIST].d1;

   *card = driver_list[what_driver].id;

   *w = driver_list[what_driver].mode_list[what_mode].w;
   *h = driver_list[what_driver].mode_list[what_mode].h;

   j = -1;
   for (i=0; i < BPP_TOTAL; i++) {
      if (driver_list[what_driver].mode_list[what_mode].bpp[i]) {
         j++;
         if (j == what_bpp) switch (i) {
            case BPP_08: *color_depth = 8;  break;
            case BPP_15: *color_depth = 15; break;
            case BPP_16: *color_depth = 16; break;
            case BPP_24: *color_depth = 24; break;
            case BPP_32: *color_depth = 32; break;
         }
      }
   }

   destroy_driver_list();

   if (ret == GFX_CANCEL)
      return FALSE;
   else 
      return TRUE;
}

