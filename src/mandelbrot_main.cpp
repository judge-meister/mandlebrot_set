
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>

#include <cstring>
#include <unistd.h>

#include <SOIL/SOIL.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

extern "C" {
#include "mandelbrot.h"
}

static double scale = 1.0;
static double xpos = -0.5;
static double ypos = 0.0;
static int factor = 50; /* percentage zoom */
static int ScreenWidth = 1024;
static int ScreenHeight = 1024;
static unsigned int initshaderProgram = -1;
static unsigned int mainshaderProgram = -1;
static unsigned int shaderProgram = -1;
static int framecount = 1;
char *real = NULL;
char *imag = NULL;

unsigned int texture;

// ----------------------------------------------------------------------------
//
static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    ScreenWidth = width;
    ScreenHeight = height;
    printf("ScreenWidth %d ScreenHeight %d\n", width, height);
    int uniform_Mouse = glGetUniformLocation(shaderProgram, "mouse");
    int uniform_Scale = glGetUniformLocation(shaderProgram, "scale");
    glUniform2f(uniform_Mouse, xpos, ypos);
    glUniform1f(uniform_Scale, scale);
}  

// ----------------------------------------------------------------------------
//
static unsigned int createShaderFromFile(const std::string filename, 
                                         unsigned int shaderType, 
                                         const char* shaderTypeName)
{
  std::fstream newfile;
  std::string shaderSource;
  std::string line;
  int  success;
  const unsigned int logsize = 1024;
  char infoLog[logsize];

  newfile.open(filename, std::ios::in); // open a file to perform read operation using file object
  if (newfile.is_open()) {              // checking whether the file is open
    while(getline(newfile, line)) {     // read data from file object and put it into string.
         shaderSource += "\n" + line;
    }
    newfile.close();                    // close the file object.
  }
  //std::cout << shaderSource << "\n";
  unsigned int shader = glCreateShader(shaderType);
  char const *shaderSourcePtr = shaderSource.c_str();
  glShaderSource(shader, 1, &shaderSourcePtr, NULL);
  glCompileShader(shader);
  
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

  if(!success)
  {
    glGetShaderInfoLog(shader, logsize, NULL, infoLog);
    std::cout << "ERROR::SHADER::"<< shaderTypeName <<"::COMPILATION_FAILED\n" << infoLog << std::endl;
    exit(1);
  }

  return shader;
}

// ----------------------------------------------------------------------------
//
static unsigned int createShaderProgram(unsigned int vertexShader, unsigned int fragmentShader)
{
  int success;
  const unsigned int logsize = 1024;
  char infoLog[logsize];
  unsigned int shaderProg = glCreateProgram();

  glAttachShader(shaderProg, vertexShader);
  glAttachShader(shaderProg, fragmentShader);
  glLinkProgram(shaderProg);

  glGetProgramiv(shaderProg, GL_LINK_STATUS, &success);
  if(!success) 
  {
    glGetProgramInfoLog(shaderProg, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINK_FAILED\n" << infoLog << std::endl;
  }

  return shaderProg;
}

// ----------------------------------------------------------------------------
//
static unsigned int getShaderProgram(const char* vert_shader, const char* frag_shader)
{    
    // SHADER CODE COMPILATION - START
    unsigned int vertexShader = createShaderFromFile(vert_shader, GL_VERTEX_SHADER, "VERTEX");

    unsigned int fragmentShader = createShaderFromFile(frag_shader, GL_FRAGMENT_SHADER, "FRAGMENT");

    unsigned int shaderProg = createShaderProgram(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader); 
    // SHADER CODE COMPILATION - END

    return shaderProg;
}

// ----------------------------------------------------------------------------
//
static void write_image(unsigned int **rgb)
{
  unsigned char* image = NULL;
  char* filename = NULL;
  char* idx = NULL;
  
  filename = (char*)calloc((size_t)100, sizeof(char));
  idx = (char*)calloc((size_t)10, sizeof(char));
  image = (unsigned char*)calloc((size_t)(ScreenWidth * ScreenHeight * 3), sizeof(unsigned char));
  for(int i=0; i< (ScreenWidth * ScreenHeight * 3); i++)
  {
    image[i] = (*rgb)[i];
  }
  strcpy(filename, "images/image");
  sprintf(idx, "%04d", framecount);
  strcat(filename, idx);
  strcat(filename, ".bmp");
  (void)SOIL_save_image(filename, SOIL_SAVE_TYPE_BMP, ScreenWidth, ScreenHeight, 3, image);
  printf("Soil save image = %s\n", filename);
  
  free(filename);
  free(idx);
  free(image);
}

// ----------------------------------------------------------------------------
//
static void createTextureFromData()
{
  unsigned int *pixels = NULL;
  pixels = (unsigned int*)calloc((size_t)(ScreenWidth * ScreenHeight * 3), sizeof(unsigned int));

#ifdef USES_THREADS
  mandelbrot_mpfr_thread_c(ScreenWidth, ScreenHeight, 1000, &pixels);
#else
  mandelbrot_mpfr_c(ScreenWidth, ScreenHeight, 1000, &pixels);
#endif
  
  write_image(&pixels);
  unsigned char* image = NULL;
  image = (unsigned char*)calloc((size_t)(ScreenWidth * ScreenHeight * 3), sizeof(unsigned char));
  for(int i=0; i< (ScreenWidth * ScreenHeight * 3); i++)
  {
    image[i] = (pixels)[i];
  }

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ScreenWidth, ScreenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);

  free(image);
  free(pixels);
}

// ----------------------------------------------------------------------------
//
/*static void createTextureFromFile()
{
  int width, height;
  unsigned char* image = SOIL_load_image("texture.png", &width, &height, 0, SOIL_LOAD_RGB);

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
  glGenerateMipmap(GL_TEXTURE_2D);
  SOIL_free_image_data(image);
}*/

// ----------------------------------------------------------------------------
//
static void reset() 
{
  if ((real != NULL) && (imag != NULL)) {
    initialize_c("-2.0", "1.0", "-1.5", "1.5", real, imag);
    printf("Using provided zoom point\n");
  } else {
    //            Xs      Xe     Ys      Ye     Cx      Cy
    initialize_c("-2.0", "1.0", "-1.5", "1.5", "99.9", "99.9");
  }
}              

// ----------------------------------------------------------------------------
//
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) 
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    printf("Cursor Pos %f, %f factor %d  screen %d\n", pX, pY, factor, ScreenWidth);
    mpfr_zoom_in_via_mouse(pX, pY, ScreenWidth, ScreenHeight, factor);
    framecount++;
    createTextureFromData();
    shaderProgram = mainshaderProgram;
  }

  if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
  {
    double pX, pY;
    glfwGetCursorPos(window, &pX, &pY);
    printf("Cursor Pos %f, %f factor %d  screen %d\n", pX, pY, factor, ScreenWidth);
    mpfr_zoom_out(/*pX, pY, ScreenWidth, ScreenHeight,*/ factor);
    framecount--;
    if (framecount < 0) { framecount = 0; }
    createTextureFromData();
    shaderProgram = mainshaderProgram;
  }
}
  
// ----------------------------------------------------------------------------
//
void processInput(GLFWwindow *window)
{
   if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { // 
     glfwSetWindowShouldClose(window, true);
   }
   if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) { // reset to start
     reset();
     printf("Cursor Pos %d, %d factor %d  screen %d\n", ScreenWidth/2, ScreenHeight/2, factor, ScreenWidth);
     framecount = 1;
     //createTextureFromData();
     shaderProgram = initshaderProgram;
   }
   if(glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) { // zoom out
     double pX, pY;
     glfwGetCursorPos(window, &pX, &pY);
     printf("Cursor Pos %f, %f factor %d  screen %d\n", pX, pY, factor, ScreenWidth);
     mpfr_zoom_out(/*pX, pY, ScreenWidth, ScreenHeight,*/ factor);
     framecount--;
     if (framecount < 0) { framecount = 0; }
     createTextureFromData();
     shaderProgram = mainshaderProgram;
   }
   if(glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) { // zoom in
     double pX, pY;
     glfwGetCursorPos(window, &pX, &pY);
     printf("Cursor Pos %f, %f factor %d  screen %d\n", pX, pY, factor, ScreenWidth);
     if ((real == NULL) && (imag == NULL))
     {
       mpfr_zoom_in_via_mouse(pX, pY, ScreenWidth, ScreenHeight, factor);
     }
     else
     {
       mpfr_zoom_in(ScreenWidth, ScreenHeight, factor);
     }
     framecount++;
     createTextureFromData();
     shaderProgram = mainshaderProgram;
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
//
static void createWindow(GLFWwindow** window, int width, int height)
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  *window = glfwCreateWindow(width, height, "Mandelbrot Set", NULL, NULL);
  if (*window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    exit(1);
  }
  glfwMakeContextCurrent(*window);
  glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);
  glfwSetMouseButtonCallback(*window, mouse_button_callback);
  //glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    exit(1);
  }   
}

// ----------------------------------------------------------------------------
//
static void usage()
{
  printf("Usage: mandelbrot_main  \n");
  printf("   -h  this help");
  printf("   -z  zoom level to start with");
  printf("   -r  real value of point to zoom in to");
  printf("   -i  imaginary value of point to zoom in to");
  printf("   -d  display size (assumes a square)");
  printf("   -a  which algorithm (float, centre, mpfr, thread)");
  printf("   -f  zoom factor");
  exit(1);
}

// ----------------------------------------------------------------------------
//
static int run_shader(const char* vert_shader, const char* frag_shader)//, int width, int height)
{
    //int ftt = 0;
    //auto start_time = std::chrono::high_resolution_clock::now();

    GLFWwindow *window;
    createWindow(&window, ScreenWidth, ScreenHeight);
    int width, height;
    //ScreenWidth = width;
    //ScreenHeight = height;
    
    mainshaderProgram = getShaderProgram("shader/texshader.vert", "shader/texshader.frag");
    initshaderProgram = getShaderProgram("shader/mandelbrot.vert", "shader/mandelbrot.frag");

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

    shaderProgram = initshaderProgram;

    int uniform_Resolution = glGetUniformLocation(shaderProgram, "resolution");
    //int uniform_Time = glGetUniformLocation(shaderProgram, "time");
    
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

    // mandelbrot library
    setup_c();
    reset(); // initialize the starting parameters
    
    //createTextureFromData(); // calls mpfr threaded mandelbrot set calc
    
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

    glBindVertexArray(0);

    while(!glfwWindowShouldClose(window))
    {
        // input - keyboard
        processInput(window);

        // rendering commands here
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind texture
        glBindTexture(GL_TEXTURE_2D, texture);
        
        // draw stuff
        glUseProgram(shaderProgram);
        glfwGetWindowSize(window, &width, &height);
        glUniform2f(uniform_Resolution, width, height);

        //auto current_time = std::chrono::high_resolution_clock::now();
        //double time_taken = std::chrono::duration_cast<std::chrono::nanoseconds>(current_time - start_time).count();
        //glUniform1f(uniform_Time, time_taken * 1e-9);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
        
        //if(ftt == 0) {
        //  shaderProgram = shaderProgram1;
        //  ftt=1;
        //}
    }

    glfwTerminate();
    return 0;
}

// ----------------------------------------------------------------------------
inline bool file_exists (const std::string& name) {
    std::ifstream f(name.c_str());
    return f.good();
}

enum { ALGO_FLOAT, ALGO_MPFR, ALGO_CENTRE, ALGO_THREAD };

// ----------------------------------------------------------------------------
//
int main(int argc, char **argv)
{
  //int zoom = 0;
  //int algo = ALGO_THREAD;
  //int width = ScreenWidth;
  //int height = ScreenHeight;
  int index = 0;
  int c = 0;
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
        real = optarg;
        break;
      case 'i': // imag centre - string rep of a float
        imag = optarg;
        break;
      case 'a': // algo
        printf("WARNING: algo is yet to be implemented.");
        /*if (strcmp(optarg, "float") == 0) {
          algo = ALGO_FLOAT; 
        }
        else if (strcmp(optarg, "mpfr") == 0) {
          algo = ALGO_MPFR; 
        }
        else if (strcmp(optarg, "centre") == 0) {
          algo = ALGO_CENTRE; 
        }
        else if (strcmp(optarg, "thread") == 0) {
          algo = ALGO_THREAD; 
        }*/
        break;
      case 'd': // display size
        ScreenWidth = atoi(optarg);
        ScreenHeight = ScreenWidth;
        if (ScreenWidth < 128) { printf("Minimum display size is 128.\n"); return 0; }
        if (ScreenWidth % 16 != 0) { printf("Display size should be factor of 16.\n"); return 0; }
        break;
      case 'f': // zoom step factor
        factor = atoi(optarg);
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

  char* vert = NULL;
  if (vert == NULL) {
    vert = strdup("shader/texshader.vert");
  }
  else {
    if (!file_exists(vert)) {
      printf("Vertex Shader '%s' doesn't exist.\n", vert);
      return 1;
    }
  }
 
  char* frag = NULL;
  if (frag == NULL) {
    frag = strdup("shader/texshader.frag");
  }
  else {
    if (!file_exists(frag)) {
      printf("Fragment Shader '%s' doesn't exist.\n", frag);
      return 1;
    }
  }
  
  printf("Using Fragment Shader '%s' and Vertex Shader '%s'\n", frag, vert);
  
  for (index = optind; index < argc; index++)
  {
    printf ("Non-option argument %s\n", argv[index]);
    return 1;
  }
  
  printf("real = %s\n",real);
  printf("imag = %s\n",imag);
  
  run_shader(vert, frag);//, ScreenWidth, ScreenHeight);
  
  free(vert);
  free(frag);
  free_mpfr_mem_c();

  return 0;
}

