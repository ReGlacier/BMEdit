#include <GameLib/MAT/MATInstance.h>


namespace gamelib::mat
{
	MATInstance::MATInstance(std::string instanceName, std::string parentClassName, std::vector<MATPropertyEntry>&& properties, std::vector<MATBind>&& binders)
		: m_name(std::move(instanceName)), m_parentName(std::move(parentClassName)), m_properties(std::move(properties)), m_binders(std::move(binders))
	{
	}

	const std::string& MATInstance::getName() const
	{
		return m_name;
	}

	const std::string& MATInstance::getParentName() const
	{
		return m_parentName;
	}

	const std::vector<MATBind>& MATInstance::getBinders() const
	{
		return m_binders;
	}
}