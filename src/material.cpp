#include "material.h"
#include "renderer.h"

void Material::reset() {
    program = 0;
    num_uniforms = 0;
    wireframe = 0;
    depth_test = 0;
    cull_face = 0;
    blending = 0;
}

void Material::bind() {
    glUseProgram(program);
    int tex_slot = 0;

    for (int i = 0; i < num_uniforms; i++) {
        uniform uni = uniforms[i];
        switch (uni.type) {
            case UNIFORM_TYPE_INT:
                glUniform1i(uni.location, uni.val.in);
                break;
            case UNIFORM_TYPE_FLOAT:
                glUniform1f(uni.location, uni.val.fl);
                break;
            case UNIFORM_TYPE_VEC2:
                glUniform2fv(uni.location, 1, &uni.val.v2[0]);
                break;
            case UNIFORM_TYPE_VEC3:
                glUniform3fv(uni.location, 1, &uni.val.v3[0]);
                break;
            case UNIFORM_TYPE_VEC4:
                glUniform4fv(uni.location, 1, &uni.val.v4[0]);
                break;
            case UNIFORM_TYPE_SAMPLER_2D:
                glUniform1i(uni.location, tex_slot);
                glActiveTexture(GL_TEXTURE0 + tex_slot);
                glBindTexture(GL_TEXTURE_2D, uni.val.in);
                tex_slot++;
                break;
        }
    }

    glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    (depth_test ? glEnable : glDisable)(GL_DEPTH_TEST);
    (cull_face ? glEnable : glDisable)(GL_CULL_FACE);
    (blending ? glEnable : glDisable)(GL_BLEND);
}

uniform *Material::findUniform(int location) {
    for (int i = 0; i < num_uniforms; i++) {
        if (uniforms[i].location == location) {
            return (uniforms + i);
        }
    }
    return 0;
}

void Material::setUniformValue(const char *name, uniform_type type, uniform_value val) {
    int location = glGetUniformLocation(program, name);

    if (location != -1) {
        uniform *uni = findUniform(location);
        if (!uni) {
            uni = &uniforms[num_uniforms];
            num_uniforms++;
        }
        uni->type = type;
        uni->location = location;
        uni->val = val;
    }
}

void Material::setInt(const char *name, int val) {
    uniform_value uni_val;
    uni_val.in = val;
    setUniformValue(name, UNIFORM_TYPE_INT, uni_val);
}

void Material::setFloat(const char *name, float val) {
    uniform_value uni_val;
    uni_val.fl = val;
    setUniformValue(name, UNIFORM_TYPE_FLOAT, uni_val);
}

void Material::setVec2(const char *name, glm::vec2 val) {
    uniform_value uni_val;
    uni_val.v2 = val;
    setUniformValue(name, UNIFORM_TYPE_VEC2, uni_val);
}

void Material::setVec3(const char *name, glm::vec3 val) {
    uniform_value uni_val;
    uni_val.v3 = val;
    setUniformValue(name, UNIFORM_TYPE_VEC3, uni_val);
}

void Material::setVec4(const char *name, glm::vec4 val) {
    uniform_value uni_val;
    uni_val.v4 = val;
    setUniformValue(name, UNIFORM_TYPE_VEC4, uni_val);
}

// NOTE: kinda sus going uint to int but I think its ok
void Material::setTexture(const char *name, uint32_t tex) {
    uniform_value uni_val;
    uni_val.in = tex;
    setUniformValue(name, UNIFORM_TYPE_SAMPLER_2D, uni_val);
}
