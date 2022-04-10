#pragma once

#include <string>
#include <vector>


namespace gamelib::prp
{
	class PRPTokenTable
	{
	public:
		PRPTokenTable() = default;
		explicit PRPTokenTable(const std::vector<std::string>& tokenList);

		[[nodiscard]] bool hasToken(const std::string &token) const;
		[[nodiscard]] int indexOf(const std::string &token) const;

		void addToken(const std::string &token);
		void removeToken(const std::string &token);

		static void serialize(const PRPTokenTable& tokenTable, std::vector<uint8_t> &destination);
		static void deserialize(PRPTokenTable& tokenTable, const std::vector<uint8_t> &source, unsigned int expectedTokensCount);

	private:
		std::vector<std::string> m_tokenList;
	};
}