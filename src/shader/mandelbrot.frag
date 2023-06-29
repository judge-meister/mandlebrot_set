#version 330 core

#ifdef GL_ES
precision highp float;
#endif

in vec2 fCoord;
out vec4 FragColor;

uniform vec2 resolution;
uniform float time;
uniform vec2 mouse; // center
uniform float scale;

const int MAX_ITER = 1000;

float Xs = -2.0;
float Xe =  1.0;
float Ys = -1.5;
float Ye =  1.5;

vec4 normalise(int a, int b, int c)
{
  return vec4(float(a)/256.0, float(b)/256.0, float(c)/256.0, 1.0);
}

vec4 ultra_fractal_colors(int it)
{
  if (it < MAX_ITER && it > 0) 
  {
    int i = it % 16;
    vec4 mapping[16];
    mapping[0] = normalise(66, 30, 15);
    mapping[1] = normalise(25, 7, 26);
    mapping[2] = normalise(9, 1, 47);
    mapping[3] = normalise(4, 4, 73);
    mapping[4] = normalise(0, 7, 100);
    mapping[5] = normalise(12, 44, 138);
    mapping[6] = normalise(24, 82, 177);
    mapping[7] = normalise(57, 125, 209);
    mapping[8] = normalise(134, 181, 229);
    mapping[9] = normalise(211, 236, 248);
    mapping[10] = normalise(241, 233, 191);
    mapping[11] = normalise(248, 201, 95);
    mapping[12] = normalise(255, 170, 0);
    mapping[13] = normalise(204, 128, 0);
    mapping[14] = normalise(153, 87, 0);
    mapping[15] = normalise(106, 52, 3);
    return mapping[i];
  }
  else 
  {
    return vec4(0, 0, 0, 1);
  }
}

vec2 f(vec2 z, vec2 c) {
  return mat2(z,-z.y,z.x)*z + c;
}

void main() {
    //vec2 c = vec2((gl_FragCoord.x + 0.5) / resolution.x * (Xe-Xs) - 2.0, 
    //              (gl_FragCoord.y + 0.5) / resolution.y * (Ye-Ys) - 1.5);
    vec2 c = vec2(((gl_FragCoord.x / resolution.x) * (Xe-Xs)) - 2.0, 
                  ((gl_FragCoord.y / resolution.y) * (Ye-Ys)) - 1.5);
    vec2 z = vec2(0.0);
    
    //vec2 uv = gl_FragCoord.xy / resolution;
    
    int k = 0;

    for (int i = 0; i < MAX_ITER; i++) {
        z = f(z, c);
        if (length(z) > 2.0) {
            k = i;
            break;
        }
    }

    FragColor = ultra_fractal_colors(k);
}

