#include "renderer.h"
#include "SDL_log.h"
#include "glm.hpp"
#include <SDL.h>

bool pointInsideViewFrustum(glm::vec3 point, const glm::mat4 &mvp) {
    glm::vec4 pointNDC = mvp * glm::vec4(point, 1.0f);
    pointNDC = pointNDC / pointNDC.w;
    for (int i = 0; i < 3; i++) {
        if (pointNDC[i] > 1.0f) {
            return false;
        }
    }
    return true;
}

// NOTE: Unused now but will be handy later
int aabbInsideViewFrustum(aabb bbox, const glm::mat4 &mvp) {
    glm::vec3 a = bbox.min + glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 b = bbox.min + glm::vec3(bbox.max.x, 0.0f, 0.0f);
    glm::vec3 c = bbox.min + glm::vec3(bbox.max.x, bbox.max.y, 0.0f);
    glm::vec3 d = bbox.min + glm::vec3(0.0f, bbox.max.y, 0.0f);
    glm::vec3 e = bbox.min + glm::vec3(0.0f, 0.0f, bbox.max.z);
    glm::vec3 f = bbox.min + glm::vec3(bbox.max.x, 0.0f, bbox.max.z);
    glm::vec3 g = bbox.min + glm::vec3(bbox.max.x, bbox.max.y, bbox.max.z);
    glm::vec3 h = bbox.min + glm::vec3(0.0f, bbox.max.y, bbox.max.z);

    if (pointInsideViewFrustum(a, mvp) ||
        pointInsideViewFrustum(b, mvp) || 
        pointInsideViewFrustum(c, mvp) || 
        pointInsideViewFrustum(d, mvp) || 
        pointInsideViewFrustum(e, mvp) || 
        pointInsideViewFrustum(f, mvp) || 
        pointInsideViewFrustum(g, mvp) || 
        pointInsideViewFrustum(h, mvp))
        return 1;
    return 0;
}

mesh createMesh(const vertex *verts, int num_verts, const uint32_t *index_data, int num_idx) {
    mesh m = {};
    m.num_verts = num_verts;
    m.topology = GL_TRIANGLE_FAN;

    glGenVertexArrays(1, &m.VAO);
    glBindVertexArray(m.VAO);
    glGenBuffers(1, &m.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex) * num_verts, verts, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void *)offsetof(vertex, pos));
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void *)offsetof(vertex, texcoord));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (const void *)offsetof(vertex, lightmap));

    if (num_idx > 0) {
        glGenBuffers(1, &m.EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * num_idx, index_data, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        m.num_indices = num_idx;
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m.bbox.min = glm::vec3(FLT_MAX);
    m.bbox.max = glm::vec3(-FLT_MAX);

    for (int i = 0; i < num_verts; i++) {
        for (int j = 0; j < 3; j++) {
            if (m.bbox.min[j] > verts[i].pos[j]) {
                m.bbox.min[j] = verts[i].pos[j];
            }
            if (m.bbox.max[j] < verts[i].pos[j]) {
                m.bbox.max[j] = verts[i].pos[j];
            }
        }
    }

    return m;
}

GLuint createTexture(const void *data, int width, int height, GLenum format, GLenum filter, GLenum wrap, int gen_mipmap) {
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, GL_NONE, format, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);

    if (gen_mipmap) {
        if (filter != GL_NEAREST) {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    return tex;
}

void updateTexture(uint32_t tex, int xoff, int yoff, int width, int height, int format, const void *pixels) {
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xoff, yoff, width, height, format, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint openGLCreateShaderProgram(const char *vert_code, const char *frag_code) {
    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_shader, 1, &vert_code, 0);
    glCompileShader(vert_shader);

    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_shader, 1, &frag_code, 0);
    glCompileShader(frag_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);

    glValidateProgram(program);
    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLsizei ignored;
        char vert_errs[2048];
        char frag_errs[2048];
        char prog_errs[2048];
        glGetShaderInfoLog(vert_shader, sizeof(vert_errs), &ignored, vert_errs);
        glGetShaderInfoLog(frag_shader, sizeof(frag_errs), &ignored, frag_errs);
        glGetProgramInfoLog(program, sizeof(prog_errs), &ignored, prog_errs);

        SDL_Log("[Vertex Shader]\n");
        SDL_Log("%s\n", vert_errs);
        SDL_Log("[Fragment Shader]\n");
        SDL_Log("%s\n", frag_errs);
        SDL_Log("[Program Errors]\n");
        SDL_Log("%s\n", prog_errs);

    }
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return program;
}

void meshDraw(mesh m) {
    glBindVertexArray(m.VAO);
    if (m.num_indices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
        glDrawElements(m.topology, m.num_indices, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(m.topology, 0, m.num_verts);
    }
}

void meshDrawIndexed(mesh m, int num_idx, uint64_t offset) {
    glBindVertexArray(m.VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m.EBO);
    glDrawElements(m.topology, num_idx, GL_UNSIGNED_INT, (const void *)offset);
}
