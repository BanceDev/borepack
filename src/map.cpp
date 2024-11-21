#include <fstream>
#include <iostream>
#include <cstring>
#include <memory>
#include "map.h"

bool Map::Initialize(const char *bspFilename, const char *paletteFilename)
{
    if (!LoadBSP(bspFilename)) {
        std::cerr << "[ERROR] Map::Initialize() Error loading bsp file\n";
        return false;
    }

    if (!LoadPalette(paletteFilename)) {
        std::cerr << "[ERROR] Map::Initialize() Error loading palette\n";
        return false;
    }

    return true;
}

int Map::LoadFile(const char *filename, void **bufferptr)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        return 0;
    }

    std::streamsize length = file.tellg();
    file.seekg(0, std::ios::beg);

    auto buffer = std::make_unique<char[]>(length);
    if (!file.read(buffer.get(), length)) {
        return 0;
    }

    *bufferptr = buffer.release();
    return static_cast<int>(length);
}

bool Map::LoadPalette(const char *filename)
{
    unsigned char *tempPal = nullptr;

    if (!LoadFile(filename, (void **)&tempPal)) {
        return false;
    }

    for (int i = 0; i < 256; i++) {
        unsigned int r = tempPal[i * 3 + 0];
        unsigned int g = tempPal[i * 3 + 1];
        unsigned int b = tempPal[i * 3 + 2];
        palette[i] = (r) | (g << 8) | (b << 16);
    }

    delete[] tempPal; // Replace free() with delete[]
    return true;
}

bool Map::LoadBSP(const char *filename)
{
    if (!LoadFile(filename, (void **)&bsp)) {
        return false;
    }

    header = reinterpret_cast<dheader_t *>(bsp);
    if (header->version != BSP_VERSION) {
        std::cerr << "[ERROR] Map::LoadBSP() BSP file version mismatch!\n";
        return false;
    }

    return true;
}
