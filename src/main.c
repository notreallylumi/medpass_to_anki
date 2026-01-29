#include <leptonica/allheaders.h>
#include <raylib.h>
#include <stdio.h>
#include <tesseract/capi.h>

#define TEST_IMAGE "/home/lumi/code/medpass_to_anki/build/bin/test.png"

int main(void) {
  TessBaseAPI *api = TessBaseAPICreate();
  if (TessBaseAPIInit3(api, NULL, "eng") != 0) {
    return 1;
  }
  PIX *image = pixRead(TEST_IMAGE);
  if (!image) {
    return 1;
  }
  TessBaseAPISetImage2(api, image);
  char *text = TessBaseAPIGetUTF8Text(api);

  printf("%s\n", text);

  pixDestroy(&image);
  TessBaseAPIEnd(api);
  TessBaseAPIDelete(api);
}
