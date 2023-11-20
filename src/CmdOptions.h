//////////////////////////////////////////////////////////////////////////////////////////
// CmdOptions.h

// class to parse the command line arguments

#ifndef CMD_OPTIONS_H
#define CMD_OPTIONS_H

#include <string>

class CmdOptions {
  public:
    CmdOptions();
    ~CmdOptions();

    int parseArgs(int argc, char **argv);

    int getWidth() {return m_width;}
    int getHeight() {return m_height;}
    int getFactor() {return m_factor;}
    std::string& getReal() {return m_real;}
    std::string& getImag() {return m_imag;}

  private:
    void usage();

    int m_factor;
    int m_width;
    int m_height;
    std::string m_real;
    std::string m_imag;
};

#endif // CMD_OPTIONS_H
