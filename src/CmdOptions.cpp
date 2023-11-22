//////////////////////////////////////////////////////////////////////////////////////////
// CmdOptions.cpp

// class to parse the cmd line inputs

#include <unistd.h>
#include <iostream>
#include "CmdOptions.h"

CmdOptions::CmdOptions()
 : m_factor(50), m_width(1024), m_height(1024),
   m_real(""), m_imag("") 
{
}

CmdOptions::~CmdOptions()
{
}

//enum { ALGO_MPFR, ALGO_THREAD };

int CmdOptions::parseArgs(int argc, char **argv)
{
  //int zoom = 0;
  //int algo = ALGO_THREAD;
  int index = 0;
  int c = 0;
  
  while((c = getopt(argc, argv, "hz:r:i:a:d:f:")) != -1)
  {
    switch (c)
    {
      case 'h':
        usage();
        exit(1);
        break;
      case 'z': // zoom 
        std::cerr << "WARNING: zoom is yet to be implemented.\n";
        //zoom = atoi(optarg);
        break;
      case 'r': // real centre - string rep of a float
        m_real = std::string(optarg);
        break;
      case 'i': // imag centre - string rep of a float
        m_imag = std::string(optarg);
        break;
      case 'a': // algo
        std::cerr << "WARNING: algo is yet to be implemented.\n";
        /*if (strcmp(optarg, "mpfr") == 0) {
          algo = ALGO_MPFR; 
        }
        else if (strcmp(optarg, "thread") == 0) {
          algo = ALGO_THREAD; 
        }*/
        break;
      case 'd': // display size
        m_width = atoi(optarg);
        m_height = m_width;
        if (m_width < 128) { 
            std::cerr << "Minimum display size is 128.\n"; 
            return 0; 
        }
        if (m_width % 16 != 0) { 
            std::cerr << "Display size should be factor of 16.\n"; 
            return 0; 
        }
        break;
      case 'f': // zoom step factor
        m_factor = atoi(optarg);
        break;
      case '?':
        if ((optopt == 'f') || (optopt == 'v')) {
          std::cerr << "Option `-" << (char)optopt << "` requires an argument.\n";
        }
        else if (isprint (optopt)) {
          std::cerr << "Unknown option `-" << (char)optopt << "'.\n";
        }
        else {
          std::cerr << "Unknown option character `" << (char)optopt << "`\n";
        }
        return 1;
      default:
        abort ();
    }
  }
  
  for (index = optind; index < argc; index++)
  {
    std::cout << "Non-option argument " << argv[index] << "\n";
    return 1;
  }
  
  std::cout << "real = " << m_real.c_str() << std::endl;
  std::cout << "imag = " << m_imag.c_str() << std::endl;

  return 0;
}

void CmdOptions::usage()
{
  std::cout << "Usage: mandelbrot_main  \n";
  std::cout << "   -h  this help\n";
  std::cout << "   -z  zoom level to start with\n";
  std::cout << "   -r  real value of point to zoom in to\n";
  std::cout << "   -i  imaginary value of point to zoom in to\n";
  std::cout << "   -d  display size (assumes a square)\n";
  std::cout << "   -a  which algorithm (float, centre, mpfr, thread)\n";
  std::cout << "   -f  zoom factor\n";
}

