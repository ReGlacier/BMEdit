#include <GameLib/MAT/MATClass.h>
#include <utility>


namespace gamelib::mat
{
	MATClass::MATClass(std::string className, std::string parentClass, std::vector<MATPropertyEntry>&& properties, std::vector<MATSubClass>&& subClasses)
	    : m_name(std::move(className)), m_parentClass(std::move(parentClass)), m_properties(std::move(properties)), m_subClasses(std::move(subClasses))
	{
	}

	const std::string& MATClass::getName() const
	{
		return m_name;
	}

	const std::string& MATClass::getParentClassName() const
	{
		return m_parentClass;
	}

	const std::vector<MATSubClass>& MATClass::getSubClasses() const
	{
		return m_subClasses;
	}
}