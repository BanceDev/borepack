#include "material.h"
#include "renderer.h"

void materialReset(material *mat) {
    *mat = {};
}

void materialBind(material mat) {
    glUseProgram(mat.program);
    int tex_slot = 0;

    for (int i = 0; i < mat.num_uniforms; i++) {
        uniform uni = mat.uniforms[i];
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

    glPolygonMode(GL_FRONT_AND_BACK, mat.wireframe ? GL_LINE : GL_FILL);
    (mat.depth_test ? glEnable : glDisable)(GL_DEPTH_TEST);
    (mat.cull_face ? glEnable : glDisable)(GL_CULL_FACE);
    (mat.blending ? glEnable : glDisable)(GL_BLEND);
}

uniform *findUniform(material *mat, int location) {
    for (int i = 0; i < mat->num_uniforms; i++) {
        if (mat->uniforms[i].location == location) {
            return (mat->uniforms + i);
        }
    }
    return 0;
}

void setUniformValue(material *mat, const char *name, uniform_type type, uniform_value val) {
    int location = glGetUniformLocation(mat->program, name);

    if (location != -1) {
        uniform *uni = findUniform(mat, location);
        if (!uni) {
            uni = &mat->uniforms[mat->num_uniforms];
            mat->num_uniforms++;
        }
        uni->type = type;
        uni->location = location;
        uni->val = val;
    }
}

void materialSetInt(material *mat, const char *name, int val) {
    uniform_value uni_val;
    uni_val.in = val;
    setUniformValue(mat, name, UNIFORM_TYPE_INT, uni_val);
}

void materialSetFloat(material *mat, const char *name, float val) {
    uniform_value uni_val;
    uni_val.fl = val;
    setUniformValue(mat, name, UNIFORM_TYPE_FLOAT, uni_val);
}

void materialSetVec2(material *mat, const char *name, glm::vec2 val) {
    uniform_value uni_val;
    uni_val.v2 = val;
    setUniformValue(mat, name, UNIFORM_TYPE_VEC2, uni_val);
}

void materialSetVec3(material *mat, const char *name, glm::vec3 val) {
    uniform_value uni_val;
    uni_val.v3 = val;
    setUniformValue(mat, name, UNIFORM_TYPE_VEC3, uni_val);
}

void materialSetVec4(material *mat, const char *name, glm::vec4 val) {
    uniform_value uni_val;
    uni_val.v4 = val;
    setUniformValue(mat, name, UNIFORM_TYPE_VEC4, uni_val);
}

// NOTE: kinda sus going uint to int but I think its ok
void materialSetTexture(material *mat, const char *name, uint32_t tex) {
    uniform_value uni_val;
    uni_val.in = tex;
    setUniformValue(mat, name, UNIFORM_TYPE_SAMPLER_2D, uni_val);
}
