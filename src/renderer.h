#pragma once
#include "glm.hpp"

#define MATERIAL_MAX_UNIFORMS 8
#define MATERIAL_MAX_TEXTURES 4

struct plane {
    float distance;
    glm::vec3 normal;
};

struct aabb {
    glm::vec3 min;
    glm::vec3 max;
};

struct vertex {
    glm::vec3 pos;
    glm::vec2 texcoord;
    glm::vec2 lightmap;
};

struct mesh {
    uint32_t VAO;
    uint32_t VBO;
    uint32_t EBO;
    int32_t num_verts;
    int32_t num_indices;
    int32_t topology;
    int32_t material_index;
    aabb bbox;
};

enum uniform_type {
    UNIFORM_TYPE_INT,
    UNIFORM_TYPE_FLOAT,
    UNIFORM_TYPE_VEC2,
    UNIFORM_TYPE_VEC3,
    UNIFORM_TYPE_VEC4,
    UNIFORM_TYPE_SAMPLER_2D
};

union uniform_value {
    int32_t in;
    float fl;
    glm::vec2 v2;
    glm::vec3 v3;
    glm::vec4 v4;
};

struct uniform {
    int32_t location;
    uniform_type type;
    uniform_value val;
};

struct material {
    uint32_t program;
    int32_t num_uniforms;
    uniform uniforms[MATERIAL_MAX_UNIFORMS];
    int32_t wireframe;
    int32_t depth_test;
    int32_t cull_face;
    int32_t blending;
};
