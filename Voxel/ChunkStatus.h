#pragma once
#include <glm/glm.hpp>
#include "Chunk.h"

enum ChunkStatus {
	Ready,
	NewChunk,
	WaitingForRerender,
	Rerendering,
	NewChunkRendering
};

ChunkStatus GetChunkStatus(const Chunk& chunk) {
	if (!chunk.chunkMesh.needsMeshUpdate) 
		return ChunkStatus::Ready; // it should continue

	if (chunk.chunkMesh.isUpdating) {
		if (chunk.chunkMesh.isNewChunk) {
			return ChunkStatus::NewChunkRendering;
		} else {
			return ChunkStatus::Rerendering; // so it should continue
		}
	} else {
		if (chunk.chunkMesh.isNewChunk) {
			return ChunkStatus::NewChunk; // so it should continue
		} else {
			return ChunkStatus::WaitingForRerender;
		}
	}
}

