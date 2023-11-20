//////////////////////////////////////////////////////////////////////////////////////////
// mandelbrot_main program

#include <iostream>
#include <string>

#include <unistd.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "CmdOptions.h"
#include "KeyboardMouseHandler.h"
#include "MandelbrotWindow.h"
#include "MandelbrotOpenGL.h"


// ----------------------------------------------------------------------------
// main method to create a window and run the event loop
static void run_program(CmdOptions* cmdOptions)
{
    MandelbrotWindow* mandWindow = new MandelbrotWindow();
    mandWindow->initialise(cmdOptions);

    KeyboardMouseHandler::setMandWindow(mandWindow);

    mandWindow->getWindow()->setMouseButtonCB(KeyboardMouseHandler::mouseButtonCB);

    MandelbrotOpenGL* mandOpenGL = new MandelbrotOpenGL();
    mandOpenGL->createVertexArray();
    
    while(!mandWindow->getWindow()->shouldClose())
    {
        int width, height;

        // force the image to stay the same size
        glfwSetWindowSize(mandWindow->getWindow()->ptr(), cmdOptions->getWidth(), cmdOptions->getHeight());
        
        // input - keyboard
        KeyboardMouseHandler::processKeyboardInput(mandWindow->getWindow()->ptr());

        // rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind texture
        glBindTexture(GL_TEXTURE_2D, mandWindow->getTexture()->texture());
        
        // draw stuff
        mandWindow->getCurrentShader()->useProgram();
        glfwGetWindowSize(mandWindow->getWindow()->ptr(), &width, &height);
        mandWindow->getCurrentShader()->uniformResolution(width, height);

        //glBindVertexArray(VAO);
        glBindVertexArray(mandOpenGL->getVertexArray());
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // check and call events and swap the buffers
        glfwSwapBuffers(mandWindow->getWindow()->ptr());
        glfwPollEvents();
    }

    mandWindow->getWindow()->terminate();
    mandWindow->getMandAdapter()->cleanUp();
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

