#version 330 core

layout (location = 0) in vec3 aPos;
out vec2 fragCoord;
out vec2 fCoord;

void main()
{
   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
   fragCoord = gl_Position.xy;
   fCoord = vec2(aPos.xy);
}

