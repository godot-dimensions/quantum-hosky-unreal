#pragma once

// Size of a render chunk
// Bigger = less draw calls
// Smaller = faster edits
// Must be a power of 2
#ifndef QH_RENDER_CHUNK_SIZE
#define QH_RENDER_CHUNK_SIZE 8
#endif

#ifndef QH_DATA_CHUNK_SIZE
#define QH_DATA_CHUNK_SIZE 4
#endif

#ifndef QH_VOXELS_PER_DATA_CHUNK
#define QH_VOXELS_PER_DATA_CHUNK QH_DATA_CHUNK_SIZE * QH_DATA_CHUNK_SIZE * QH_DATA_CHUNK_SIZE * QH_DATA_CHUNK_SIZE
#endif

using FQHVoxelValue = bool;
using FQHVoxelCellIndex = uint16;
