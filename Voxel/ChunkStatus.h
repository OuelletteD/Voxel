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
	const auto& mesh = chunk.chunkMesh;
	if (!mesh.dirty.load(std::memory_order_acquire)) {
		return ChunkStatus::Ready; // it should continue
	}
	if (mesh.queued.load(std::memory_order_acquire)) {
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

