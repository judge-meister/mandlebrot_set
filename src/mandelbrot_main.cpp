//////////////////////////////////////////////////////////////////////////////////////////
// mandelbrot_main program

#include <iostream>
#include <string>
//#include <chrono>

#include <unistd.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Window.h"
#include "Shader.h"
#include "Texture.h"
#include "ImageFile.h"
#include "MandelbrotAdapter.h"

// GLOBALS - for the time being
class Options
{
  public:
      int factor;
      int ScreenWidth;
      int ScreenHeight;
      char *real;
      char *imag;
};

// class instance pointers
Shader* mainShader = nullptr;
Shader* initShader = nullptr;
Shader* currentShader = nullptr;
Window* window = nullptr;
MandelbrotAdapter* mandAdapter = nullptr;
Texture* texture = nullptr;
ImageFile* image = nullptr;


Options options;

// ----------------------------------------------------------------------------
//
/*static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //glViewport(0, 0, ScreenWidth, ScreenHeight);
    //glViewport(0, 0, width, height);
    //ScreenWidth = width;
    //ScreenHeight = height;
    //printf("ScreenWidth %d ScreenHeight %d\n", width, height);
    int uniform_Mouse = glGetUniformLocation(shaderProgram, "mouse");
    //int uniform_Scale = glGetUniformLocation(shaderProgram, "scale");
    glUniform2f(uniform_Mouse, xpos, ypos);
    //glUniform1f(uniform_Scale, scale);
}*/

// ----------------------------------------------------------------------------
static void updateDisplay()
{
    unsigned char *pixels;
    mandAdapter->getTextureData(&pixels);
    texture->createTexture(&pixels);
    currentShader = mainShader;
    image->writeImage(mandAdapter->framecount(), pixels);
    free(pixels);
}
// ----------------------------------------------------------------------------
static void zoomIn(const double pX, const double pY)
{
    mandAdapter->zoomIn(pX, pY);
    updateDisplay();
}

// ----------------------------------------------------------------------------
static void zoomOut(double pX, double pY)
{
    mandAdapter->zoomOut();
    updateDisplay();
}
// ----------------------------------------------------------------------------
// react to the mouse clicks in the display
//
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    zoomIn(pX, pY);
  }

  if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    zoomOut(pX, pY);
  }
}
  
// ----------------------------------------------------------------------------
// respond to the keyboard
//
void processInput(GLFWwindow *window)
{
  if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { // 
    glfwSetWindowShouldClose(window, true);
  }
  if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) { // reset to start
    mandAdapter->reset();
    currentShader = initShader;
  }
  if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) // zoom out
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    zoomOut(pX, pY);
  }
  if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) // zoom in
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    zoomIn(pX, pY);
  }
  if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) { // move up
  }
  if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) { // move down
  }
  if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) { // move left
  }
  if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { // move right
  }
}

// ----------------------------------------------------------------------------
// create the shader programs
void createShaders()
{
    mainShader = new Shader();
    initShader = new Shader();
    mainShader->createVertexShaderFromFile("../shader/texshader.vert");
    mainShader->createFragmentShaderFromFile("../shader/texshader.frag");
    mainShader->createShaderProgram();
    
    initShader->createVertexShaderFromFile("../shader/mandelbrot.vert");
    initShader->createFragmentShaderFromFile("../shader/mandelbrot.frag");
    initShader->createShaderProgram();
}

// ----------------------------------------------------------------------------
// create the vertexbuffer, elementbuffer and vertexarray
unsigned int createVertexArray()
{         
    // create 2 triangles to cover all of the viewport
    float vertices[] = {
        // position           // color           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f,  // tr
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f,  // br
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f,  // bl
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f   // tl
    };  
    unsigned int indices[] = {
      0, 1, 3,
      1, 2, 3
    };

    // create VertexBuffer, VertexArray and ElementArray
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    glGenBuffers(1, &VBO);  
    glGenBuffers(1, &EBO);  
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBindVertexArray(0);
    return VAO;
} 
// ----------------------------------------------------------------------------
//
static void usage()
{
  printf("Usage: mandelbrot_main  \n");
  printf("   -h  this help\n");
  printf("   -z  zoom level to start with\n");
  printf("   -r  real value of point to zoom in to\n");
  printf("   -i  imaginary value of point to zoom in to\n");
  printf("   -d  display size (assumes a square)\n");
  printf("   -a  which algorithm (float, centre, mpfr, thread)\n");
  printf("   -f  zoom factor\n");
  exit(1);
}

// ----------------------------------------------------------------------------
// main method to create a window and run the event loop
//
static int run_shader()//const char* vert_shader, const char* frag_shader)//, int width, int height)
{
    window = Window::GetInstance("Mandelbrot Set", options.ScreenWidth, options.ScreenHeight);
    //window->setFramebufferSizeCB(framebuffer_size_callback);
    window->setMouseButtonCB(mouse_button_callback);

    int width, height;
    
    createShaders();  
    currentShader = initShader;

    unsigned int VAO = createVertexArray();
 
    texture = new Texture(options.ScreenWidth, options.ScreenHeight);
    image = new ImageFile(options.ScreenWidth, options.ScreenHeight);

    // initialise mandelbrot library
    mandAdapter = new MandelbrotAdapter(options.ScreenWidth, options.ScreenHeight, options.real, options.imag, options.factor);
    
    //window->eventLoop();
    while(!window->shouldClose())
    {
        // force the image to stay the same size
        glfwSetWindowSize(window->ptr(), options.ScreenWidth, options.ScreenHeight);
        
        // input - keyboard
        processInput(window->ptr());

        // rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind texture
        glBindTexture(GL_TEXTURE_2D, texture->texture());
        
        // draw stuff
        currentShader->useProgram();
        glfwGetWindowSize(window->ptr(), &width, &height);
        currentShader->uniformResolution(width, height);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // check and call events and swap the buffers
        glfwSwapBuffers(window->ptr());
        glfwPollEvents();
    }

    window->terminate();
    return 0;
}


enum { ALGO_MPFR, ALGO_THREAD };

// ----------------------------------------------------------------------------
// read the command line arguments
int main(int argc, char **argv)
{
  //int zoom = 0;
  //int algo = ALGO_THREAD;
  int index = 0;
  int c = 0;

  options.factor = 50; /* percentage zoom */
  options.ScreenWidth = 1024;
  options.ScreenHeight = 1024;
  options.real = NULL;
  options.imag = NULL;
  
  while((c = getopt(argc, argv, "hz:r:i:a:d:f:")) != -1)
  {
    switch (c)
    {
      case 'h':
        usage();
        break;
      case 'z': // zoom 
        printf("WARNING: zoom is yet to be implemented.");
        //zoom = atoi(optarg);
        break;
      case 'r': // real centre - string rep of a float
        options.real = optarg;
        break;
      case 'i': // imag centre - string rep of a float
        options.imag = optarg;
        break;
      case 'a': // algo
        printf("WARNING: algo is yet to be implemented.");
        /*if (strcmp(optarg, "mpfr") == 0) {
          algo = ALGO_MPFR; 
        }
        else if (strcmp(optarg, "thread") == 0) {
          algo = ALGO_THREAD; 
        }*/
        break;
      case 'd': // display size
        options.ScreenWidth = atoi(optarg);
        options.ScreenHeight = options.ScreenWidth;
        if (options.ScreenWidth < 128) { printf("Minimum display size is 128.\n"); return 0; }
        if (options.ScreenWidth % 16 != 0) { printf("Display size should be factor of 16.\n"); return 0; }
        break;
      case 'f': // zoom step factor
        options.factor = atoi(optarg);
        break;
      case '?':
        if ((optopt == 'f') || (optopt == 'v')) {
          fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        }
        else if (isprint (optopt)) {
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        }
        else {
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        }
        return 1;
      default:
        abort ();
    }
  }
  
  for (index = optind; index < argc; index++)
  {
    printf ("Non-option argument %s\n", argv[index]);
    return 1;
  }
  
  printf("real = %s\n",options.real);
  printf("imag = %s\n",options.imag);
  
  run_shader();
  
  mandAdapter->cleanUp(); 

  return 0;
}

