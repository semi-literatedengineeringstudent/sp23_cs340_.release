#include "gif.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

unsigned int _log2(unsigned int x) {
  unsigned int ans = 0;
  while (x >>= 1) ans++;
  return ans;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage:\n  %s <gif file>\n", argv[0]);
    return 1;
  }

  const char *filename_in = argv[1];

  FILE *f = fopen(filename_in, "r");
  fseek(f, 0, SEEK_END);
  int len = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *buffer = malloc(len);
  fread(buffer, 1, len, f);
  fclose(f);

  char tempfile_name[] = "/tmp/gif-illinify-XXXXXX";
  int fd_out = mkstemp(tempfile_name);
  write(fd_out, buffer, len);
  close(fd_out);


  /* Transform the input filename into "{filename}.gif" => {filename}-illinify.gif" */
  char *filename_out = malloc(strlen(filename_in) + 11);
  strcpy(filename_out, filename_in);
  strcpy(filename_out + strlen(filename_in) - 4, "-illinify.gif");
  printf("Saving `%s` as `%s`\n", filename_in, filename_out);

  gd_GIF *gif_in;
  gif_in = gd_open_gif(tempfile_name);

  ge_GIF *gif_out;
  gif_out = ge_new_gif(
    filename_out,
    gif_in->width, gif_in->height,
    gif_in->palette->colors, _log2(gif_in->palette->size),
    -1, /* gif_in->bgindex, */
    gif_in->loop_count
  );

  int frame_ct = 0;
  while (1) {
    printf("Processing GIF Frame #%d\n", ++frame_ct);

    int ret = gd_get_frame(gif_in);
    if (ret != 1) { break; }

    memcpy(gif_out->frame, gif_in->frame, gif_in->width * gif_in->height);
    ge_add_frame(gif_out, gif_in->gce.delay);
  }

  gd_close_gif(gif_in);
  ge_close_gif(gif_out);

  return 0;
}