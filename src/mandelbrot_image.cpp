//////////////////////////////////////////////////////////////////////////////////////////
// mandelbrot_main program

#include <iostream>
#include <string>

#include <unistd.h>


#include "ImageData.h"
#include "ImageFile.h"
#include "CmdOptions.h"
#include "MandelbrotAdapter.h"

const int RGB = 3;

// ----------------------------------------------------------------------------
// main method to create a window and run the event loop
static void run_program(CmdOptions* cmdOptions)
{
    /*
     * while image quality has not bottomed out
     *   zoom to next level
     *   generate new image
     *   save image as png
     *   check quality of image
     *
     * mandAdapter->cleanup()
     */
    MandelbrotAdapter* mandAdapter = new MandelbrotAdapter(cmdOptions);
    ImageFile* imgFile = new ImageFile();
    ImageData* imgData = new ImageData(cmdOptions->getWidth(), cmdOptions->getHeight(), RGB);
    bool ok = true;

    // do first image
    mandAdapter->getTextureData(imgData);
    imgFile->writeImage(mandAdapter->framecount(), imgData);

    while (ok)
    {
        mandAdapter->zoomIn();
        mandAdapter->getTextureData(imgData);
        imgFile->writeImage(mandAdapter->framecount(), imgData);
        ok = imgData->isNotBottomedOut(mandAdapter->framecount());
    }
    mandAdapter->cleanUp();
}

// ----------------------------------------------------------------------------
// read the command line arguments and start the program
int main(int argc, char **argv)
{
  int status = 0;
  CmdOptions* cmdOptions = new CmdOptions();
  if ((status = cmdOptions->parseArgs(argc, argv)) != 0) {
    return status;
  }

  run_program(cmdOptions);
  
  return 0;
}
