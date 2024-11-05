#include "fwd.hpp"
#include "glm.hpp"
#include "ext/vector_float3.hpp"
#include "ext/vector_int3_sized.hpp"
#include <cstdint>

//==================================================
// ENGINE STRUCTS
//==================================================

typedef struct color {
    uint8_t r;        // Color red value
    uint8_t g;        // Color green value
    uint8_t b;        // Color blue value
    uint8_t a;        // Color alpha value
} color;

// TODO: remove anything that uses this and replace with glm::mat4
// Matrix, 4x4 components, column major, OpenGL style, right-handed
typedef struct matrix {
    float m0, m4, m8, m12;  // Matrix first row (4 components)
    float m1, m5, m9, m13;  // Matrix second row (4 components)
    float m2, m6, m10, m14; // Matrix third row (4 components)
    float m3, m7, m11, m15; // Matrix fourth row (4 components)
} matrix;

typedef struct mesh {
    int vertexCount;        // Number of vertices stored in arrays
    int triangleCount;      // Number of triangles stored (indexed or not)

    // Vertex attributes data
    float *vertices;        // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
    float *texcoords;       // Vertex texture coordinates (UV - 2 components per vertex) (shader-location = 1)
    float *texcoords2;      // Vertex texture second coordinates (UV - 2 components per vertex) (shader-location = 5)
    float *normals;         // Vertex normals (XYZ - 3 components per vertex) (shader-location = 2)
    float *tangents;        // Vertex tangents (XYZW - 4 components per vertex) (shader-location = 4)
    unsigned char *colors;      // Vertex colors (RGBA - 4 components per vertex) (shader-location = 3)
    unsigned short *indices;    // Vertex indices (in case vertex data comes indexed)
    
    // NOTE: I stole this from another codebase as a reference idk if we will do animations like this or not
    // Animation vertex data
    float *animVertices;    // Animated vertex positions (after bones transformations)
    float *animNormals;     // Animated normals (after bones transformations)
    unsigned char *boneIds; // Vertex bone ids, max 255 bone ids, up to 4 bones influence by vertex (skinning) (shader-location = 6)
    float *boneWeights;     // Vertex bone weight, up to 4 bones influence by vertex (skinning) (shader-location = 7)
    matrix *boneMatrices;   // Bones animated transformation matrices
    int boneCount;          // Number of bones

    // OpenGL identifiers
    unsigned int vaoId;     // OpenGL Vertex Array Object id
    unsigned int *vboId;    // OpenGL Vertex Buffer Objects id (default vertex data)
} mesh;

// Texture, tex data stored in GPU memory (VRAM)
typedef struct texture {
    unsigned int id;        // OpenGL texture id
    int width;              // Texture base width
    int height;             // Texture base height
    int mipmaps;            // Mipmap levels, 1 by default
    int format;             // Data format (PixelFormat type)
} texture;

typedef struct map_model {
    glm::mat4 transform;

    int meshCount;          // Number of meshes
    int textureCount;       // Number of textures
    mesh *meshes;           // Meshes array
    texture *textures;      // Textures array
} map_model;

// regular bbox uses float type vector
typedef struct bbox {
    glm::vec3 min, max;
} bbox;

// bbox with short type vector
typedef struct bbox_short {
    glm::i16vec3 min, max;
} bbox_short;

