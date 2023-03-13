#include <am.h>
#include <amdev.h>
#include <klib.h>
#include <klib-macros.h>

#define SIDE 16
//in pixel, size of one block

static int w, h;  // Screen size

#define KEYNAME(key) \
  [AM_KEY_##key] = #key,
static const char *key_names[] = { AM_KEYS(KEYNAME) };

static inline void puts(const char *s) {
  for (; *s; s++) putch(*s);
}

void print_key() {
  AM_INPUT_KEYBRD_T event = { .keycode = AM_KEY_NONE }; //X: type of keyboard event
  //X: used as a buffer to receive info from ioe_read

  ioe_read(AM_INPUT_KEYBRD, &event);
  if (event.keycode != AM_KEY_NONE && event.keydown) {
    puts("Key pressed: ");
    puts(key_names[event.keycode]);
    puts("\n");

    //my piece
    if(event.keycode == AM_KEY_ESCAPE)
      halt(0);
    //end my piece
  }
}

static void draw_tile(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: large stack-allocated memory

  AM_GPU_FBDRAW_T event = {
    .x = x, .y = y, .w = w, .h = h, .sync = 1,
    .pixels = pixels,
  };

  //paint an area of w*h 
  // with color 
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  ioe_write(AM_GPU_FBDRAW, &event);
}

void splash() {
  //fetch the width and hight of the screen
  //in pix
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;

  for (int x = 0; x * SIDE <= w; x ++) {
    for (int y = 0; y * SIDE <= h; y++) {
      if ((x & 1) ^ (y & 1)) {
        draw_tile(x * SIDE, y * SIDE, SIDE, SIDE, 0xff0000); // white
      }
    }
  }
  //X: the last argument of draw_tile signify the color
}

void show_photo()
{
  //fetch the width and hight of the screen
  AM_GPU_CONFIG_T info = {0};
  ioe_read(AM_GPU_CONFIG, &info);
  w = info.width;
  h = info.height;

  //the photo will be some grids 
  //4*4
  uint32_t color = 0x0;
  for (int x = 0; x * SIDE <= w; x ++) {
    
    for (int y = 0; y * SIDE <= h; y++) {
      assert(color < 0x1000000);
      draw_tile(x*SIDE,y*SIDE, SIDE, SIDE, color);
      color += 0x05;
    
    }
    color += 0x100;
  }


}

// Operating system is a C program!
int main(const char *args) {

  ioe_init();

  puts("main args = \"");
  puts(args);  // make run mainargs=xxx
  puts("\"\n");

  splash();
  //show_photo();
  
  puts("Press any key to see its key code...\n");
  while (1) {
    print_key();
  }
  return 0;
}
