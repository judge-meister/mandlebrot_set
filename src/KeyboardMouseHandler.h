//////////////////////////////////////////////////////////////////////////////////////////
// KeyboardMouseHandler.h

// class to handle key presses and mouse buttons

#ifndef KEYBOARD_MOUSE_HANDLER_H
#define KEYBOARD_MOUSE_HANDLER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "MandelbrotWindow.h"

class KeyboardMouseHandler {
  public:

    static void setMandWindow(MandelbrotWindow* mandWindow);
    static void mouseButtonCB(GLFWwindow* window, int button, int action, int mods);
    static void processKeyboardInput(GLFWwindow *window);

  private:
    static MandelbrotWindow* m_mandWindow;

    KeyboardMouseHandler(){};
    ~KeyboardMouseHandler(){};

};

#endif // KEYBOARD_MOUSE_HANDLER_H
