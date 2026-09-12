#ifndef HOLYC_LINUX_H
#define HOLYC_LINUX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef void U0;
typedef bool Bool;
typedef int8_t I8;
typedef uint8_t U8;
typedef int16_t I16;
typedef uint16_t U16;
typedef int32_t I32;
typedef uint32_t U32;
typedef int64_t I64;
typedef uint64_t U64;
typedef double F64;

#define TRUE true
#define FALSE false

enum {
  BLACK,
  BLUE,
  GREEN,
  CYAN,
  RED,
  PURPLE,
  BROWN,
  LTGRAY,
  DKGRAY,
  LTBLUE,
  LTGREEN,
  LTCYAN,
  LTRED,
  LTPURPLE,
  YELLOW,
  WHITE
};

enum {
  HC_KEY_LEFT,
  HC_KEY_RIGHT,
  HC_KEY_UP,
  HC_KEY_DOWN,
  HC_KEY_SPACE,
  HC_KEY_ENTER,
  HC_KEY_ESCAPE,
  HC_KEY_COUNT
};

enum {
  HC_PAD_A,
  HC_PAD_B,
  HC_PAD_X,
  HC_PAD_Y,
  HC_PAD_BACK,
  HC_PAD_START,
  HC_PAD_LEFT_SHOULDER,
  HC_PAD_RIGHT_SHOULDER,
  HC_PAD_DPAD_UP,
  HC_PAD_DPAD_DOWN,
  HC_PAD_DPAD_LEFT,
  HC_PAD_DPAD_RIGHT,
  HC_PAD_BUTTON_COUNT
};

enum {
  HC_PAD_AXIS_LEFT_X,
  HC_PAD_AXIS_LEFT_Y,
  HC_PAD_AXIS_RIGHT_X,
  HC_PAD_AXIS_RIGHT_Y,
  HC_PAD_AXIS_TRIGGER_LEFT,
  HC_PAD_AXIS_TRIGGER_RIGHT,
  HC_PAD_AXIS_COUNT
};

typedef struct {
  I64 color;
  I64 thick;
} CDC;

typedef struct {
  I64 pix_width;
  I64 pix_height;
} CTask;

Bool HCInit(int argc, char **argv);
U0 HCShutdown(void);
CDC *HCDCAlias(void);
CTask *HCGetTask(void);
Bool HCScanChar(void);
Bool HCQuitRequested(void);
Bool KeyDown(I64 key);
Bool KeyPressed(I64 key);
Bool GamepadConnected(void);
Bool GamepadButtonDown(I64 button);
Bool GamepadButtonPressed(I64 button);
I64 GamepadAxis(I64 axis);
I16 HCRandI16(void);

#define DCAlias HCDCAlias()
#define Fs HCGetTask()
#define ScanChar HCScanChar()
#define RandI16 HCRandI16()

U0 DCDel(CDC *dc);
U0 DCFill(CDC *dc);
Bool GrPlot(CDC *dc, I64 x, I64 y);
Bool GrLine(CDC *dc, I64 x1, I64 y1, I64 x2, I64 y2);
Bool GrRect(CDC *dc, I64 x, I64 y, I64 width, I64 height);
Bool GrText(CDC *dc, I64 x, I64 y, const char *text);
U0 Sleep(I64 milliseconds);
I64 ClampI64(I64 value, I64 minimum, I64 maximum);
I64 SignI64(I64 value);

#endif
