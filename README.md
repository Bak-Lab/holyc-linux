# HolyC Linux

HolyC Linux is an experimental source-compatibility runtime for running a
useful subset of TempleOS-style HolyC programs as native Linux applications.
Its C++17 compiler frontend translates supported `.HC` source into C, compiles
it with Clang, and maps TempleOS graphics and system calls onto SDL2 and Linux.

This is not TempleOS in userspace, a kernel port, or a binary emulator. The
initial milestone is deliberately small: compile HolyC-like source, open a
640x480 16-color window, draw pixels, lines, and rectangles, and process basic
keyboard and window events.

## Requirements

- Linux
- Clang
- `pkg-config`
- SDL2 development files (`sdl2-compat` works on Arch Linux)

On Arch Linux:

```sh
sudo pacman -S --needed clang pkgconf sdl2-compat
```

## Try it

```sh
make
./.holyc-build/Hello
./.holyc-build/Graphics
```

Or compile a file directly:

```sh
./.holyc-build/bin/holyc examples/Graphics.HC
./.holyc-build/Graphics
```

Use `-o` to select the executable path and `--emit-c` to inspect the generated
C. Repeat `-I` to add directories containing reusable `.HC` files:

```sh
./.holyc-build/bin/holyc examples/Graphics.HC \
  -I path/to/includes -o build/graphics --emit-c build/graphics.c
```

Press any key or close the window to stop a graphical program that watches
`ScanChar`.

## Supported surface

The compiler currently accepts the C-compatible part of HolyC and recognizes
a final bare entry point such as `Main;`. The compatibility header provides:

- `U0`, `Bool`, `I8` through `I64`, `U8` through `U64`, and `F64`
- TempleOS's 16 color names
- `CDC`, `DCAlias`, `DCDel`, and `DCFill`
- `GrPlot`, `GrLine`, `GrRect`, and 5x7 `GrText`
- `Fs->pix_width` and `Fs->pix_height`
- Arrow/WASD input through `KeyDown` and `KeyPressed`
- `ScanChar`, `HCQuitRequested`, `Sleep`, `RandI16`, `ClampI64`, and `SignI64`

Unsupported HolyC syntax fails during Clang compilation with source locations
mapped back to the `.HC` file. Known direct hardware operations are rejected
earlier with a compatibility error.

## Security

Generated programs are ordinary native executables and are **not sandboxed**.
Only compile source you trust. TempleOS programs often assume kernel-level
access; review them before use, and expect hardware-specific APIs to be
unsupported.

## Scope and roadmap

Near-term work:

1. Improve HolyC syntax diagnostics and add language constructs as real
   programs require them.
2. Add text rendering, mouse state, sprites, and more `CDC` operations.
3. Build a compatibility fixture suite from small, redistributable examples.
4. Add a restricted execution mode before running untrusted source.
5. Target a simple graphical TempleOS demo end-to-end.

Contributions should add focused tests and document any compatibility
differences from TempleOS.

## License

MIT
