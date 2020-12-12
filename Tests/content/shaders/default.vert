#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "attributes_simple.vert.incl"

void main() {
    gl_Position = vec4(inPosition, 1.0);
    outFragColor = inColor;
}
