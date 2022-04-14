#pragma once

namespace ZBio::ZBinaryReader
{
	class BinaryReader;
}

namespace gamelib::gms
{
	class GMSHeader
	{
	public:
		GMSHeader();

		static void deserialize(GMSHeader &header, const ZBio::ZBinaryReader::BinaryReader *binaryReader);

	private:
		//https://github.com/ReGlacier/ReHitmanTools/issues/3#issuecomment-769654029
	};
}