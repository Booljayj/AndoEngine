#include "Rendering/Shader.h"
#include "Resources/RegisteredResource.h"

DEFINE_REFLECT_STRUCT(Rendering, Shader);
DEFINE_REFLECT_STRUCT(Rendering, VertexShader);
DEFINE_REFLECT_STRUCT(Rendering, FragmentShader);

REGISTER_RESOURCE(Rendering, VertexShader);
REGISTER_RESOURCE(Rendering, FragmentShader);
