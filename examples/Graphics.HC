U0 Main()
{
  CDC *dc = DCAlias;
  I64 frame, x;

  dc->color = BLUE;
  DCFill(dc);

  for (frame = 0; frame < 180 && !ScanChar; frame++) {
    dc->color = frame & 15;
    x = (frame * 4) % Fs->pix_width;
    GrRect(dc, x, 80, 28, 120);
    GrLine(dc, 0, frame * 3 % Fs->pix_height,
          Fs->pix_width - 1, Fs->pix_height - 1);
    Sleep(4);
  }

  DCDel(dc);
}
Main;
