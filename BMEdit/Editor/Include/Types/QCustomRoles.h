#pragma once

#include <Qt>

namespace types {
	constexpr int kSceneObjectRole       = Qt::UserRole + 1;
	// 2..8 - free
	constexpr int kChunkIndexRole        = Qt::UserRole + 9;
	constexpr int kChunkKindRole         = Qt::UserRole + 10;
	constexpr int kChunkVertexFormatRole = Qt::UserRole + 11;
	// +12 .. - free
}
