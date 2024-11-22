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

struct bsp_lump {
    int32_t offset;
    int32_t length;
};

struct bsp_header {
    int32_t version;
    bsp_lump lumps[BSP_LUMP_COUNT];
};
