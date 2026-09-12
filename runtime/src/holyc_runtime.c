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
static Bool initialized;

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
    if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN)
      quit_requested = true;
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
  pump_events();
  return quit_requested;
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
  present();
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
