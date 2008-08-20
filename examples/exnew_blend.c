/* An example demonstrating different blending modes.
 */
#include <allegro5/allegro5.h>
#include <allegro5/a5_font.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>

/* A structure holding all variables of our example program. */
struct Example
{
   ALLEGRO_BITMAP *example; /* Our example bitmap. */
   ALLEGRO_BITMAP *offscreen; /* An offscreen buffer, for testing. */
   ALLEGRO_BITMAP *memory; /* A memory buffer, for testing. */
   A5FONT_FONT *myfont; /* Our font. */
   ALLEGRO_EVENT_QUEUE *queue; /* Our events queue. */
   int image; /* Which test image to use. */
   int mode; /* How to draw it. */
   int BUTTONS_X; /* Where to draw buttons. */

   int FPS;
   double last_second;
   int frames_accum;
   double fps;
} ex;

/* Print some text with a shadow. */
static void print(int x, int y, bool vertical, char const *format, ...)
{
   va_list list;
   char message[1024];
   ALLEGRO_COLOR color;
   int h;
   int j;

   va_start(list, format);
   uvszprintf(message, sizeof message, format, list);
   va_end(list);

   h = a5font_text_height(ex.myfont);

   for (j = 0; j < 2; j++) {
      if (j == 0)
         color = al_map_rgb(0, 0, 0);
      else
         color = al_map_rgb(255, 255, 255);

      al_set_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, color);
      if (vertical) {
         int i;
         for (i = 0; i < ustrlen(message); i++) {
            char c[10] = "";
            int u = ugetat(message, i);
            usetat(c, 0, u);
            a5font_textout(ex.myfont, c, x + 1 - j, y + 1 - j + h * i);
         }
      }
      else {
         a5font_textout(ex.myfont, message, x + 1 - j, y + 1 - j);
      }
   }
}

/* Create an example bitmap. */
static ALLEGRO_BITMAP *create_example_bitmap(void)
{
   ALLEGRO_BITMAP *raw;
   ALLEGRO_BITMAP *bitmap;
   int i, j;
   ALLEGRO_LOCKED_REGION locked;
   unsigned char *data;

   /* Create out example bitmap as a memory bitmap with a fixed format. */
   al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
   al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ABGR_8888);
   raw = al_create_bitmap(100, 100);
   al_lock_bitmap(raw, &locked, ALLEGRO_LOCK_WRITEONLY);
   data = locked.data;
 
   for (j = 0; j < 100; j++) {
      for (i = 0; i < 100; i++) {
         int x = i - 50, y = j - 50;
         int r = sqrt(x * x + y * y);
         float rc = 1 - r / 50.0;
         if (rc < 0) rc = 0;
         data[i * 4 + 0] = i * 255 / 100;
         data[i * 4 + 1] = j * 255 / 100;
         data[i * 4 + 2] = rc * 255;
         data[i * 4 + 3] = rc * 255;
      }
      data += locked.pitch;
   }
   al_unlock_bitmap(raw);
   
   /* Now clone it into a fast display bitmap. */
   al_set_new_bitmap_flags(0);
   al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_WITH_ALPHA);
   bitmap = al_clone_bitmap(raw);
   
   /* And don't forget to destroy the memory copy. */
   al_destroy_bitmap(raw);

   return bitmap;
}

/* Draw our example scene. */
static void draw(void)
{
   ALLEGRO_COLOR blendcolor, white;
   ALLEGRO_COLOR test[5];
   ALLEGRO_BITMAP *target = al_get_target_bitmap();

   char const *blend_names[] = {"ZERO", "ONE", "ALPHA", "INVERSE"};
   char const *blend_vnames[] = {"ZERO", "ONE", "ALPHA", "INVER"};
   int blend_modes[] = {ALLEGRO_ZERO, ALLEGRO_ONE, ALLEGRO_ALPHA,
      ALLEGRO_INVERSE_ALPHA};
   float x = 40, y = 40;
   int i, j;

   al_clear(al_map_rgb_f(0.5, 0.5, 0.5));

   blendcolor = al_map_rgba_f(1, 1, 1, 1);
   white = al_map_rgba_f(1, 1, 1, 1);

   test[0] = al_map_rgba_f(1, 1, 1, 1);
   test[1] = al_map_rgba_f(1, 1, 1, 0.5);
   test[2] = al_map_rgba_f(1, 1, 1, 0.25);
   test[3] = al_map_rgba_f(1, 0, 0, 0.75);
   test[4] = al_map_rgba_f(0, 0, 0, 0);

   print(x, 0, false, "D  E  S  T  I  N  A  T  I  O  N  (%0.2f fps)", ex.fps);
   print(0, y, true, "S O U R C E");
   for (i = 0; i < 4; i++) {
      print(x + i * 110, 20, false, blend_names[i]);
      print(20, y + i * 110, true, blend_vnames[i]);
   }

   al_set_blender(ALLEGRO_ONE, ALLEGRO_ZERO, white);
   if (ex.mode >= 1 && ex.mode <= 5) {
      al_set_target_bitmap(ex.offscreen);
      al_clear(test[ex.mode - 1]);
   }
   if (ex.mode >= 6 && ex.mode <= 10) {
      al_set_target_bitmap(ex.memory);
      al_clear(test[ex.mode - 6]);
   }

   for (j = 0; j < 4; j++) {
      for (i = 0; i < 4; i++) {
         al_set_blender(blend_modes[j], blend_modes[i], blendcolor);
         if (ex.image == 0)
            al_draw_bitmap(ex.example, x + i * 110, y + j * 110, 0);
         else if (ex.image >= 1 && ex.image <= 6) {
            al_draw_rectangle(x + i * 110, y + j * 110,
               x + i * 110 + 100, y + j * 110 + 100, test[ex.image - 1],
               ALLEGRO_FILLED);
         }
      }
   }

   if (ex.mode >= 1 && ex.mode <= 5) {
      al_set_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, white);
      al_set_target_bitmap(target);
      al_draw_bitmap_region(ex.offscreen, x, y, 430, 430, x, y, 0);
   }
   if (ex.mode >= 6 && ex.mode <= 10) {
      al_set_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, white);
      al_set_target_bitmap(target);
      al_draw_bitmap_region(ex.memory, x, y, 430, 430, x, y, 0);
   }

   #define IS(x)  ((ex.image == x) ? "*" : " ")
   print(ex.BUTTONS_X, 20 * 1, false, "What to draw");
   print(ex.BUTTONS_X, 20 * 2, false, "%s Picture", IS(0));
   print(ex.BUTTONS_X, 20 * 3, false, "%s Rec1 (1/1/1/1)", IS(1));
   print(ex.BUTTONS_X, 20 * 4, false, "%s Rec2 (1/1/1/.5)", IS(2));
   print(ex.BUTTONS_X, 20 * 5, false, "%s Rec3 (1/1/1/.25)", IS(3));
   print(ex.BUTTONS_X, 20 * 6, false, "%s Rec4 (1/0/0/.75)", IS(4));
   print(ex.BUTTONS_X, 20 * 7, false, "%s Rec5 (0/0/0/0)", IS(5));
   #undef IS

   #define IS(x)  ((ex.mode == x) ? "*" : " ")
   print(ex.BUTTONS_X, 20 * 9, false, "Where to draw");
   print(ex.BUTTONS_X, 20 * 10, false, "%s screen", IS(0));

   print(ex.BUTTONS_X, 20 * 11, false, "%s offscreen1", IS(1));
   print(ex.BUTTONS_X, 20 * 12, false, "%s offscreen2", IS(2));
   print(ex.BUTTONS_X, 20 * 13, false, "%s offscreen3", IS(3));
   print(ex.BUTTONS_X, 20 * 14, false, "%s offscreen4", IS(4));
   print(ex.BUTTONS_X, 20 * 15, false, "%s offscreen5", IS(5));

   print(ex.BUTTONS_X, 20 * 16, false, "%s memory1", IS(6));
   print(ex.BUTTONS_X, 20 * 17, false, "%s memory2", IS(7));
   print(ex.BUTTONS_X, 20 * 18, false, "%s memory3", IS(8));
   print(ex.BUTTONS_X, 20 * 19, false, "%s memory4", IS(9));
   print(ex.BUTTONS_X, 20 * 20, false, "%s memory5", IS(10));
   #undef IS
}

/* Called a fixed amount of times per second. */
static void tick(void)
{
   /* Count frames during the last second or so. */
   double t = al_current_time();
   if (t >= ex.last_second + 1) {
      ex.fps = ex.frames_accum / (t - ex.last_second);
      ex.frames_accum = 0;
      ex.last_second = t;
   }

   draw();
   al_flip_display();
   ex.frames_accum++;
}

/* Run our test. */
static void run(void)
{
   ALLEGRO_EVENT event;
   float x, y;
   bool need_draw = true;

   while (1) {
      /* Perform frame skipping so we don't fall behind the timer events. */
      if (need_draw && al_event_queue_is_empty(ex.queue)) {
         tick();
         need_draw = false;
      }

      al_wait_for_event(ex.queue, &event);

      switch (event.type) {
         /* Was the X button on the window pressed? */
         case ALLEGRO_EVENT_DISPLAY_CLOSE:
            return;

         /* Was a key pressed? */
         case ALLEGRO_EVENT_KEY_DOWN:
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
               return;
            break;

         /* Is it time for the next timer tick? */
         case ALLEGRO_EVENT_TIMER:
            need_draw = true;
            break;

         /* Mouse click? */
         case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
            x = event.mouse.x;
            y = event.mouse.y;
            if (x >= ex.BUTTONS_X) {
               int button = y / 20;
               if (button == 2) ex.image = 0;
               if (button == 3) ex.image = 1;
               if (button == 4) ex.image = 2;
               if (button == 5) ex.image = 3;
               if (button == 6) ex.image = 4;
               if (button == 7) ex.image = 5;

               if (button == 10) ex.mode = 0;

               if (button == 11) ex.mode = 1;
               if (button == 12) ex.mode = 2;
               if (button == 13) ex.mode = 3;
               if (button == 14) ex.mode = 4;
               if (button == 15) ex.mode = 5;
               
               if (button == 16) ex.mode = 6;
               if (button == 17) ex.mode = 7;
               if (button == 18) ex.mode = 8;
               if (button == 19) ex.mode = 9;
               if (button == 20) ex.mode = 10;
            }
            break;
      }
   }
}

/* Initialize the example. */
static void init(void)
{
   ex.BUTTONS_X = 40 + 110 * 4;
   ex.FPS = 60;

   ex.myfont = a5font_load_font("font.tga", 0);
   if (!ex.myfont) {
      TRACE("font.tga not found\n");
      exit(1);
   }
   ex.example = create_example_bitmap();

   ex.offscreen = al_create_bitmap(640, 480);
   al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
   ex.memory = al_create_bitmap(640, 480);
}

int main(void)
{
   ALLEGRO_DISPLAY *display;
   ALLEGRO_TIMER *timer;

   al_init();
   al_install_keyboard();
   al_install_mouse();
   a5font_init();

   display = al_create_display(640, 480);
   if (!display) {
      TRACE("Error creating display\n");
      return 1;
   }
   al_show_mouse_cursor();

   init();

   timer = al_install_timer(1.0 / ex.FPS);

   ex.queue = al_create_event_queue();
   al_register_event_source(ex.queue,
      (ALLEGRO_EVENT_SOURCE *)al_get_keyboard());
   al_register_event_source(ex.queue, (ALLEGRO_EVENT_SOURCE *)al_get_mouse());
   al_register_event_source(ex.queue, (ALLEGRO_EVENT_SOURCE *)display);
   al_register_event_source(ex.queue, (ALLEGRO_EVENT_SOURCE *)timer);

   al_start_timer(timer);
   run();

   al_destroy_event_queue(ex.queue);  

   return 0;
}
END_OF_MAIN()
