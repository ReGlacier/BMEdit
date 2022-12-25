#pragma once


namespace gamelib::prm
{
	enum class PRMChunkRecognizedKind
	{
		CRK_ZERO_CHUNK,  			///< Only for chunk #0
		CRK_INDEX_BUFFER,           ///< For chunk with indices data
		CRK_VERTEX_BUFFER,          ///< For chunk with vertices data
		CRK_DESCRIPTION_BUFFER,     ///< For chunk with description
		CRK_BONE_BUFFER,            ///< Rigid body bones description
		CRK_UNKNOWN_BUFFER,         ///< For unrecognized chunk, it may contains any kind of data
	};
}