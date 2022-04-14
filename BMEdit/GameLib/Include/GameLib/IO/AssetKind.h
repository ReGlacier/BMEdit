#pragma once


namespace gamelib::io
{
	enum AssetKind : int
	{
		SCENE = 0,  ///< GMS
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
		ZGF, ///< ZGF (dummy file)


		LAST_ASSET_KIND
	};
}