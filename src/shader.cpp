#include <cstdint>
#include <cstring>
#include <fstream>
#include <sstream>
#include "glm.hpp"

#define SHADER_STORAGE_MAX 32

struct shader_storage_entry {
    char name[32];
    uint32_t program;
};

static int32_t num_storage_entries;
static shader_storage_entry shader_storage[SHADER_STORAGE_MAX];

static void addShaderToStorage(const char *name, uint32_t program) {
    if (num_storage_entries < SHADER_STORAGE_MAX) {
        strcpy(shader_storage[num_storage_entries].name, name);
        shader_storage[num_storage_entries].program = program;
        num_storage_entries++;
    }
}

static shader_storage_entry *findShader(const char *name) {
    for (int i = 0; i < num_storage_entries; i++) {
        shader_storage_entry *entry = shader_storage + i;
        if (strcmp(name, entry->name) == 0) {
            return entry;
        }
    }
    return 0;
}

uint32_t getShader(const char *name) {
    shader_storage_entry *entry = findShader(name);
    if (entry) {
        return entry->program;
    }
    return 0;
}

uint32_t loadShader(const char *filename, const char *name) {
    shader_storage_entry *entry = findShader(name);

    if (entry) {
        return entry->program;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        return 0;
    }

    std::string line;
    std::ostringstream vertexShaderCode, fragmentShaderCode;
    std::ostringstream *currentShaderCode = nullptr;

    while (std::getline(file, line)) {
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (line == "#vertex") {
            currentShaderCode = &vertexShaderCode;
        } else if (line == "#fragment") {
            currentShaderCode = &fragmentShaderCode;
        } else if (currentShaderCode) {
            *currentShaderCode << line << "\n";
        }
    }

    file.close();
    uint32_t program = openGLCreateShaderProgram(vertexShaderCode.str().c_str(), fragmentShaderCode.str().c_str());
    addShaderToStorage(name, program);
    return program;
}
