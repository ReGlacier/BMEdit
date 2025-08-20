#pragma once

#include <GameLib/MAT/MATEntries.h>
#include <GameLib/MAT/MATBind.h>
#include <string>
#include <vector>


namespace gamelib::mat
{
	class MATInstance
	{
	public:
		MATInstance(std::string instanceName, std::string parentClassName, std::vector<MATPropertyEntry>&& properties, std::vector<MATBind>&& binders);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] const std::string& getParentName() const;
		[[nodiscard]] const std::vector<MATBind>& getBinders() const;

	private:
		std::string m_name {};
		std::string m_parentName {};
		std::vector<MATPropertyEntry> m_properties {};
		std::vector<MATBind> m_binders {};
	};
}