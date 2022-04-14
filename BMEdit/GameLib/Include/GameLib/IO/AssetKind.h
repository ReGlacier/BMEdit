#pragma once


namespace gamelib::io
{
	enum class AssetKind
	{
		SCENE,  ///< GMS
		PROPERTIES, ///< PRP
		TEXTURES, ///< TEX
		GEOMETRY, ///< PRM
		MATERIALS, ///< MAT
		OCTREE, ///< OCT
		ROOM_TREE_INSIDE, ///< RMI
		ROOM_TREE_OUTSIDE, ///< RMC
		LOCALIZATION, ///< LOC
		ANIMATION, ///< ANM
		SOUND, ///< SND
		BUFFER, ///< BUF
	};
}