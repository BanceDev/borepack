#pragma once
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <istream>
#include <filesystem>
#include <span>
#include "ext/vector_float3.hpp"
#include "fwd.hpp"

#include "models.h"

//==================================================
// BSPFILE STRUCTS
//==================================================

struct color_rgb8 {
    uint8_t r, g, b;
};

// function prototype implemented at end
color_rgb8 palette(uint8_t id);

struct dir_entry {
    int32_t offset;
    int32_t size;
};

// specification for bsp file so that we know where all the data is
struct bsp_header {
    int32_t version; // must be equal to 0x17 (23) to be valid

    dir_entry entities; // list of entities
    dir_entry planes; // list of map plantes

    dir_entry miptex; // list of wall textures with 4 level mipmap
    dir_entry verticies; // list of map geometry verts

    dir_entry visibility; // leaves vis list
    dir_entry nodes; // bsp tree nodes

    dir_entry texinfos; // texture info for faces
    dir_entry faces; // faces of each surface

    dir_entry lightmaps; // wall light maps (probably will go unused for us)
    dir_entry clipnodes; // clip nodes, for models

    dir_entry leaves; // list of bsp leaves

    dir_entry listfaces; // list of faces
    dir_entry edges; // edges of face

    dir_entry listedges; // list of edges
    dir_entry models; // list of models
};

// each entity def is just a list of key value pairs
struct entity {
    std::unordered_map<std::string, std::string> tags;
};

struct bsp_model {
    bbox bound;
    glm::vec3 origin; // origin of model usually 0 0 0
    int32_t bsp_node_id; // index of first BSP node
    int32_t clipnode1_id; // index of first clip node
    int32_t clipnode2_id; // index of second clip node
    int32_t _dummy_id; // unused value usually 0
    int32_t numleafs; // number of bsp leaves
    int32_t face_id; // index of faces
    int32_t face_num; // number of faces
};

struct edge {
    uint16_t vs; // start vertex
    uint16_t ve; // end vertex
};

struct plane {
    glm::vec3 normal; // the orthogonal vector to plane
    float dist; // offset to plane along normal vector
    int32_t type; // type of plane, depending on normal vector
};

struct tex_info {
    glm::vec3 u_axis; // U vector, horizontal in texture space
    float u_offset; // horizontal offset in texture space
    glm::vec3 v_axis; // V vector, vertical in texture space
    float v_offset; // vertical offset in texture space
    uint32_t miptex_id; // index of mip texture
    // NOTE: We can expand this by adding as many as we want thru the mapper
    uint32_t animated; // 0 for normal, 1 for water
};

struct face {
    uint16_t plane_id; // plane that the face lies on
    uint16_t side; // 0 if drawn on front of plane 1 if behind
    int32_t ledge_id; // first edge in the list of edges

    uint16_t ledge_num; // number of edges in the list of edges
    uint16_t texinfo_id; // index of the texture info the face is part of

    // NOTE: typelight is implemented by the Quake engine, we could choose to
    // make implementations as well or do something new with this field
    uint8_t typelight; // type of lighting
    uint8_t baselight; // from 0xFF to 0x00
    uint8_t light[2]; // two additional light models (not sure what use is)
    uint32_t lightmap; // pointer inside the general light map, or -1
};

struct miptex {
    // NOTE: We will likely just yoink the name and replace with a higher quality
    // png texture instead of the RGB indexed color mode of Quake
    char name[16];  // texture name

    // dimensions must be multiple of 8 or mipmapping alg will shit itself
    uint32_t width;
    uint32_t height;
    uint32_t offset[4]; // offsets for where the mipmap level data is
};

// NOTE: the way the node works is a bit odd for leaf detection
// if you have questions on this just ask me cuz it requires some understanding
// of binary math to get why it is this way
struct node {
    uint32_t plane_id;  // the plane that splits the node
    int16_t front;      // if > 0, front = index of front child node
                        // else, ~front = index of child leaf
    int16_t back;       // if > 0, back = index of back child node
                        // else, ~back = index of child leaf
    bbox_short box;     // bounding box of node and its children
    uint16_t face_id;   // index of first polygons in the node
    uint16_t face_num;  // number of faces in the node
};

struct leaf {
    int32_t type;       // special type for the leaf
    int32_t vis_id;     // beginning of vis lists

    bbox_short bound;   // bbox of the leaf
    uint16_t listface_id;

    uint16_t listface_num;  // number of faces in the leaf
    uint8_t sndwater;   // level of the four ambient sounds:
    uint8_t sndsky;     // 0 is silent 0xFF is max
    uint8_t sndslime;   //
    uint8_t sndlava;    // obv quake specific but could be repurposed for our engine
};

struct clipnode {
    uint32_t planenum;  // the plane that splits the node
    int16_t front;      // if pos, id of front child node
                        // if -2, the front part is inside the model
                        // if -1, the front part is outside the model
    int16_t back;      // if pos, id of back child node
                        // if -2, the back part is inside the model
                        // if -1, the back part is outside the model
};

// if pragma pack is needed I would pop here


class BSPFile {
public:
    entity readEntity(std::istream &stream);
    glm::vec3 fromQuake(glm::vec3 quakeVec);
    glm::vec3 verticesNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c);
    std::istream &bsp_file;
    bsp_header header;
    BSPFile(std::ifstream &file);
    template<typename T> T read(dir_entry dir, size_t idx);
    std::vector<entity> entities();
    plane getPlane(size_t idx);
    int32_t miptexCount();
    miptex getMiptex(size_t idx);
    glm::vec3 getVertex(size_t idx);
    node getNode(size_t idx);
    tex_info getTexInfo(size_t idx);
    face getFace(size_t idx);
    leaf getLeaf(size_t idx);
    uint16_t getListFace(size_t idx);
    edge getEdge(size_t idx);
    int32_t getListEdge(size_t idx);
    bsp_model getModel(size_t idx);
    std::vector<color_rgb8> getMiptexData(size_t idx, uint8_t miplevel);
    mesh genMeshFaces(std::span<const face> faces);
};

// map loader function
std::vector<map_model> loadModelsFromBSPFile(const std::filesystem::path &path);
