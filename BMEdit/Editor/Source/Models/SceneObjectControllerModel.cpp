#include <Models/SceneObjectControllerModel.h>
#include <GameLib/PRP/PRPMathTypes.h>
#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeComplex.h>
#include <string_view>

using namespace models;


static constexpr std::string_view kScriptC = "ZScriptC";
static const std::string kScriptNamePN = "ScriptName";

SceneObjectControllerModel::SceneObjectControllerModel(QObject *parent)
	: ValueModelBase(parent)
{
	connect(this, &ValueModelBase::valueChanged, [this]() { onValueChanged(); });
}

void SceneObjectControllerModel::setGeom(gamelib::scene::SceneObject *geom)
{
	const bool isNewGeom = m_geom != geom;
	beginResetModel();

	m_specialRows.clear();
	m_geom = geom;
	m_currentControllerIndex = kUnset;
	endResetModel();

	if (isNewGeom)
	{
		resetValue();
	}
}

void SceneObjectControllerModel::resetGeom()
{
	beginResetModel();

	// and after that we've ready to do smth else
	m_specialRows.clear();
	m_geom = nullptr;
	m_currentControllerIndex = kUnset;
	endResetModel();

	resetValue();
}

void SceneObjectControllerModel::setControllerIndex(int controllerIndex)
{
	if (!m_geom || controllerIndex < 0 || controllerIndex >= m_geom->getControllers().size() || m_currentControllerIndex == controllerIndex)
	{
		return;
	}

	auto& controller = m_geom->getControllers().at(controllerIndex);

	beginResetModel();

	// reset special rows info
	m_specialRows.clear();

	// reset controller index
	m_currentControllerIndex = controllerIndex;
	endResetModel();

	if (controller.type->getName() == kScriptC)
	{
		// Ok, need handle this correctly
		gamelib::Value proxy { controller.properties };

		// And here we need to update proxy views (just add mappings)
		addSugarViews(controller.type, proxy, proxy.getObject<std::string>(kScriptNamePN));

		// And store it here
		setValue(proxy);
	}
	else
	{
		// Default way
		setValue(controller.properties);
	}
}

void SceneObjectControllerModel::resetController()
{
	beginResetModel();
	m_currentControllerIndex = kUnset;
	endResetModel();

	resetValue();
}

QVariant SceneObjectControllerModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::BackgroundRole)
	{
		for (const auto& specialRow : m_specialRows)
		{
			if (specialRow.includes(index.row()))
			{
				return specialRow.backgroundColor;
			}
		}
	}

	// other requests redirect to root logic
	return ValueModelBase::data(index, role);
}

void SceneObjectControllerModel::addSugarViews(const gamelib::Type* pControllerType, gamelib::Value &v, const std::string &scriptName)
{
	// Ok, it must be pretty easy. First of all we need to find a script description in TypeRegistry
	// Then we need to copy all unexposed instructions to 'v' bucket and add views for VARIABLE entries
	const auto asDefault = pControllerType->makeDefaultPropertiesPack(); //TODO: Less hacks, please
	const int baseViewsNr = asDefault.getEntries().size();
	const int baseSize = asDefault.getInstructions().size();

	SpecialRow row {};
	row.endRow = row.startRow = baseSize;
	row.backgroundColor = QColor(26, 188, 156);

	if (auto scriptInfo = gamelib::TypeRegistry::getInstance().getScriptInfo(scriptName); scriptInfo.has_value())
	{
		// Ok, let's insert that data
		for (const auto& ent : scriptInfo.value().entries)
		{
			// need to rebase this thing
			gamelib::ValueEntry temp = ent;
			temp.instructions.iOffset += baseSize;
			v += temp;
			++row.endRow; // I'm not sure that +1 is enough here, but += temp.instructions.iSize is not valid too!
		}

		// save new row
		m_specialRows.emplace_back(row);
	}
}

void SceneObjectControllerModel::removeSugarViews(const gamelib::Type* pControllerType, gamelib::Value &v)
{
	// Just reset views & entries from owner type
	if (pControllerType->getKind() == gamelib::TypeKind::COMPLEX)
	{
		v.removeEntriesAndViewsSince(
		    pControllerType->makeDefaultPropertiesPack().getEntries().size()
		);
	}
}

void SceneObjectControllerModel::onValueChanged()
{
	if (m_geom && m_currentControllerIndex != kUnset && m_currentControllerIndex >= 0 && m_currentControllerIndex < m_geom->getControllers().size() && getValue().has_value())
	{
		auto& controller = m_geom->getControllers().at(m_currentControllerIndex);

		if (controller.type->getName() == kScriptC)
		{
			// Here we need to drop our modified views and store 'typed' original views
			gamelib::Value proxy = getValue().value();

			const bool bScriptChanged = controller.properties.getObject<std::string>(kScriptNamePN) != proxy.getObject<std::string>(kScriptNamePN);
			const std::string kNewScriptName = proxy.getObject<std::string>(kScriptNamePN);

			if (bScriptChanged && gamelib::TypeRegistry::getInstance().hasScriptInfo(kNewScriptName))
			{
				// Take original first instruction
				std::vector<gamelib::prp::PRPInstruction> instructions = {
				    getValue().value().getInstructions()[0]
				};

				// Nice! Now we've ready to append a new bunch of properties
				const auto scriptInfo = gamelib::TypeRegistry::getInstance().getScriptInfo(kNewScriptName);

				for (const auto& instruction : scriptInfo->initialInstructions)
				{
					instructions.push_back(instruction);
				}

				// And remember: don't forget about SkipMark opcode. Game iterating until not see this opcode and will crash
				instructions.emplace_back(gamelib::prp::PRPOpCode::SkipMark);

				// Create new properties bucket
				gamelib::Value newProperties { controller.type, instructions };

				// Add 1 entry (original property markup)
				newProperties += getValue().value().getEntries()[0];

				// Save
				controller.properties = newProperties;

				// And add extra stubs
				for (const auto& ent : scriptInfo->entries)
					newProperties += ent;

				// Done
				setValue(newProperties);
			}
			else
			{
				// Desugar this stub
				removeSugarViews(controller.type, proxy);

				// Save
				controller.properties = proxy;
			}
		}
		else
		{
			// Default way
			controller.properties = getValue().value();
		}
	}
}