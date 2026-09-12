#include "Fixture.HC"
#include <stdlib.h>

U0 Main()
{
  if (FixtureValue() != 42)
    exit(1);
}

Main;
