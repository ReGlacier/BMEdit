#include <GameLib/PRPTokenTable.h>
#include <ZBinaryWriter.hpp>
#include <ZBinaryReader.hpp>
#include <algorithm>
#include <utility>


namespace gamelib::prp {
	PRPTokenTable::PRPTokenTable(const std::vector<std::string> &tokenList)
	{
		m_tokenList.reserve(tokenList.size());

		//That's not good enough but who cares
		//TODO: Optimize if it will be required
		for (const auto &token: tokenList) {
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

	void PRPTokenTable::addToken(const std::string &token)
	{
		if (hasToken(token)) {
			return;
		}

		m_tokenList.push_back(token);
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

	void PRPTokenTable::serialize(const PRPTokenTable &tokenTable, std::vector<uint8_t> &destination)
	{
		using namespace ZBio;

		auto bufferSink = std::make_unique<ZBinaryWriter::BufferSink>();
		ZBinaryWriter::BinaryWriter writer { std::move(bufferSink) };

		for (const auto& token: tokenTable.m_tokenList) {
			writer.writeCString(token);
		}

		auto serializedData = writer.release();

		if (serializedData.has_value()) {
			auto &data = serializedData.value();
			std::copy(data.begin(), data.end(), std::back_inserter(destination));
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