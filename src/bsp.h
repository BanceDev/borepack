#pragma once
#include "glm.hpp"

#define BSP_LUMP_ENTITIES       0
#define BSP_LUMP_PLANES         1
#define BSP_LUMP_MIPTEX         2
#define BSP_LUMP_VERTICES       3
#define BSP_LUMP_VISIBILITY     4
#define BSP_LUMP_NODES          5
#define BSP_LUMP_TEXINFO        6
#define BSP_LUMP_FACES          7
#define BSP_LUMP_LIGHTMAPS      8
#define BSP_LUMP_CLIPNODES      9
#define BSP_LUMP_LEAFS          10
#define BSP_LUMP_MARKSURFACES   11  // List of faces
#define BSP_LUMP_EDGES          12
#define BSP_LUMP_SURFEDGES      13
#define BSP_LUMP_MODELS         14

#define BSP_LUMP_COUNT          15

#define MIPLEVELS 4

#define PLANE_X    0
#define PLANE_Y    1
#define PLANE_Z    2
#define PLANE_ANYX 3
#define PLANE_ANYY 4
#define PLANE_ANYZ 5

#define TEX_SPECIAL 1 // sky or slime, no lightmap
#define TEX_MISSING 2 // this texinfo has no texture

#define MAXLIGHTMAPS 4

struct bsp_lump {
    int32_t offset;
    int32_t length;
};

struct bsp_header {
    int32_t version;
    bsp_lump lumps[BSP_LUMP_COUNT];
};

struct bsp_model {
    glm::vec3 min;
    glm::vec3 max;
    glm::vec3 origin;
    int32_t head_nodes[4];
    int32_t visleafs;
    int32_t first_face;
    int32_t num_faces;
};

struct bsp_miptex_lump {
    int32_t miptex_count;
    int32_t data_offset[4];
};

struct bsp_miptex {
    char name[16];
    uint32_t width;
    uint32_t height;
    uint32_t offsets[MIPLEVELS];
};

struct bsp_plane {
    glm::vec3 normal;
    float distance;
    int32_t type;
};

struct bsp_node {
    int32_t plane;
    int16_t children[2];
    int16_t min[3];
    int16_t max[3];
    uint16_t first_face;
    uint16_t face_count;
};

struct bsp_clip_node {
    int32_t plane;
    int32_t children[2];
};

struct bsp_texinfo {
    glm::vec3 uaxis;
    float uoffset;
    glm::vec3 vaxis;
    float voffset;
    int32_t miptex;
    int32_t flags;
};

typedef uint16_t bsp_edge[2];

struct bsp_face {
    int16_t plane;
    int16_t side;
    int32_t first_edge;
    int16_t edge_count;
    int16_t texinfo;
    uint8_t styles[MAXLIGHTMAPS];
    int32_t light_offset;
};

struct bsp_leaf {
    int32_t contents;
    int32_t visoffset;
    int16_t min[3];
    int16_t max[3];
    uint16_t first_mark_surface;
    uint16_t mark_surface_count;
    uint8_t ambient_level[4];
};
