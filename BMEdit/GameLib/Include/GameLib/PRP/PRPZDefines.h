#pragma once

#include <cstdint>
#include <vector>

#include <GameLib/PRP/PRPDefinition.h>
#include <GameLib/PRP/PRPDefinitionType.h>
#include <GameLib/PRP/PRPTokenTable.h>


namespace ZBio::ZBinaryWriter
{
	class BinaryWriter;
}

namespace gamelib::prp {
	class PRPZDefines {
	public:
		struct ReadResult {
			bool isOk { false };
			int64_t lastOffset { 0 };

			[[nodiscard]] explicit operator bool() const noexcept { return isOk; }
		};

		PRPZDefines() = default;

		void read(const uint8_t *data, int64_t size, const PRPTokenTable *tokenTable, ReadResult& result);

		[[nodiscard]] const std::vector<PRPDefinition> &getDefinitions() const;

		static void serialize(const PRPZDefines &defines, const PRPTokenTable *tokenTable, ZBio::ZBinaryWriter::BinaryWriter *binaryWriter);

	private:
		std::vector<PRPDefinition> m_definitions;
	};
}