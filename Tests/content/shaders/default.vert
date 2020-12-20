#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "uniforms_standard.incl"
#include "attributes_simple.vert.incl"

void main() {
    gl_Position = object.modelViewProjection * vec4(inPosition, 1.0);
    outFragColor = inColor;
}
