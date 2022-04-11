#include <GameLib/PRPTokenTable.h>
#include <ZBinaryWriter.hpp>
#include <ZBinaryReader.hpp>

#include <algorithm>
#include <utility>


namespace gamelib::prp {
	PRPTokenTable::PRPTokenTable(const uint8_t *data, int64_t size, int tokenCount)
	{
		ZBio::ZBinaryReader::BinaryReader binaryReader { reinterpret_cast<const char*>(data), size };
		for (int i = 0; i <= tokenCount; i++) {
			std::string token = binaryReader.readCString();
			addToken(token);
		}
	}

	bool PRPTokenTable::hasToken(const std::string &token) const
	{
		return indexOf(token) >= 0;
	}

	int PRPTokenTable::indexOf(const std::string &token) const
	{
		auto it = std::find(m_tokenList.begin(), m_tokenList.end(), token);
		if (it == m_tokenList.end())
		{
			return -1;
		}

		return static_cast<int>(it - m_tokenList.begin());
	}

	bool PRPTokenTable::hasIndex(int index) const
	{
		return index >= 0 && index < m_tokenList.size();
	}

	const std::string &PRPTokenTable::tokenAt(uint32_t index) const
	{
		static std::string kInvalidStr;

		if (index < m_tokenList.size()) {
			return m_tokenList[index];
		}

		return kInvalidStr;
	}

	int PRPTokenTable::getTokenCount() const
	{
		return static_cast<int>(m_tokenList.size());
	}

	int PRPTokenTable::getNonEmptyTokenCount() const
	{
		int result = 0;
		for (const auto &tok: m_tokenList)
		{
			result += 1 * (!tok.empty());
		}
		return result;
	}

	bool PRPTokenTable::addToken(const std::string &token)
	{
		if (hasToken(token)) {
			return false;
		}

		m_tokenList.push_back(token);
		return true;
	}

	void PRPTokenTable::removeToken(const std::string &token)
	{
		auto tokenIndex = indexOf(token);
		if (tokenIndex < 0)
		{
			return;
		}

		m_tokenList.erase(m_tokenList.begin() + tokenIndex);
	}

	void PRPTokenTable::serialize(const PRPTokenTable &tokenTable, ZBio::ZBinaryWriter::BinaryWriter *writerStream)
	{
		for (const auto &token: tokenTable.m_tokenList)
		{
			writerStream->writeCString(token);
		}
	}

	void PRPTokenTable::deserialize(PRPTokenTable &tokenTable, const std::vector<uint8_t> &source, unsigned int expectedTokensCount)
	{
		using namespace ZBio;

		auto bufferSink = std::make_unique<ZBinaryReader::BufferSource>(reinterpret_cast<const char*>(source.data()), source.size());
		ZBinaryReader::BinaryReader reader { std::move(bufferSink) };

		for (int i = 0; i < expectedTokensCount; i++) {
			tokenTable.addToken(reader.readCString());
		}
	}
}