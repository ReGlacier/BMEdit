#pragma once

#include <QMetaType>


namespace models
{
	class SceneTexturesModel;
}

namespace types
{
	struct QTextureREF
	{
		uint32_t textureIndex;
		const models::SceneTexturesModel* ownerModel { nullptr };
	};
}

Q_DECLARE_METATYPE(types::QTextureREF)