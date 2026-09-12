#include <stdlib.h>

U0 Main()
{
  if (GamepadButtonDown(-1) ||
      GamepadButtonPressed(HC_PAD_BUTTON_COUNT) ||
      GamepadAxis(HC_PAD_AXIS_COUNT) != 0)
    exit(1);
}

Main;
