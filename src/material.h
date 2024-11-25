#pragma once
#include "renderer.h"

class Material {
public:
    void reset();
    void bind();
    uniform *findUniform(int location);
    void setUniformValue(const char *name, uniform_type type, uniform_value val);
    void setInt(const char *name, int val);
    void setFloat(const char *name, float val);
    void setVec2(const char *name, glm::vec2 val);
    void setVec3(const char *name, glm::vec3 val);
    void setVec4(const char *name, glm::vec4 val);
    void setTexture(const char *name, uint32_t tex);

    uint32_t program;
    int32_t num_uniforms;
    uniform uniforms[MATERIAL_MAX_UNIFORMS];
    int32_t wireframe;
    int32_t depth_test;
    int32_t cull_face;
    int32_t blending;
private:
};
