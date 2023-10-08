#pragma once

#include <GameLib/MAT/MATEntries.h>
#include <GameLib/MAT/MATSubClass.h>
#include <string>
#include <vector>


namespace gamelib::mat
{
	class MATClass
	{
	public:
		MATClass(std::string clasName, std::string parentClass, std::vector<MATPropertyEntry>&& properties, std::vector<MATSubClass>&& subClasses);

		[[nodiscard]] const std::string& getName() const;
		[[nodiscard]] const std::string& getParentClassName() const;
		[[nodiscard]] const std::vector<MATSubClass>& getSubClasses() const;

	private:
		std::string m_name;
		std::string m_parentClass;
		std::vector<MATPropertyEntry> m_properties;
		std::vector<MATSubClass> m_subClasses;
	};
}