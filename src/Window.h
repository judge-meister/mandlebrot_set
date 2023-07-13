//////////////////////////////////////////////////////////////////////////////////////////
// Window.h
#ifndef WINDOW_H
#define WINDOW_H
 
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// there is only ever a single window so implement as a singleton

class Window {
  public:
    // singletons should not be cloneable or assignable
    Window(Window &other) = delete;
    void operator=(const Window&) = delete;
    
    static Window *GetInstance();
    static Window *GetInstance(const std::string title, const int width, const int height);
    
    int width() { return m_screenWidth; }
    int height() { return m_screenHeight; }
    void setTitle(const std::string title);
    
    void setFramebufferSizeCB(GLFWframebuffersizefun framebufferSizeCB);
    void setMouseButtonCB(GLFWmousebuttonfun mouseButtonCB);
    
    void processInput();
    void eventLoop();
    void terminate();
    bool shouldClose();
    
    GLFWwindow* ptr() { return m_window; }
    
  protected:
    Window();
    Window(const std::string title, const int width, const int height);
    virtual ~Window(){}

    // singleton reference
    static Window* instance;
    
  private:
    void create(const std::string title);
    
    int m_screenWidth;
    int m_screenHeight;
    GLFWwindow* m_window;
    
};

#endif /* WINDOW_H */
