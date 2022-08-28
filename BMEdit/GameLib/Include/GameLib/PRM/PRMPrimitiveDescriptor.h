#pragma once

#include <cstdint>


namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::prm
{
	struct PRMPrimitiveDescriptor
	{
		uint32_t declarationOffset; // +0x0
		uint32_t declarationSize; // +0x4 (need to be confirmed! Source: https://github.com/HHCHunter/Hitman-BloodMoney/blob/master/TOOLS/PRMConverter/Source/PRMConvert.cpp#L23)
		bool bShouldBeLoaded; // +0x8
		uint32_t unkC; // +0xC

		static constexpr int kDescriptorSize = 0x10;

		static void deserialize(PRMPrimitiveDescriptor &descriptor, ZBio::ZBinaryReader::BinaryReader *binaryReader);
	};
}