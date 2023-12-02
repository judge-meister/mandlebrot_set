// a class to support  

#define DEFAULT_PRECISION  512
#define DEFAULT_MAXITER   1000

#include "mpfr.h"

class MandelbrotMpfr
{
public:
    MandelbrotMpfr(const int precision, const int maxiter, const int zoom_factor)
     : m_maxIter(maxiter),
       m_zoom_factor(zoom_factor),
       zoom_level(0),
       ncpus(1)
     { 
        PRECISION = precision;
        if(PRECISION < DEFAULT_PRECISION) {
            PRECISION = DEFAULT_PRECISION;
        }
        if(m_maxIter < DEFAULT_MAXITER) {
            m_maxIter = DEFAULT_MAXITER;
        }
        ncpus = cpuCount();
        initialise_mpfr_vars();
     }
    ~MandelbrotMpfr(){}

    void mandelbrot_mpfr_c( 
                        const unsigned int xsize,   // width of screen/display/window 
                        const unsigned int ysize,   // height of screen/display/window 
                        unsigned char **bytearray); // reference/pointer to result list of color values 

    void zoom_out( );
    void zoom_in(       const unsigned int screen_width, 
                        const unsigned int screen_height); 
    void zoom_in_via_mouse(
                        const double mouse_x, 
                        const double mouse_y,
                        const unsigned int screen_width, 
                        const unsigned int screen_height); 
               
    void free_mpfr_mem_c();
    void initialize_c(  const char* Xs_str,  // string repr of mpfr_t for X top left 
                        const char* Xe_str,  // string repr of mpfr_t for X top right 
                        const char* Ys_str,  // string repr of mpfr_t for Y bottom left 
                        const char* Ye_str,  // string repr of mpfr_t for Y bottom right 
                        const char* Cx_str,  // string repr of centre X pos for zooming in 
                        const char* Cy_str); // string repr of centre Y pos for zooming in 
               
    static void* worker_process_slice(void* arg);

    // Setter and Getters
    void setMaxIter(const int maxiter) {m_maxIter = maxiter;}
    void setPrecision(const int precision) {PRECISION = precision;}
    const int getMaxIter() {return m_maxIter;}
    const int getPrecision() {return PRECISION;}
    const int getNcpus() {return ncpus;}

private:
    // Attributes
    int PRECISION;
    int m_maxIter;
    int m_zoom_factor;
    int zoom_level; // = 0;
    int ncpus; // = 1; // just to be safe 

    // mpfr vars
    mpfr_t Xe, Xs, Ye, Ys, Cx, Cy;       // algorithm values 
    mpfr_t MX, MY, Xe_Xs, Ye_Ys, CX, CY;

    // methods
    void initialise_mpfr_vars();
    void push_sq_back_into_bounds();
    int cpuCount();
};
