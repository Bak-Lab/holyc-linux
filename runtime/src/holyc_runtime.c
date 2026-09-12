#include "holyc.h"

#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { HC_WIDTH = 640, HC_HEIGHT = 480 };

static const U32 palette[16] = {
    0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA,
    0xFFAA0000, 0xFFAA00AA, 0xFFAA5500, 0xFFAAAAAA,
    0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF,
    0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF,
};

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *texture;
static U8 pixels[HC_WIDTH * HC_HEIGHT];
static U32 display_pixels[HC_WIDTH * HC_HEIGHT];
static CTask task = {HC_WIDTH, HC_HEIGHT};
static U32 random_state = 0x54454D50U;
static Bool quit_requested;
static Bool key_down[HC_KEY_COUNT];
static Bool key_pressed[HC_KEY_COUNT];
static Bool initialized;

static I64 map_key(SDL_Scancode code)
{
  switch (code) {
  case SDL_SCANCODE_LEFT:
  case SDL_SCANCODE_A:
    return HC_KEY_LEFT;
  case SDL_SCANCODE_RIGHT:
  case SDL_SCANCODE_D:
    return HC_KEY_RIGHT;
  case SDL_SCANCODE_UP:
  case SDL_SCANCODE_W:
    return HC_KEY_UP;
  case SDL_SCANCODE_DOWN:
  case SDL_SCANCODE_S:
    return HC_KEY_DOWN;
  case SDL_SCANCODE_SPACE:
    return HC_KEY_SPACE;
  case SDL_SCANCODE_RETURN:
    return HC_KEY_ENTER;
  case SDL_SCANCODE_ESCAPE:
    return HC_KEY_ESCAPE;
  default:
    return -1;
  }
}

static U0 present(void)
{
  I64 index;

  if (!renderer)
    return;

  for (index = 0; index < HC_WIDTH * HC_HEIGHT; ++index)
    display_pixels[index] = palette[pixels[index] & 15];

  SDL_UpdateTexture(texture, NULL, display_pixels, HC_WIDTH * (int)sizeof(U32));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);
}

static U0 pump_events(void)
{
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      quit_requested = true;
    } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
      const I64 key = map_key(event.key.keysym.scancode);
      if (key >= 0) {
        key_down[key] = event.type == SDL_KEYDOWN;
        if (event.type == SDL_KEYDOWN && !event.key.repeat)
          key_pressed[key] = true;
      }
    }
  }
}

Bool HCInit(int argc, char **argv)
{
  (void)argc;
  (void)argv;

  if (initialized)
    return true;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    fprintf(stderr, "holyc runtime: SDL initialization failed: %s\n", SDL_GetError());
    return false;
  }

  window = SDL_CreateWindow(
      "HolyC Linux Runtime", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      HC_WIDTH, HC_HEIGHT, SDL_WINDOW_SHOWN);
  if (!window) {
    fprintf(stderr, "holyc runtime: window creation failed: %s\n", SDL_GetError());
    HCShutdown();
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  if (!renderer) {
    fprintf(stderr, "holyc runtime: renderer creation failed: %s\n", SDL_GetError());
    HCShutdown();
    return false;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                              SDL_TEXTUREACCESS_STREAMING, HC_WIDTH, HC_HEIGHT);
  if (!texture) {
    fprintf(stderr, "holyc runtime: texture creation failed: %s\n", SDL_GetError());
    HCShutdown();
    return false;
  }

  memset(pixels, BLACK, sizeof(pixels));
  memset(key_down, 0, sizeof(key_down));
  memset(key_pressed, 0, sizeof(key_pressed));
  quit_requested = false;
  initialized = true;
  present();
  return true;
}

U0 HCShutdown(void)
{
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  texture = NULL;
  renderer = NULL;
  window = NULL;
  initialized = false;
  SDL_Quit();
}

CDC *HCDCAlias(void)
{
  CDC *dc = malloc(sizeof(*dc));

  if (!dc) {
    fprintf(stderr, "holyc runtime: failed to allocate device context\n");
    exit(1);
  }

  dc->color = WHITE;
  dc->thick = 1;
  return dc;
}

CTask *HCGetTask(void)
{
  return &task;
}

Bool HCScanChar(void)
{
  I64 key;

  pump_events();
  if (quit_requested)
    return true;
  for (key = 0; key < HC_KEY_COUNT; ++key)
    if (key_pressed[key])
      return true;
  return false;
}

Bool HCQuitRequested(void)
{
  pump_events();
  return quit_requested;
}

Bool KeyDown(I64 key)
{
  pump_events();
  return key >= 0 && key < HC_KEY_COUNT && key_down[key];
}

Bool KeyPressed(I64 key)
{
  Bool pressed;

  pump_events();
  if (key < 0 || key >= HC_KEY_COUNT)
    return false;
  pressed = key_pressed[key];
  key_pressed[key] = false;
  return pressed;
}

I16 HCRandI16(void)
{
  random_state ^= random_state << 13;
  random_state ^= random_state >> 17;
  random_state ^= random_state << 5;
  return (I16)(random_state & 0xFFFF);
}

U0 DCDel(CDC *dc)
{
  free(dc);
}

U0 DCFill(CDC *dc)
{
  memset(pixels, dc ? (int)(dc->color & 15) : BLACK, sizeof(pixels));
}

Bool GrPlot(CDC *dc, I64 x, I64 y)
{
  if (!dc || x < 0 || x >= HC_WIDTH || y < 0 || y >= HC_HEIGHT)
    return false;

  pixels[y * HC_WIDTH + x] = (U8)(dc->color & 15);
  return true;
}

Bool GrLine(CDC *dc, I64 x1, I64 y1, I64 x2, I64 y2)
{
  I64 delta_x = llabs(x2 - x1);
  I64 step_x = x1 < x2 ? 1 : -1;
  I64 delta_y = -llabs(y2 - y1);
  I64 step_y = y1 < y2 ? 1 : -1;
  I64 error = delta_x + delta_y;

  for (;;) {
    GrPlot(dc, x1, y1);
    if (x1 == x2 && y1 == y2)
      break;
    I64 doubled_error = 2 * error;
    if (doubled_error >= delta_y) {
      error += delta_y;
      x1 += step_x;
    }
    if (doubled_error <= delta_x) {
      error += delta_x;
      y1 += step_y;
    }
  }
  return true;
}

Bool GrRect(CDC *dc, I64 x, I64 y, I64 width, I64 height)
{
  I64 row;
  I64 column;

  if (!dc || width < 0 || height < 0)
    return false;

  for (row = y; row < y + height; ++row)
    for (column = x; column < x + width; ++column)
      GrPlot(dc, column, row);
  return true;
}

Bool GrText(CDC *dc, I64 x, I64 y, const char *text)
{
  static const U8 font[96][5] = {
      ['!' - 32] = {0x00, 0x00, 0x5F, 0x00, 0x00},
      ['\'' - 32] = {0x00, 0x07, 0x00, 0x00, 0x00},
      [',' - 32] = {0x00, 0x40, 0x30, 0x00, 0x00},
      ['-' - 32] = {0x08, 0x08, 0x08, 0x08, 0x08},
      ['.' - 32] = {0x00, 0x60, 0x60, 0x00, 0x00},
      [':' - 32] = {0x00, 0x36, 0x36, 0x00, 0x00},
      ['?' - 32] = {0x02, 0x01, 0x51, 0x09, 0x06},
      ['0' - 32] = {0x3E, 0x51, 0x49, 0x45, 0x3E},
      ['1' - 32] = {0x00, 0x42, 0x7F, 0x40, 0x00},
      ['2' - 32] = {0x42, 0x61, 0x51, 0x49, 0x46},
      ['3' - 32] = {0x21, 0x41, 0x45, 0x4B, 0x31},
      ['4' - 32] = {0x18, 0x14, 0x12, 0x7F, 0x10},
      ['5' - 32] = {0x27, 0x45, 0x45, 0x45, 0x39},
      ['6' - 32] = {0x3C, 0x4A, 0x49, 0x49, 0x30},
      ['7' - 32] = {0x01, 0x71, 0x09, 0x05, 0x03},
      ['8' - 32] = {0x36, 0x49, 0x49, 0x49, 0x36},
      ['9' - 32] = {0x06, 0x49, 0x49, 0x29, 0x1E},
      ['A' - 32] = {0x7E, 0x11, 0x11, 0x11, 0x7E},
      ['B' - 32] = {0x7F, 0x49, 0x49, 0x49, 0x36},
      ['C' - 32] = {0x3E, 0x41, 0x41, 0x41, 0x22},
      ['D' - 32] = {0x7F, 0x41, 0x41, 0x22, 0x1C},
      ['E' - 32] = {0x7F, 0x49, 0x49, 0x49, 0x41},
      ['F' - 32] = {0x7F, 0x09, 0x09, 0x09, 0x01},
      ['G' - 32] = {0x3E, 0x41, 0x49, 0x49, 0x7A},
      ['H' - 32] = {0x7F, 0x08, 0x08, 0x08, 0x7F},
      ['I' - 32] = {0x00, 0x41, 0x7F, 0x41, 0x00},
      ['J' - 32] = {0x20, 0x40, 0x41, 0x3F, 0x01},
      ['K' - 32] = {0x7F, 0x08, 0x14, 0x22, 0x41},
      ['L' - 32] = {0x7F, 0x40, 0x40, 0x40, 0x40},
      ['M' - 32] = {0x7F, 0x02, 0x0C, 0x02, 0x7F},
      ['N' - 32] = {0x7F, 0x04, 0x08, 0x10, 0x7F},
      ['O' - 32] = {0x3E, 0x41, 0x41, 0x41, 0x3E},
      ['P' - 32] = {0x7F, 0x09, 0x09, 0x09, 0x06},
      ['Q' - 32] = {0x3E, 0x41, 0x51, 0x21, 0x5E},
      ['R' - 32] = {0x7F, 0x09, 0x19, 0x29, 0x46},
      ['S' - 32] = {0x46, 0x49, 0x49, 0x49, 0x31},
      ['T' - 32] = {0x01, 0x01, 0x7F, 0x01, 0x01},
      ['U' - 32] = {0x3F, 0x40, 0x40, 0x40, 0x3F},
      ['V' - 32] = {0x1F, 0x20, 0x40, 0x20, 0x1F},
      ['W' - 32] = {0x3F, 0x40, 0x38, 0x40, 0x3F},
      ['X' - 32] = {0x63, 0x14, 0x08, 0x14, 0x63},
      ['Y' - 32] = {0x07, 0x08, 0x70, 0x08, 0x07},
      ['Z' - 32] = {0x61, 0x51, 0x49, 0x45, 0x43},
  };
  I64 column;
  I64 horizontal;
  I64 row;
  I64 vertical;

  if (!dc || !text)
    return false;

  while (*text) {
    unsigned char character = (unsigned char)*text++;
    if (character >= 'a' && character <= 'z')
      character = (unsigned char)(character - 'a' + 'A');
    if (character >= 32 && character < 128) {
      for (column = 0; column < 5; ++column)
        for (row = 0; row < 7; ++row)
          if (font[character - 32][column] & (1U << row))
            for (horizontal = 0; horizontal < 2; ++horizontal)
              for (vertical = 0; vertical < 2; ++vertical)
                GrPlot(dc, x + column * 2 + horizontal,
                       y + row * 2 + vertical);
    }
    x += 12;
  }
  return true;
}

U0 Sleep(I64 milliseconds)
{
  pump_events();
  present();
  if (milliseconds > 0)
    SDL_Delay((U32)milliseconds);
}

I64 ClampI64(I64 value, I64 minimum, I64 maximum)
{
  if (value < minimum)
    return minimum;
  if (value > maximum)
    return maximum;
  return value;
}

I64 SignI64(I64 value)
{
  return (value > 0) - (value < 0);
}
