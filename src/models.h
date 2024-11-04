#pragma once

#include "borepackdefs.h"

// Material map index
typedef enum {
    MATERIAL_MAP_ALBEDO = 0,        // Albedo material
    MATERIAL_MAP_METALNESS,         // Metalness material
    MATERIAL_MAP_NORMAL,            // Normal material
    MATERIAL_MAP_ROUGHNESS,         // Roughness material
    MATERIAL_MAP_OCCLUSION,         // Ambient occlusion material
    MATERIAL_MAP_EMISSION,          // Emission material
    MATERIAL_MAP_HEIGHT,            // Heightmap material
    MATERIAL_MAP_CUBEMAP,           // Cubemap material
    MATERIAL_MAP_IRRADIANCE,        // Irradiance material
    MATERIAL_MAP_PREFILTER,         // Prefilter material
    MATERIAL_MAP_BRDF               // Brdf material
} material_map_index;

map_model LoadMapModelFromMesh(mesh input_mesh);
void UploadMesh(mesh *mesh, bool dynamic);
