#pragma once

#include <cstdint>
#include <vector>

#include <GameLib/PRPDefinition.h>
#include <GameLib/PRPDefinitionType.h>
#include <GameLib/PRPTokenTable.h>


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

	private:
		std::vector<PRPDefinition> m_definitions;
	};
}