#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <iterator>
#include <ostream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <set>
#include <span>
#include <filesystem>
#include "ext/vector_float3.hpp"
#include "fwd.hpp"
#include "geometric.hpp"

#include "bsp.h"
#include "glad/glad.h"
#include "models.h"

//==================================================
// FILE READING
//==================================================

// generic
template<typename T> T ReadT(std::istream &stream) {
    T data{};
    stream.read((char *)&data, sizeof(T));
    return data;
}

// indexed
template<typename T> T ReadT(std::istream &stream, size_t idx) {
    stream.seekg(idx * sizeof(T), std::ios::cur);
    return ReadT<T>(stream);
}

// TODO: check if I need a pragma pack here

//==================================================
// CONSTRUCTOR
//==================================================

BSPFile::BSPFile(std::ifstream &file) : bsp_file(file) {
    if (bsp_file.good() == false) {
        throw std::runtime_error("File opening failed!");
    }

    header = ReadT<bsp_header>(bsp_file);
}

//==================================================
// PRIVATE MEMBERS
//==================================================

// an entity looks smth like this
/*
 * {
 * "key1" "value1"
 * "key2" "value2"
 * }
*/
entity BSPFile::readEntity(std::istream &stream) {
    entity ent{};

    char token;
    stream >> token;
    if (token != '{')
        std::cerr << "Expected '{', found " << token << std::endl;
    
    // discard leading whitespace
    while (stream >> std::ws) {
        token = stream.peek();
        if (token == '"') {
            std::string tag, tagVal;
            stream >> std::quoted(tag) >> std::quoted(tagVal);
            ent.tags[tag] = tagVal;
        } else if (token == '}') {
            stream >> token;
            break;
        } else {
            std::cerr << "Expected '}', found " << token << std::endl;
        }
    }

    return ent;
}

// convert a quake vector to a useful vector for our engine
glm::vec3 BSPFile::fromQuake(glm::vec3 quakeVec) {
    return quakeVec * 0.05f;
}

glm::vec3 BSPFile::verticesNormal(glm::vec3 a, glm::vec3 b, glm::vec3 c) {
    glm::vec3 ba = b - a;
    glm::vec3 ca = c - a;
    return glm::normalize(glm::cross(ba, ca));
}

// reads data from a directory entry
template<typename T> T BSPFile::read(dir_entry dir, size_t idx) {
    assert(idx < dir.size / sizeof(T));
    bsp_file.clear();
    bsp_file.seekg(dir.offset);
    return ReadT<T>(bsp_file, idx);
}

std::vector<entity> BSPFile::entities() {
    bsp_file.clear();
    bsp_file.seekg(header.entities.offset);

    std::vector<entity> entities{};
    // loop until the file pointer position is past all entities
    while (bsp_file.tellg() < header.entities.offset + header.entities.size) {
        entities.push_back(readEntity(bsp_file));
    }

    return entities;
}

plane BSPFile::getPlane(size_t idx) {
    return read<plane>(header.planes, idx);
}

// TODO: remove this
// the first value at the miptex offset is the number of miptexs in the BSP
int32_t BSPFile::miptexCount() {
    bsp_file.clear();
    bsp_file.seekg(header.miptex.offset);
    return ReadT<int32_t>(bsp_file);
}

miptex BSPFile::getMiptex(size_t idx) {
    int32_t offset = ReadT<int32_t>(bsp_file, idx);
    bsp_file.seekg(header.miptex.offset + offset);
    return ReadT<miptex>(bsp_file);
}

glm::vec3 BSPFile::getVertex(size_t idx) {
    return read<glm::vec3>(header.verticies, idx);
}

node BSPFile::getNode(size_t idx) {
    return read<node>(header.nodes, idx);
}

tex_info BSPFile::getTexInfo(size_t idx) {
    return read<tex_info>(header.texinfos, idx);
}

face BSPFile::getFace(size_t idx) {
    return read<face>(header.faces, idx);
}

leaf BSPFile::getLeaf(size_t idx) {
    return read<leaf>(header.leaves, idx);
}

uint16_t BSPFile::getListFace(size_t idx) {
    return read<uint16_t>(header.listfaces, idx);
}

edge BSPFile::getEdge(size_t idx) {
    return read<edge>(header.edges, idx);
}

int32_t BSPFile::getListEdge(size_t idx) {
    return read<int32_t>(header.listedges, idx);
}

bsp_model BSPFile::getModel(size_t idx) {
    return read<bsp_model>(header.models, idx);
}

std::vector<color_rgb8> BSPFile::getMiptexData(size_t idx, uint8_t miplevel) {
    miptex mip = getMiptex(idx);
    
    // this is why the miptex must have width and height be a multiple of 8
    uint32_t width = mip.width >> miplevel;
    uint32_t height = mip.height >> miplevel;
    
    // move file read pointer back from end of miptex data
    // to the mip level specified
    bsp_file.seekg(-sizeof(miptex) + mip.offset[miplevel], std::ios::cur);
    
    // read in indexed color info
    std::vector<uint8_t> palette_indices(width * height);
    bsp_file.read((char *)palette_indices.data(), width * height);

    std::vector<color_rgb8> color_data;
    std::transform(palette_indices.begin(), palette_indices.end(), std::back_inserter(color_data), palette);
    return color_data;
}

mesh BSPFile::genMeshFaces(std::span<const face> faces) {
    std::vector<glm::vec3> vertices{};
    std::vector<glm::vec2> texcoords{};
    std::vector<glm::vec3> normals{};

    for (const face &f : faces) {
        // get the texture info and miptex for each face
        tex_info texinfo = getTexInfo(f.texinfo_id);
        miptex mip = getMiptex(texinfo.miptex_id);

        std::vector<glm::vec3> face_vertices{};
        std::vector<glm::vec2> face_texcoords{};
        
        // Iterate over all edges in a face
        // NOTE: ledge = list of edges in face
        for (size_t i = 0; i < f.ledge_num; i++) {
            int16_t ledge = getListEdge(f.ledge_id + i);
            edge e = getEdge(labs(ledge));
            
            glm::vec3 vert = getVertex(ledge >= 0 ? e.vs : e.ve);
            face_vertices.push_back(vert);
            
            // gets the UV mapping of the texture
            glm::vec2 uv;
            uv.x = (glm::dot(vert, texinfo.u_axis) + texinfo.u_offset) / mip.width;
            uv.y = (glm::dot(vert, texinfo.v_axis) + texinfo.v_offset) / mip.height;
            face_texcoords.push_back(uv);
        }
        
        // collect vertices from each face into total vertices
        for (size_t i = face_vertices.size() - 2; i > 0; i--) {
            glm::vec3 v[3] = {
                fromQuake(face_vertices.back()),
                fromQuake(face_vertices[i]),
                fromQuake(face_vertices[i-1]),
            };
            vertices.insert(vertices.end(), std::begin(v), std::end(v));

            texcoords.push_back(face_texcoords.back());
            texcoords.push_back(face_texcoords[i]);
            texcoords.push_back(face_texcoords[i-1]);
            
            // assign the normal to the tri and push it for each vert
            glm::vec3 normal = verticesNormal(v[0], v[1], v[2]);
            for (size_t v = 0; v < 3; v++) {
                normals.push_back(normal);
            }
        }
    }

    mesh m{};
    m.vertexCount = vertices.size();
    m.vertices = (float*)vertices.data();
    m.texcoords = (float*)texcoords.data();
    UploadMesh(&m, false);
    
    // so the free functions dont error
    m.vertices = (float*)malloc(1);
    m.texcoords = (float*)malloc(1);
    m.normals = (float*)malloc(1);
    return m;
}

//==================================================
// MAP LOADING
//==================================================

std::vector<map_model> loadModelsFromBSPFile(const std::filesystem::path &path) {
    std::ifstream bsp_file{path, std::ios::binary};
    BSPFile map{bsp_file};

    node bsp_root = map.getNode(map.getModel(0).bsp_node_id);
    std::vector<node> nodes{bsp_root};
    std::set<size_t> leaves{};

    while (nodes.empty() == false) {
        node n = nodes.back();
        nodes.pop_back();

        // look at the nodes children and add to tree
        for (int16_t i : {n.front, n.back}) {
            if (i > 0) {
                nodes.push_back(map.getNode(i));
            } else {
                size_t leaf_id = ~i;
                leaves.insert(leaf_id);
            }
        }
    }

    std::unordered_map<std::string, texture> texture_name_to_object{};
    std::unordered_map<std::string, std::vector<face>> texture_name_to_face_list{};

    for (size_t leaf_id : leaves) {
        leaf l = map.getLeaf(leaf_id);
        for (size_t i = 0; i < l.listface_num; i++) {
            uint16_t face_id = map.getListFace(l.listface_id + i);
            face f = map.getFace(face_id);

            tex_info texinfo = map.getTexInfo(f.texinfo_id);
            miptex mip = map.getMiptex(texinfo.miptex_id);
            
            // grouping faces by texture name to reduce draw calls
            std::string texname = mip.name;
            texture_name_to_face_list[texname].push_back(f);
            
            if (texture_name_to_object.contains(texname) == false) {
                std::vector<color_rgb8> color_data = map.getMiptexData(texinfo.miptex_id, 0);

                // create an opengl texture id
                unsigned int texID;
                glGenTextures(1, &texID);
                glBindTexture(GL_TEXTURE_2D, texID);

                // setup filtering parameters for display
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                // load colors data into texture
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)mip.width, (int)mip.height, 0, GL_RGB, GL_UNSIGNED_BYTE, color_data.data());
                // Unbind current texture
                glBindTexture(GL_TEXTURE_2D, 0); 
                
                texture tex = {
                    .id = texID,
                    .width = (int)mip.width,
                    .height = (int)mip.height,
                    .mipmaps = 1,
                    .format = GL_RGB,
                };

                texture_name_to_object[texname] = tex;
            }
        }
    }

    std::vector<map_model> models{};
    for (auto &[texname, faces] : texture_name_to_face_list) {
        mesh ms = map.genMeshFaces(faces);
        map_model mdl = LoadMapModelFromMesh(ms);
        mdl.textures[0] = texture_name_to_object.at(texname);
        models.push_back(mdl);
    }

    return models;
}

//==================================================
// QUAKE PALETTE
//==================================================

// temporary for rendering will likely outmode soon
color_rgb8 palette(uint8_t id)
{
	static const color_rgb8 _PALETTE[] = {
		{0, 0, 0},
		{15, 15, 15},
		{31, 31, 31},
		{47, 47, 47},
		{63, 63, 63},
		{75, 75, 75},
		{91, 91, 91},
		{107, 107, 107},
		{123, 123, 123},
		{139, 139, 139},
		{155, 155, 155},
		{171, 171, 171},
		{187, 187, 187},
		{203, 203, 203},
		{219, 219, 219},
		{235, 235, 235},
		{15, 11, 7},
		{23, 15, 11},
		{31, 23, 11},
		{39, 27, 15},
		{47, 35, 19},
		{55, 43, 23},
		{63, 47, 23},
		{75, 55, 27},
		{83, 59, 27},
		{91, 67, 31},
		{99, 75, 31},
		{107, 83, 31},
		{115, 87, 31},
		{123, 95, 35},
		{131, 103, 35},
		{143, 111, 35},
		{11, 11, 15},
		{19, 19, 27},
		{27, 27, 39},
		{39, 39, 51},
		{47, 47, 63},
		{55, 55, 75},
		{63, 63, 87},
		{71, 71, 103},
		{79, 79, 115},
		{91, 91, 127},
		{99, 99, 139},
		{107, 107, 151},
		{115, 115, 163},
		{123, 123, 175},
		{131, 131, 187},
		{139, 139, 203},
		{0, 0, 0},
		{7, 7, 0},
		{11, 11, 0},
		{19, 19, 0},
		{27, 27, 0},
		{35, 35, 0},
		{43, 43, 7},
		{47, 47, 7},
		{55, 55, 7},
		{63, 63, 7},
		{71, 71, 7},
		{75, 75, 11},
		{83, 83, 11},
		{91, 91, 11},
		{99, 99, 11},
		{107, 107, 15},
		{7, 0, 0},
		{15, 0, 0},
		{23, 0, 0},
		{31, 0, 0},
		{39, 0, 0},
		{47, 0, 0},
		{55, 0, 0},
		{63, 0, 0},
		{71, 0, 0},
		{79, 0, 0},
		{87, 0, 0},
		{95, 0, 0},
		{103, 0, 0},
		{111, 0, 0},
		{119, 0, 0},
		{127, 0, 0},
		{19, 19, 0},
		{27, 27, 0},
		{35, 35, 0},
		{47, 43, 0},
		{55, 47, 0},
		{67, 55, 0},
		{75, 59, 7},
		{87, 67, 7},
		{95, 71, 7},
		{107, 75, 11},
		{119, 83, 15},
		{131, 87, 19},
		{139, 91, 19},
		{151, 95, 27},
		{163, 99, 31},
		{175, 103, 35},
		{35, 19, 7},
		{47, 23, 11},
		{59, 31, 15},
		{75, 35, 19},
		{87, 43, 23},
		{99, 47, 31},
		{115, 55, 35},
		{127, 59, 43},
		{143, 67, 51},
		{159, 79, 51},
		{175, 99, 47},
		{191, 119, 47},
		{207, 143, 43},
		{223, 171, 39},
		{239, 203, 31},
		{255, 243, 27},
		{11, 7, 0},
		{27, 19, 0},
		{43, 35, 15},
		{55, 43, 19},
		{71, 51, 27},
		{83, 55, 35},
		{99, 63, 43},
		{111, 71, 51},
		{127, 83, 63},
		{139, 95, 71},
		{155, 107, 83},
		{167, 123, 95},
		{183, 135, 107},
		{195, 147, 123},
		{211, 163, 139},
		{227, 179, 151},
		{171, 139, 163},
		{159, 127, 151},
		{147, 115, 135},
		{139, 103, 123},
		{127, 91, 111},
		{119, 83, 99},
		{107, 75, 87},
		{95, 63, 75},
		{87, 55, 67},
		{75, 47, 55},
		{67, 39, 47},
		{55, 31, 35},
		{43, 23, 27},
		{35, 19, 19},
		{23, 11, 11},
		{15, 7, 7},
		{187, 115, 159},
		{175, 107, 143},
		{163, 95, 131},
		{151, 87, 119},
		{139, 79, 107},
		{127, 75, 95},
		{115, 67, 83},
		{107, 59, 75},
		{95, 51, 63},
		{83, 43, 55},
		{71, 35, 43},
		{59, 31, 35},
		{47, 23, 27},
		{35, 19, 19},
		{23, 11, 11},
		{15, 7, 7},
		{219, 195, 187},
		{203, 179, 167},
		{191, 163, 155},
		{175, 151, 139},
		{163, 135, 123},
		{151, 123, 111},
		{135, 111, 95},
		{123, 99, 83},
		{107, 87, 71},
		{95, 75, 59},
		{83, 63, 51},
		{67, 51, 39},
		{55, 43, 31},
		{39, 31, 23},
		{27, 19, 15},
		{15, 11, 7},
		{111, 131, 123},
		{103, 123, 111},
		{95, 115, 103},
		{87, 107, 95},
		{79, 99, 87},
		{71, 91, 79},
		{63, 83, 71},
		{55, 75, 63},
		{47, 67, 55},
		{43, 59, 47},
		{35, 51, 39},
		{31, 43, 31},
		{23, 35, 23},
		{15, 27, 19},
		{11, 19, 11},
		{7, 11, 7},
		{255, 243, 27},
		{239, 223, 23},
		{219, 203, 19},
		{203, 183, 15},
		{187, 167, 15},
		{171, 151, 11},
		{155, 131, 7},
		{139, 115, 7},
		{123, 99, 7},
		{107, 83, 0},
		{91, 71, 0},
		{75, 55, 0},
		{59, 43, 0},
		{43, 31, 0},
		{27, 15, 0},
		{11, 7, 0},
		{0, 0, 255},
		{11, 11, 239},
		{19, 19, 223},
		{27, 27, 207},
		{35, 35, 191},
		{43, 43, 175},
		{47, 47, 159},
		{47, 47, 143},
		{47, 47, 127},
		{47, 47, 111},
		{47, 47, 95},
		{43, 43, 79},
		{35, 35, 63},
		{27, 27, 47},
		{19, 19, 31},
		{11, 11, 15},
		{43, 0, 0},
		{59, 0, 0},
		{75, 7, 0},
		{95, 7, 0},
		{111, 15, 0},
		{127, 23, 7},
		{147, 31, 7},
		{163, 39, 11},
		{183, 51, 15},
		{195, 75, 27},
		{207, 99, 43},
		{219, 127, 59},
		{227, 151, 79},
		{231, 171, 95},
		{239, 191, 119},
		{247, 211, 139},
		{167, 123, 59},
		{183, 155, 55},
		{199, 195, 55},
		{231, 227, 87},
		{127, 191, 255},
		{171, 231, 255},
		{215, 255, 255},
		{103, 0, 0},
		{139, 0, 0},
		{179, 0, 0},
		{215, 0, 0},
		{255, 0, 0},
		{255, 243, 147},
		{255, 247, 199},
		{255, 255, 255},
		{159, 91, 83},
	};
	return _PALETTE[id];
}
