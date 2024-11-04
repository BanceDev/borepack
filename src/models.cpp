#include "models.h"
#include <cstdlib>
#include "glad/glad.h"

void UploadMesh(mesh* m, bool dynamic) {
    // Generate a Vertex Array Object (VAO) to store mesh format
    glGenVertexArrays(1, &m->vaoId);
    glBindVertexArray(m->vaoId);

    // Allocate memory for VBOs for each attribute
    m->vboId = (unsigned int*)malloc(3 * sizeof(unsigned int));  // Assuming only 3 attributes: vertices, texcoords, normals
    glGenBuffers(3, m->vboId);

    // Upload vertex positions (XYZ) to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m->vboId[0]);
    glBufferData(GL_ARRAY_BUFFER, m->vertexCount * 3 * sizeof(float), m->vertices, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Upload texture coordinates (UV) to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m->vboId[1]);
    glBufferData(GL_ARRAY_BUFFER, m->vertexCount * 2 * sizeof(float), m->texcoords, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    // Upload normals (XYZ) to GPU
    glBindBuffer(GL_ARRAY_BUFFER, m->vboId[2]);
    glBufferData(GL_ARRAY_BUFFER, m->vertexCount * 3 * sizeof(float), m->normals, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // Unbind VAO and VBOs to avoid accidental modification
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

map_model LoadMapModelFromMesh(mesh input_mesh) {
    map_model model{};

    // Allocate space for one mesh and texture in the map model
    model.meshCount = 1;
    model.textureCount = 1;

    // Allocate memory for the mesh and textures array
    model.meshes = new mesh[model.meshCount];
    model.textures = new texture[model.textureCount];

    // Copy input mesh data into the model's mesh array
    model.meshes[0] = input_mesh;

    // Initialize the transform matrix to an identity matrix
    model.transform = matrix{
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };

    return model;
}
