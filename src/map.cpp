#include "map.h"
#include "bsp.h"
#include "camera.h"
#include "gtc/type_ptr.hpp"
#include "material.h"
#include "renderer.h"
#include "shader.h"
#include <cstring>
#include <fstream>

#define loadBSP(filename) ((bsp_header *)loadBinaryFile(filename))

static map loaded_map;
static int num_indicies;
static uint32_t *indices;
static int num_render_faces;
static int *render_faces;

#define LIGHTMAP_WIDTH 1024
#define LIGHTMAP_HEIGHT 1024

static int allocated[LIGHTMAP_WIDTH];

bool allocBlock(int width, int height, int *x, int *y) {
    int best = LIGHTMAP_HEIGHT;
    
    for (int i = 0; i < LIGHTMAP_WIDTH - width; i++) {
        int best2 = 0;
        int j;
        for (j = 0; j < width; j++) {
            if (allocated[i + j] >= best) break;
            if (allocated[i + j] > best2) best2 = allocated[i + j];
        }

        if (j == width) {
            *x = i;
            *y = best = best2;
        }
    }

    if (best + height > LIGHTMAP_HEIGHT) return false;

    for (int i = 0; i < width; i++) {
        allocated[*x + i] = best + height;
    }

    return true;
}

void *loadBinaryFile(const char *filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    void *result = 0;
    if (file)
    {
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        result = malloc(static_cast<size_t>(fileSize));
        if (!file.read(static_cast<char*>(result), fileSize))
        {
            free(result);
        }

    }
    return result;
}

uint32_t buildTexture(uint8_t *miptex_data, int width, int height, int offset, int pitch, color *palette, GLenum filter, color *pixel_buffer) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int miptex_idx = ((x + offset) + y * pitch);
            int pixel_idx = x + y * width;
            color clr = palette[miptex_data[miptex_idx]];
            pixel_buffer[pixel_idx] = {clr.r, clr.g, clr.b};
        }
    }
    uint32_t result = createTexture(pixel_buffer, width, height, GL_RGB, filter, GL_REPEAT, 1);
    return result;
}

template <typename T>
void copyLump(bsp_header *header, int lump_type, T **out_items, int *out_num_items) {
    bsp_lump lump = header->lumps[lump_type];
    if (out_num_items) *out_num_items = lump.length / sizeof(T);
    *out_items = (T *)((uint8_t *)header + lump.offset);
}

void mapInitBSP(bsp_header *header) {
    copyLump(header, BSP_LUMP_PLANES, &loaded_map.planes, &loaded_map.num_planes);
    copyLump(header, BSP_LUMP_VERTICES, &loaded_map.vertices, &loaded_map.num_vertices);
    copyLump(header, BSP_LUMP_NODES, &loaded_map.nodes, &loaded_map.num_nodes);
    copyLump(header, BSP_LUMP_TEXINFO, &loaded_map.texinfos, &loaded_map.num_texinfos);
    copyLump(header, BSP_LUMP_FACES, &loaded_map.faces, &loaded_map.num_faces);
    copyLump(header, BSP_LUMP_CLIPNODES, &loaded_map.clipnodes, &loaded_map.num_clipnodes);
    copyLump(header, BSP_LUMP_LEAFS, &loaded_map.leafs, &loaded_map.num_leafs);
    copyLump(header, BSP_LUMP_MARKSURFACES, &loaded_map.mark_surfaces, &loaded_map.num_mark_surfaces);
    copyLump(header, BSP_LUMP_EDGES, &loaded_map.edges, &loaded_map.num_edges);
    copyLump(header, BSP_LUMP_SURFEDGES, &loaded_map.surfedges, &loaded_map.num_surfedges);
    copyLump(header, BSP_LUMP_MODELS, &loaded_map.models, &loaded_map.num_models);
    copyLump(header, BSP_LUMP_LIGHTMAPS, &loaded_map.lightmap, 0);

    bsp_lump miptex_lump = header->lumps[BSP_LUMP_MIPTEX];
    loaded_map.miptex_lump = (bsp_miptex_lump *)((uint8_t *)header + miptex_lump.offset);
}

void mapInitMaterials() {
    int num_texs = loaded_map.miptex_lump->miptex_count;
    material *mats = (material *)malloc(sizeof(material) * num_texs);
    memset(mats, 0, sizeof(material) * num_texs);
    loaded_map.materials = mats;
    loaded_map.num_materials = num_texs;
}

void mapInitTextures() {
    // TODO: fix this jank
    color palette[256] = {
        #include "colormap.h"
    };
    int num_texs = loaded_map.miptex_lump->miptex_count;

    int max_tex_width = 256;
    int max_tex_height = 256;
    color *pixel_buffer = (color *)malloc(sizeof(color) * max_tex_width * max_tex_height);

    for (int i = 0; i < num_texs; i++) {
        material *mat = &loaded_map.materials[i];
        mat->cull_face = 1;

        bsp_miptex *miptex = (bsp_miptex *)((uint8_t *)loaded_map.miptex_lump + loaded_map.miptex_lump->data_offset[i]);

        int tex_width = miptex->width;
        int tex_height = miptex->height;
        uint8_t *mip_data = (uint8_t *)miptex + miptex->offsets[0];

        if (strncmp(miptex->name, "sky", 3) == 0) {
            mat->program = getShader("SkyShader");
            mat->depth_test = true;
            int sky_tex_width = tex_width >> 1;
            uint32_t fg_tex = buildTexture(mip_data, sky_tex_width, tex_height, 0, tex_width, palette, GL_NEAREST, pixel_buffer);
            uint32_t bg_tex = buildTexture(mip_data, sky_tex_width, tex_height, sky_tex_width, tex_width, palette, GL_NEAREST, pixel_buffer);
            materialSetFloat(mat, "Time", 0.0f);
            materialSetVec3(mat, "CameraPosition", glm::vec3(0.0f));
            materialSetTexture(mat, "Texture0", fg_tex);
            materialSetTexture(mat, "Texture2", bg_tex);
        } else {
            uint32_t tex = buildTexture(mip_data, tex_width, tex_height, 0, tex_width, palette, GL_LINEAR, pixel_buffer);
            mat->program = getShader("SurfaceShader");
            mat->depth_test = true;
            materialSetTexture(mat, "Texture0", tex);
        }
    }
    free(pixel_buffer);
}

void calcSurfaceExtents(surface *surf) {
    glm::vec2 uv_min = glm::vec2(FLT_MAX);
    glm::vec2 uv_max = glm::vec2(-FLT_MAX);
    bsp_face face = loaded_map.faces[surf->face];
    bsp_texinfo texinfo = loaded_map.texinfos[face.texinfo];

    for (int i = 0; i < face.edge_count; i++) {
        int edge = ((int *)loaded_map.surfedges)[face.first_edge + i];
        glm::vec3 pos = {};

        if (edge >= 0) {
            pos = loaded_map.vertices[loaded_map.edges[edge][0]];
        } else {
            pos = loaded_map.vertices[loaded_map.edges[-edge][1]];
        }

        double u = (double)pos.x * (double)texinfo.uaxis.x + 
                   (double)pos.y * (double)texinfo.uaxis.y +
                   (double)pos.x * (double)texinfo.uaxis.z +
                   (double)texinfo.uoffset;

        double v = (double)pos.x * (double)texinfo.vaxis.x + 
                   (double)pos.y * (double)texinfo.vaxis.y +
                   (double)pos.x * (double)texinfo.vaxis.z +
                   (double)texinfo.voffset;

        if (uv_min.s > u) uv_min.s = (float)u;
        if (uv_min.t > v) uv_min.t = (float)v;

        if (uv_max.s < u) uv_max.s = (float)u;
        if (uv_max.t < v) uv_max.t = (float)v;

    }

    for (int i = 0; i < 2; i++) {
        int min = (int)glm::floor(uv_min[i] / 16);
        int max = (int)glm::ceil(uv_max[i] / 16);

        surf->tex_mins[i] = min * 16;
        surf->uv_extents[i] = (max - min) * 16;
    }
}

void markSurface(int leaf_idx) {
    bsp_leaf leaf = loaded_map.leafs[leaf_idx];
    for (int i = 0; i < leaf.mark_surface_count; i++) {
        int face_idx = loaded_map.mark_surfaces[leaf.first_mark_surface + i];
        render_faces[face_idx] = 1;
    }
}

void buildBSPTree(bsp_node node) {
    if ((node.children[0] & 0x8000) == 0) {
        int child_idx = node.children[0];
        buildBSPTree(loaded_map.nodes[child_idx]);
    } else {
        int leaf_idx = ~node.children[0];
        markSurface(leaf_idx);
    }

    if ((node.children[1] & 0x8000) == 0) {
        int child_idx = node.children[1];
        buildBSPTree(loaded_map.nodes[child_idx]);
    } else {
        int leaf_idx = ~node.children[1];
        markSurface(leaf_idx);
    }
}

void createSurfaces() {
    num_render_faces = loaded_map.num_mark_surfaces;
    render_faces = (int *)malloc(sizeof(int) * num_render_faces);
    memset(render_faces, 0, sizeof(int) * num_render_faces);

    bsp_model mdl = loaded_map.models[0];
    buildBSPTree(loaded_map.nodes[mdl.head_nodes[0]]);

    loaded_map.surfaces = (surface *)malloc(sizeof(surface) * loaded_map.num_mark_surfaces);

    for (int i = 0; i < num_render_faces; i++) {
        if (render_faces[i]) {
            surface *surf = loaded_map.surfaces + i;
            *surf = {};
            surf->face = i;
            calcSurfaceExtents(surf);
            loaded_map.num_surfaces++;
        }
    }
}

int getVertexFromEdge(int surf_edge) {
    int edge = ((int *)loaded_map.surfedges)[surf_edge];
    if (edge >= 0)
        return loaded_map.edges[edge][0];
    return loaded_map.edges[-edge][1];
}

void triangulateSurface(surface *surf, uint32_t *triangle) {
    surf->first_index = num_indicies;
    bsp_face face = loaded_map.faces[surf->face];
    int num_tris = face.edge_count - 2;

    for(int i = 1; i <= num_tris; i++) {
        int edge_idx = face.first_edge + i;
        *triangle++ = getVertexFromEdge(face.first_edge);
        *triangle++ = getVertexFromEdge(edge_idx);
        *triangle++ = getVertexFromEdge(edge_idx + 1);
        surf->num_indices += 3;
    }
}

void mapInitMeshes() {
    createSurfaces();

    // allocate memory
    int max_tris = 65536;
    indices = (uint32_t *)malloc(sizeof(uint32_t) * max_tris);

    uint64_t size_in_bytes = sizeof(uint8_t) * LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT;
    uint8_t *lightmap_bitmap = (uint8_t *)malloc(size_in_bytes);
    memset(lightmap_bitmap, 0, size_in_bytes);

    // create tris
    for (int i = 0; i < loaded_map.num_surfaces; i++) {
        surface *surf = loaded_map.surfaces + i;
        triangulateSurface(surf, indices + num_indicies);
        num_indicies += surf->num_indices;
    }

    uint64_t max_vertex_buffer_size = sizeof(vertex) * 65536;
    uint64_t num_vertex_buffers = (uint64_t)loaded_map.miptex_lump->miptex_count;

    vertex **vertex_buffers = (vertex **)malloc(sizeof(vertex *) * num_vertex_buffers);
    for (int i = 0; i < num_vertex_buffers; i++) {
        vertex_buffers[i] = (vertex *)malloc(sizeof(vertex *) * max_vertex_buffer_size);
    }

    int *vertex_buffers_num_vertices = (int *)malloc(sizeof(int) * num_vertex_buffers);
    memset(vertex_buffers_num_vertices, 0, sizeof(int) * num_vertex_buffers);

    for (int i = 0; i < loaded_map.num_surfaces; i++) {
        surface *surf = loaded_map.surfaces + i;
        bsp_face face = loaded_map.faces[surf->face];
        bsp_texinfo texinfo = loaded_map.texinfos[face.texinfo];
        bsp_miptex *miptex = (bsp_miptex *)((uint8_t *)loaded_map.miptex_lump + loaded_map.miptex_lump->data_offset[texinfo.miptex]);
        glm::vec3 *map_vertices = loaded_map.vertices;

        int vertex_buffer_idx = texinfo.miptex;
        vertex *vertex_buffer = vertex_buffers[vertex_buffer_idx];
        int *num_vertices = vertex_buffers_num_vertices + vertex_buffer_idx;

        int lightmap_block_width = (surf->uv_extents.s >> 4) + 1;
        int lightmap_block_height = (surf-> uv_extents.t >> 4) + 1;
        allocBlock(lightmap_block_width, lightmap_block_height, &surf->lightmap_offset.x, &surf->lightmap_offset.y);

        uint8_t *lightmap_texels = loaded_map.lightmap + face.light_offset;

        for (int y = 0; y < lightmap_block_height; y++) {
            for (int x = 0; x < lightmap_block_width; x++) {
                uint8_t pixel = lightmap_texels[x + y * lightmap_block_width];
                uint32_t image_idx = (x + surf->lightmap_offset.x) + (y + surf->lightmap_offset.y) * LIGHTMAP_WIDTH;

                if (face.light_offset == -1) {
                    lightmap_bitmap[image_idx] = 255;
                } else {
                    lightmap_bitmap[image_idx] = pixel;
                }
            }
        }

        for (int j = 0; j < surf->num_indices; j++) {
            int vert_idx = indices[surf->first_index + j];
            glm::vec3 pos = map_vertices[vert_idx];
            glm::vec2 texcoord = glm::vec2(
                (glm::dot(pos, texinfo.uaxis) + texinfo.uoffset) / miptex->width,
                (glm::dot(pos, texinfo.vaxis) + texinfo.voffset) / miptex->height
            );

            float s = glm::dot(pos, texinfo.uaxis) + texinfo.uoffset;
            s -= (float)surf->tex_mins.s;
            s += (float)surf->lightmap_offset.x * 16;
            s += 8;
            s /= (float)(LIGHTMAP_WIDTH * 16);

            float t = glm::dot(pos, texinfo.vaxis) + texinfo.voffset;
            t -= (float)surf->tex_mins.t;
            t += (float)surf->lightmap_offset.y * 16;
            t += 8;
            t /= (float)(LIGHTMAP_HEIGHT * 16);

            vertex_buffer[*num_vertices].pos = pos;
            vertex_buffer[*num_vertices].texcoord = texcoord;
            vertex_buffer[*num_vertices].lightmap = glm::vec2(s, t);
            *num_vertices = *num_vertices + 1;
        }
    }

    loaded_map.lightmap_tex = createTexture(lightmap_bitmap, LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT, GL_RED, GL_LINEAR, GL_CLAMP_TO_EDGE);
    free(lightmap_bitmap);

    loaded_map.num_meshes = num_vertex_buffers;
    loaded_map.meshes = (mesh *)malloc(sizeof(mesh) * loaded_map.num_meshes);

    for (int i = 0; i < num_vertex_buffers; i++) {
        loaded_map.meshes[i] = createMesh(vertex_buffers[i], vertex_buffers_num_vertices[i], 0, 0);
        loaded_map.meshes[i].topology = GL_TRIANGLES;
        loaded_map.meshes[i].material_index = i;

        materialSetTexture(&loaded_map.materials[i], "Texture1", loaded_map.lightmap_tex);
    }

    free(vertex_buffers_num_vertices);
    for (int i = 0; i < num_vertex_buffers; i++) {
        free(vertex_buffers[i]);
    }
    free(vertex_buffers);
}

void loadMap(const char *filename) {
    bsp_header *header = loadBSP(filename);
    mapInitBSP(header);
    mapInitMaterials();
    mapInitTextures();
    mapInitMeshes();
}

void drawMap(float time, camera *cam) {
    glm::mat4 proj_mtx = cam->projection_mtx;
    glm::mat4 view_mtx = cameraGetViewMatrix(cam);
    glm::mat4 quake_transform_mtx = glm::mat4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,-1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    for (int i = 0; i < loaded_map.num_meshes; i++) {
        mesh m = loaded_map.meshes[i];
        //glm::mat4 mvp = proj_mtx * view_mtx * quake_transform_mtx;
        material *mat = &loaded_map.materials[m.material_index];
        materialBind(*mat);
        materialSetFloat(mat, "Time", time);
        materialSetVec3(mat, "CameraPosition", cam->pos);
        glUniformMatrix4fv(glGetUniformLocation(mat->program, "ProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(proj_mtx));
        glUniformMatrix4fv(glGetUniformLocation(mat->program, "ViewMatrix"), 1, GL_FALSE, glm::value_ptr(view_mtx));
        glUniformMatrix4fv(glGetUniformLocation(mat->program, "ModelMatrix"), 1, GL_FALSE, glm::value_ptr(quake_transform_mtx));
        meshDraw(m);
    }
}
