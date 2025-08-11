#ifndef CHUNK_H
#define CHUNK_H

#include <stdint.h>

constexpr uint32_t CHUNK_ID_CUE   = 0x20657563;
constexpr uint32_t CHUNK_ID_LIST  = 0x5453494C;
constexpr uint32_t CHUNK_ID_LABL  = 0x6C62616C;
constexpr uint32_t CHUNK_ID_DATA =  0x61746164;
constexpr uint32_t LIST_TYPE_ADTL = 0x6C746461;


struct CuePoint {
    uint32_t ID;
    uint32_t position;
    uint32_t dataChunkID;
    uint32_t chunkStart;
    uint32_t blockStart;
    uint32_t sampleOffset;
};

// for CueChunks, follow header with a list of CuePoints, each 24 bytes. Info = numCuePoints

// For ListChunkHeader, follow header with a collection of LabelChunks. Info = ListType

// For LabelChunks, follow with a string, padded to an even byte size. Info = CuePointID

struct Chunk {
    uint32_t ckID;
    uint32_t ckSize;
    uint32_t ckInfo;
};
// #pragma pack(push, 1)
// #pragma pack(pop)

#endif