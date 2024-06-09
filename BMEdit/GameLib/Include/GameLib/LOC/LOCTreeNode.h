#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace ZBio::ZBinaryWriter
{
	class BinaryWriter;
}

namespace gamelib::loc
{
	enum LOCTreeNodeType : int8_t
	{
		EMPTY_BLOCK = 0x8, ///< Empty chunk, no data at all
		LOCALIZED_STRING = 0x9,   ///< Key value (string to aligned string)
		CHILDREN = 0x10, ///< Container (amount & list of offsets)
		SUBTITLES = 0x29, ///< Subtitles value (long text with extra parameters) | 0x20 mask means that extra data exists
	};

	enum LOCMissionObjectiveType : char
	{
		HBM_Target = 'T',
		HBM_Retrieve = 'R',
		HBM_Escape = 'E',
		HBM_Dispose = 'D',
		HBM_Protect = 'P',
		HBM_Optional = 'O'
	};

	/**
	 * Notes:
	 *
	 * 1. Decompiler ref: https://github.com/ReGlacier/ReHitmanTools/blob/main/Tools/LOCC/source/Decompiler.cpp
	 * 2. Format notes
	 * File format;
	 * 		1. Root node name omitted, starts from amount of nodes [u8]
	 * 		2. If amount more than 1 - offsets segment starts there
	 *
	 * Node format:
	 * 		Name - CString
	 * 		+0x0 - kind of node (TreeNodeType : 0x0 - value or data (leaf), 0x10 - node with children (bone)
	 * 		+0x1 - how much child nodes coming (but +1)
	 * 		Then going list of offsets by 0x4 bytes each. Each offset starts after offsets block
	 *
	 * We should read
	 */
	struct LOCTreeNode
	{
		using Ptr = std::shared_ptr<LOCTreeNode>;
		using Ref = std::weak_ptr<LOCTreeNode>;

		std::vector<LOCTreeNode::Ptr> children {};  // When NODE_WITH_CHILDREN
		LOCTreeNode::Ref parent {};
		LOCTreeNodeType type { LOCTreeNodeType::LOCALIZED_STRING };
		std::string name {};
		std::string value {}; // When SIMPLE_VALUE or PARAGRAPH_VALUE
		struct SubtitleData
		{
			uint8_t unkData[8];
		} subtitle;

		static void deserialize(const LOCTreeNode::Ptr &node, ZBio::ZBinaryReader::BinaryReader* binaryReader);
		static void serialize(const LOCTreeNode::Ptr& node, ZBio::ZBinaryWriter::BinaryWriter* binaryWriter);
	};
}