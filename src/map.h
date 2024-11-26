#pragma once
#include "bsp.h"
#include "renderer.h"
#include "fwd.hpp"
#include "glad/glad.h"
#include "glm.hpp"
#include "camera.h"
#include "material.h"

#define MAP_MAX_SKY_TEXTURES 8


struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct surface {
    int32_t face;
    int32_t num_indices;
    int32_t first_index;
    glm::i16vec2 tex_mins;
    glm::i16vec2 uv_extents;
    glm::ivec2 lightmap_offset;
};

struct sky_texture {
    int32_t miptex;
    GLuint foreground;
    GLuint background;
};

struct render_group {
    int32_t num_faces;
    int32_t *faces;
    int32_t facetype;
};

struct map {
    int32_t num_meshes;
    mesh *meshes;

    GLuint program;
    GLuint *textures;
    GLuint lightmap_tex;

    char *ents;

    int num_materials;
    Material *materials;

    int32_t num_surfaces;
    surface *surfaces;

    int32_t num_planes;
    bsp_plane *planes;

    int32_t num_vertices;
    glm::vec3 *vertices;

    int32_t num_nodes;
    bsp_node *nodes;

    int32_t num_texinfos;
    bsp_texinfo *texinfos;

    int32_t num_faces;
    bsp_face *faces;

    int32_t num_clipnodes;
    bsp_clip_node *clipnodes;

    int32_t num_leafs;
    bsp_leaf *leafs;

    int32_t num_mark_surfaces;
    uint16_t *mark_surfaces;

    int32_t num_edges;
    bsp_edge *edges;

    int32_t num_surfedges;
    int16_t *surfedges;

    int32_t num_models;
    bsp_model *models;

    uint8_t *lightmap;

    bsp_miptex_lump *miptex_lump;
    bsp_miptex *miptex;
};

extern map loaded_map;

bool allocBlock(int width, int height, int *x, int *y);
void *loadBinaryFile(const char *filename);
uint32_t buildTexture(uint8_t *miptex_data, int width, int height, int offset, int pitch, color *palette, GLenum filter, color *pixel_buffer);

template <typename T>
void copyLump(bsp_header *header, int lump_type, T **out_items, int *out_num_items);
void mapInitBSP(bsp_header *header);
void mapInitMaterials();
void mapInitTextures();
void calcSurfaceExtents(surface *surf);
void markSurface(int leaf_idx);
void buildBSPTree(bsp_node node);
void createSurfaces();
int getVertexFromEdge(int surf_edge);
void triangulateSurface(surface *surf, uint32_t *triangle);
void mapInitMeshes();
void loadMap(const char *filename);
void drawMap(float time, Camera &cam);
char *getEntities();
void expandTreeCollisions(int nodeIndex, const glm::vec3& mins, const glm::vec3& maxs);
int findLeaf(const glm::vec3& position);
