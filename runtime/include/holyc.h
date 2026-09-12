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
U0 Sleep(I64 milliseconds);
I64 ClampI64(I64 value, I64 minimum, I64 maximum);
I64 SignI64(I64 value);

#endif
