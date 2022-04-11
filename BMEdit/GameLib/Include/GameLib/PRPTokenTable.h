#pragma once

#include <string>
#include <vector>


namespace ZBio::ZBinaryWriter
{
	class BinaryWriter;
}

namespace gamelib::prp
{
	class PRPTokenTable
	{
	public:
		PRPTokenTable() = default;
		PRPTokenTable(const uint8_t *data, int64_t size, int tokenCount);

		[[nodiscard]] bool hasToken(const std::string &token) const;
		[[nodiscard]] int indexOf(const std::string &token) const;
		[[nodiscard]] bool hasIndex(int index) const;
		[[nodiscard]] const std::string& tokenAt(uint32_t index) const;
		[[nodiscard]] int getTokenCount() const;
		[[nodiscard]] int getNonEmptyTokenCount() const;

		bool addToken(const std::string &token);
		void removeToken(const std::string &token);

		static void serialize(const PRPTokenTable& tokenTable, ZBio::ZBinaryWriter::BinaryWriter *writerStream);
		static void deserialize(PRPTokenTable& tokenTable, const std::vector<uint8_t> &source, unsigned int expectedTokensCount);

	private:
		std::vector<std::string> m_tokenList;
	};
}