#pragma once
#include "renderer.h"

void materialReset(material *mat);
void materialBind(material mat);
uniform *findUniform(material *mat, int location);
void setUniformValue(material *mat, const char *name, uniform_type type, uniform_value val);
void materialSetInt(material *mat, const char *name, int val);
void materialSetFloat(material *mat, const char *name, float val);
void materialSetVec2(material *mat, const char *name, glm::vec2 val);
void materialSetVec3(material *mat, const char *name, glm::vec3 val);
void materialSetVec4(material *mat, const char *name, glm::vec4 val);
void materialSetTexture(material *mat, const char *name, uint32_t tex);
