#include <Widgets/TypeRefTabPropertyWidget.h>
#include <Utils/TSpinboxFactory.hpp>
#include <QApplication>
#include <QPainter>

#include <GameLib/Type.h>
#include <GameLib/TypeEnum.h>

// Layout
#include <QVBoxLayout>
#include <QHBoxLayout>

// Trivial widgets
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QCheckBox>
#include <QMenu>

// Models
#include <QStringListModel>

// STL
#include <string>

// Namespaces
using namespace widgets;

using gamelib::prp::PRPInstruction;
using gamelib::prp::PRPOperandVal;
using gamelib::prp::PRPOpCode;

// Constants
static constexpr const char* STRING_ENTRY_ELEMENT_TEMPLATE_ID = "ZREFTAB/StringEntry/%1";
static constexpr const char* CHAR_ENTRY_ELEMENT_TEMPLATE_ID   = "ZREFTAB/CharEntry/%1";
static constexpr const char* BOOL_ENTRY_ELEMENT_TEMPLATE_ID   = "ZREFTAB/BoolEntry/%1";
static constexpr const char* I8_ENTRY_ELEMENT_TEMPLATE_ID     = "ZREFTAB/I8Entry/%1";
static constexpr const char* I16_ENTRY_ELEMENT_TEMPLATE_ID    = "ZREFTAB/I16Entry/%1";
static constexpr const char* I32_ENTRY_ELEMENT_TEMPLATE_ID    = "ZREFTAB/I32Entry/%1";
static constexpr const char* F32_ENTRY_ELEMENT_TEMPLATE_ID    = "ZREFTAB/F32Entry/%1";
static constexpr const char* F64_ENTRY_ELEMENT_TEMPLATE_ID    = "ZREFTAB/F64Entry/%1";

// Enums
enum class InternalDataClass
{
	Integer, // i8..i32
	Float32, // f32
	Float64, // f64
	String,  // generic string
	Char,    // char
	Boolean  // true/false
};

enum class IntegerSubClass
{
	I8, I16, I32, // From Integer
};

namespace {
	std::optional<InternalDataClass> getContainerEntryClass(const types::QGlacierValue& gVal)
	{
		const auto& instructions = gVal.instructions;

		if (instructions.size() <= 1 || !instructions.at(0).isContainer())
		{
			return std::nullopt;
		}

		if (instructions.size() >= 2)
		{
			// Verify that all types are same
			for (int i = 2; i < instructions.size() - 1; ++i)
			{
				if (instructions.at(1).getOpCode() != instructions.at(i).getOpCode())
				{
					qt_assert("instructions.at(0).getOpCode() == instructions.at(i).getOpCode(): bad container instance!", __FILE__, __LINE__);
					return std::nullopt;
				}
			}
		}

		switch (instructions.at(1).getOpCode())
		{
			case PRPOpCode::String:
			case PRPOpCode::NamedString:
				return InternalDataClass::String;
		    case PRPOpCode::Char:
		    case PRPOpCode::NamedChar:
			    return InternalDataClass::Char;
		    case PRPOpCode::Bool:
		    case PRPOpCode::NamedBool:
			    return InternalDataClass::Boolean;
		    case PRPOpCode::Float32:
		    case PRPOpCode::NamedFloat32:
			    return InternalDataClass::Float32;
		    case PRPOpCode::Float64:
		    case PRPOpCode::NamedFloat64:
			    return InternalDataClass::Float64;
		    case PRPOpCode::Int8:
		    case PRPOpCode::NamedInt8:
		    case PRPOpCode::Int16:
		    case PRPOpCode::NamedInt16:
		    case PRPOpCode::Int32:
		    case PRPOpCode::NamedInt32:
			    return InternalDataClass::Integer;
		    default:
			    return std::nullopt;
		}

		return std::nullopt;
	}

	std::optional<IntegerSubClass> getContainerEntryClassOfIntegerSubClass(const types::QGlacierValue& gVal)
	{
		const auto& instructions = gVal.instructions;

		if (instructions.size() <= 1 || !instructions.at(0).isContainer())
		{
			return std::nullopt;
		}

		if (instructions.size() >= 2)
		{
			// Verify that all types are same
			for (int i = 2; i < instructions.size() - 1; ++i)
			{
				if (instructions.at(1).getOpCode() != instructions.at(i).getOpCode())
				{
					qt_assert("instructions.at(0).getOpCode() == instructions.at(i).getOpCode(): bad container instance!", __FILE__, __LINE__);
					return std::nullopt;
				}
			}
		}

		switch (instructions.at(1).getOpCode())
		{
		case PRPOpCode::Int8:
		case PRPOpCode::NamedInt8:
			return IntegerSubClass::I8;
		case PRPOpCode::Int16:
		case PRPOpCode::NamedInt16:
			return IntegerSubClass::I16;
		case PRPOpCode::Int32:
		case PRPOpCode::NamedInt32:
			return IntegerSubClass::I32;
		default:
			return std::nullopt;
		}

		return std::nullopt;
	}

	std::optional<gamelib::prp::PRPInstruction> constructDefaultInstructionForContainerEntry(const types::QGlacierValue &gVal)
	{
		const auto kindOpt = getContainerEntryClass(gVal);
		if (!kindOpt.has_value())
		{
			return std::nullopt;
		}

		switch (kindOpt.value())
		{
			case InternalDataClass::Integer:
			{
			    const auto intClassOpt = getContainerEntryClassOfIntegerSubClass(gVal);
			    if (!intClassOpt.has_value())
			    {
				    return std::nullopt;
			    }

			    switch (intClassOpt.value())
			    {
				    case IntegerSubClass::I8:
				        return gamelib::prp::PRPInstruction(gamelib::prp::PRPOpCode::Int8, gamelib::prp::PRPOperandVal(0));
				    case IntegerSubClass::I16:
				        return gamelib::prp::PRPInstruction(gamelib::prp::PRPOpCode::Int16, gamelib::prp::PRPOperandVal(0));
				    case IntegerSubClass::I32:
				        return gamelib::prp::PRPInstruction(gamelib::prp::PRPOpCode::Int32, gamelib::prp::PRPOperandVal(0));
			    }
		    }
			case InternalDataClass::Float32:
			    return gamelib::prp::PRPInstruction(gamelib::prp::PRPOpCode::Float32, gamelib::prp::PRPOperandVal(0.0f));
			case InternalDataClass::Float64:
			    return gamelib::prp::PRPInstruction(gamelib::prp::PRPOpCode::Float64, gamelib::prp::PRPOperandVal(0.0));
			case InternalDataClass::String:
			    return gamelib::prp::PRPInstruction(gamelib::prp::PRPOpCode::String, gamelib::prp::PRPOperandVal(std::string()));
			case InternalDataClass::Char:
			    return gamelib::prp::PRPInstruction(gamelib::prp::PRPOpCode::Char, gamelib::prp::PRPOperandVal(' '));
			case InternalDataClass::Boolean:
			    return gamelib::prp::PRPInstruction(gamelib::prp::PRPOpCode::Bool, gamelib::prp::PRPOperandVal(false));
		}

		return std::nullopt;
	}
}


void TypeRefTabPropertyWidget::buildLayout(const types::QGlacierValue &value)
{
	const bool isEmpty = value.instructions.empty() || value.instructions.size() == 1;
	if (!isEmpty)
	{
		createLayout(value);
	}
	else
	{
		// Build layout with ability to select target type (container of integers, strings, booleans or other trivial types)
		createLayoutForEmptyContainer(value);
	}

	if (auto layout = this->layout())
	{
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(0);
	}
}

void TypeRefTabPropertyWidget::updateLayout(const types::QGlacierValue &value)
{
	if (value.instructions.empty() || value.instructions.size() == 1) return;

	if (value.instructions.size() != m_value.instructions.size())
	{
		const bool hasNewData = value.instructions.size() > m_value.instructions.size();

		if (hasNewData)
		{
			// Create new elements
			auto rootLayout = qobject_cast<QVBoxLayout*>(layout());
			rootLayout->insertLayout(static_cast<int>(rootLayout->children().size()), createLayoutForEntry(value, static_cast<int>(value.instructions.size() - 1)));
		}
		// else we made removal actions, that case was skipped before
	}
	else
	{
		const auto entOpt = getContainerEntryClass(value);
		if (!entOpt.has_value())
		{
			qt_assert("entOpt.has_value()", __FILE__, __LINE__);
			return;
		}

		const auto& entKind = entOpt.value();

		switch (entKind)
		{
			case InternalDataClass::String:
			{
			    for (int i = 0; i < value.instructions.size() - 1; ++i)
			    {
				    auto id = QString(STRING_ENTRY_ELEMENT_TEMPLATE_ID).arg(i);

				    if (auto entryEditor = layout()->findChild<QLineEdit*>(id); entryEditor != nullptr)
				    {
					    QSignalBlocker blocker { entryEditor };
					    entryEditor->setText(QString::fromStdString(value.instructions.at(i + 1).getOperand().get<const std::string&>()));
				    }
			    }
		    }
			break;
			case InternalDataClass::Char:
		    {
			    for (int i = 0; i < value.instructions.size() - 1; ++i)
			    {
				    auto id = QString(CHAR_ENTRY_ELEMENT_TEMPLATE_ID).arg(i);

				    if (auto entryEditor = layout()->findChild<QLineEdit*>(id); entryEditor != nullptr)
				    {
					    QSignalBlocker blocker { entryEditor };
					    entryEditor->setText(QString(value.instructions.at(i + 1).getOperand().get<char>()));
				    }
			    }
		    }
		    break;
		    case InternalDataClass::Boolean:
		    {
			    for (int i = 0; i < value.instructions.size() - 1; ++i)
			    {
				    auto id = QString(BOOL_ENTRY_ELEMENT_TEMPLATE_ID).arg(i);

				    if (auto entryEditor = layout()->findChild<QCheckBox*>(id); entryEditor != nullptr)
				    {
					    QSignalBlocker blocker { entryEditor };
					    entryEditor->setChecked(value.instructions.at(i + 1).getOperand().get<bool>());
				    }
			    }
		    }
		    break;
			case InternalDataClass::Integer:
		    {
			    const auto intSubClassOpt = getContainerEntryClassOfIntegerSubClass(value);
			    if (!intSubClassOpt.has_value())
			    {
				    qt_assert("intSubClassOpt.has_value()", __FILE__, __LINE__);
				    return;
			    }

			    for (int i = 0; i < value.instructions.size() - 1; ++i)
			    {
				    switch (intSubClassOpt.value())
				    {
					    case IntegerSubClass::I8:
				        {
					        auto id = QString(I8_ENTRY_ELEMENT_TEMPLATE_ID).arg(i);
					        utils::TSpinboxFactory<std::int8_t>::updateValue(id, value, i + 1, this);
				        }
					    break;
				        case IntegerSubClass::I16:
				        {
					        auto id = QString(I16_ENTRY_ELEMENT_TEMPLATE_ID).arg(i);
					        utils::TSpinboxFactory<std::int16_t>::updateValue(id, value, i + 1, this);
				        }
				        break;
					    case IntegerSubClass::I32:
				        {
					        auto id = QString(I32_ENTRY_ELEMENT_TEMPLATE_ID).arg(i);
					        utils::TSpinboxFactory<std::int32_t>::updateValue(id, value, i + 1, this);
				        }
				        break;
				    }
			    }
		    }
		    break;
			case InternalDataClass::Float32:
		    {
			    for (int i = 0; i < value.instructions.size() - 1; ++i)
			    {
				    auto id = QString(F32_ENTRY_ELEMENT_TEMPLATE_ID).arg(i);
				    utils::TSpinboxFactory<float>::updateValue(id, value, i + 1, this);
			    }
		    }
		    break;
			case InternalDataClass::Float64:
		    {
			    for (int i = 0; i < value.instructions.size() - 1; ++i)
			    {
				    auto id = QString(F64_ENTRY_ELEMENT_TEMPLATE_ID).arg(i);
				    utils::TSpinboxFactory<double>::updateValue(id, value, i + 1, this);
			    }
		    }
		    break;
		}
	}
}

void TypeRefTabPropertyWidget::createLayout(const types::QGlacierValue &value)
{
	auto layout = new QVBoxLayout(this);
	const auto capacity = value.instructions.at(0).getOperand().get<std::int32_t>();

	const auto entryKindOpt = getContainerEntryClass(value);
	if (!entryKindOpt.has_value())
	{
		qt_assert("entryKindOpt.has_value()", __FILE__, __LINE__);
		return;
	}

	const auto& entryKind = entryKindOpt.value();

	for (int i = 0; i < capacity; ++i)
	{
		layout->addLayout(createLayoutForEntry(value, i + 1));
	}

	layout->addLayout(createLayoutForFooter());
}

void TypeRefTabPropertyWidget::createLayoutForEmptyContainer(const types::QGlacierValue &value)
{
	auto rootLayout = new QVBoxLayout(this);
	auto createEntryButton = new QPushButton("Add Entry", this);
	createEntryButton->setContextMenuPolicy(Qt::CustomContextMenu);
	createEntryButton->setIcon(QIcon(":/bmedit/add_icon.png"));
	rootLayout->addWidget(createEntryButton);
	setLayout(rootLayout);

	connect(createEntryButton, SIGNAL(clicked()), this, SLOT(showAddEntriesContextMenu()));
}

void TypeRefTabPropertyWidget::paintPreview(QPainter *painter, const QStyleOptionViewItem &option, const types::QGlacierValue &data)
{
	if (data.instructions.empty() || data.instructions.at(0).getOperand().get<std::int32_t>() == 0u)
	{
		QTextOption textOptions;
		textOptions.setAlignment(Qt::AlignCenter);
		painter->drawText(option.rect, QString("(Empty RefTab)"), textOptions);
		return;
	}

	// Container of strings
	auto capacity = data.instructions.at(0).getOperand().get<std::int32_t>();
	QStringList stringList;
	stringList.reserve(capacity);

	const auto& entOpt = getContainerEntryClass(data);
	if (!entOpt.has_value())
	{
		qt_assert("entOpt.has_value()", __FILE__, __LINE__);
		return;
	}

	const auto& entKind = entOpt.value();

	for (int i = 0; i < capacity; ++i)
	{
		const auto& ip = data.instructions.at(1);

		if (entKind == InternalDataClass::String)
		{
			stringList.push_back(QString("[%1] '%2'").arg(i + 1).arg(QString::fromStdString(data.instructions.at(i + 1).getOperand().get<const std::string&>())));
		}
		else if (entKind == InternalDataClass::Char)
		{
			stringList.push_back(QString("[%1] '%2'").arg(i + 1).arg(data.instructions.at(i + 1).getOperand().get<char>()));
		}
		else if (entKind == InternalDataClass::Float32)
		{
			stringList.push_back(QString("[%1] %2").arg(i + 1).arg(data.instructions.at(i + 1).getOperand().get<float>()));
		}
		else if (entKind == InternalDataClass::Float64)
		{
			stringList.push_back(QString("[%1] %2").arg(i + 1).arg(data.instructions.at(i + 1).getOperand().get<double>()));
		}
		else if (entKind == InternalDataClass::Integer)
		{
			stringList.push_back(QString("[%1] %2").arg(i + 1).arg(data.instructions.at(i + 1).getOperand().get<std::int32_t>()));
		}
		else if (entKind == InternalDataClass::Boolean)
		{
			stringList.push_back(QString("[%1] %2").arg(i + 1).arg(data.instructions.at(i + 1).getOperand().get<bool>()));
		}
	}

	QTextOption textOptions;
	textOptions.setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	painter->drawText(option.rect, stringList.join('\n'), textOptions);
}

QLayout *TypeRefTabPropertyWidget::createLayoutForFooter()
{
	// Add element button
    auto newLay = new QVBoxLayout(this);

	auto addNewEntryButton = new QPushButton(QIcon(":/bmedit/add_icon.png"), QString("Add entry"), this);
	connect(addNewEntryButton, &QPushButton::clicked, [this]() {
		auto newSnapshot = m_value;

		// Add value
		const auto& firstEntryInstruction = newSnapshot.instructions.at(1);
		const auto newInstructionOpt = constructDefaultInstructionForContainerEntry(newSnapshot);
		if (!newInstructionOpt.has_value())
		{
			qt_assert("newInstructionOpt.has_value()", __FILE__, __LINE__);
			return;
		}

		newSnapshot.instructions.push_back(newInstructionOpt.value());

		// Update capacity
		const auto oldOpCode   = m_value.instructions.at(0).getOpCode();
		const auto newCapacity = m_value.instructions.at(0).getOperand().get<std::int32_t>() + 1;
		newSnapshot.instructions[0] = gamelib::prp::PRPInstruction(oldOpCode, gamelib::prp::PRPOperandVal(newCapacity));

		// Update & sync data
		m_value = newSnapshot;
		emit editFinished();
	});

	newLay->addWidget(addNewEntryButton);

	return newLay;
}

QLayout *TypeRefTabPropertyWidget::createLayoutForEntry(const types::QGlacierValue &value, int i)
{
	auto localLayout = new QHBoxLayout(this);

	// Index label
	{
		auto entryIndexLabel = new QLabel(QString("[%1] ").arg(i), this);
		localLayout->addWidget(entryIndexLabel);
	}

	// Value widget
	{
		const auto entOpt = getContainerEntryClass(value);
		if (!entOpt.has_value())
		{
			qt_assert("entOpt.has_value()", __FILE__, __LINE__);
			delete localLayout;
			return nullptr;
		}

		const auto& entKind = entOpt.value();

		switch (entKind)
		{
			case InternalDataClass::String:
			{
			    auto entryEditorWidget = new QLineEdit(QString::fromStdString(value.instructions.at(i).getOperand().get<const std::string&>()), this);
			    connect(entryEditorWidget, &QLineEdit::textChanged, [this, instructionIndex = i](const QString& newStr) {
				    m_value.instructions[instructionIndex] = gamelib::prp::PRPInstruction(PRPOpCode::String, PRPOperandVal(newStr.toStdString()));
				    emit valueChanged();
			    });
			    entryEditorWidget->setAccessibleName(QString(STRING_ENTRY_ELEMENT_TEMPLATE_ID).arg(i));
			    localLayout->addWidget(entryEditorWidget);
		    }
			break;
		    case InternalDataClass::Char:
		    {
			    auto entryEditorWidget = new QLineEdit(QString::fromStdString(value.instructions.at(i).getOperand().get<const std::string&>()), this);
			    entryEditorWidget->setMaxLength(1);
			    connect(entryEditorWidget, &QLineEdit::textChanged, [this, instructionIndex = i](const QString& newStr) {
				    if (newStr.length() < 1)
				    {
					    return;
				    }

				    m_value.instructions[instructionIndex] = gamelib::prp::PRPInstruction(PRPOpCode::String, PRPOperandVal(newStr.toStdString()[0]));
				    emit valueChanged();
			    });
			    entryEditorWidget->setAccessibleName(QString(CHAR_ENTRY_ELEMENT_TEMPLATE_ID).arg(i));
			    localLayout->addWidget(entryEditorWidget);
		    }
		    break;
		    case InternalDataClass::Boolean:
		    {
			    auto entryEditorWidget = new QCheckBox(QString("Value #%1").arg(i), this);
			    entryEditorWidget->setChecked(value.instructions.at(i).getOperand().get<bool>());
			    connect(entryEditorWidget, &QCheckBox::stateChanged, [this, i](int state) {
				    m_value.instructions[i] = gamelib::prp::PRPInstruction(PRPOpCode::Bool, PRPOperandVal(state == Qt::CheckState::Checked));
				    emit valueChanged();
			    });
			    entryEditorWidget->setAccessibleName(QString(BOOL_ENTRY_ELEMENT_TEMPLATE_ID).arg(i));
			    localLayout->addWidget(entryEditorWidget);
		    }
		    break;
			case InternalDataClass::Integer:
		    {
			    const auto entIntSubClassOpt = getContainerEntryClassOfIntegerSubClass(value);
			    if (!entIntSubClassOpt.has_value())
			    {
				    qt_assert("entIntSubClassOpt.has_value()", __FILE__, __LINE__);
				    delete localLayout;
				    return nullptr;
			    }

			    switch (entIntSubClassOpt.value())
			    {
				    case IntegerSubClass::I8:
			        {
				        utils::TSpinboxFactory<std::int8_t>::createAndSetup(QString(I8_ENTRY_ELEMENT_TEMPLATE_ID).arg(i), value, i, this, localLayout, [this](int entryIdx, int8_t newValue) {
					        m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
					        emit valueChanged();
				        });
			        }
			        break;
				    case IntegerSubClass::I16:
				        utils::TSpinboxFactory<std::int16_t>::createAndSetup(QString(I16_ENTRY_ELEMENT_TEMPLATE_ID).arg(i), value, i, this, localLayout, [this](int entryIdx, int16_t newValue) {
					        m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
					        emit valueChanged();
				        });
			        break;
				    case IntegerSubClass::I32:
				        utils::TSpinboxFactory<std::int32_t>::createAndSetup(QString(I32_ENTRY_ELEMENT_TEMPLATE_ID).arg(i), value, i, this, localLayout, [this](int entryIdx, int32_t newValue) {
					        m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
					        emit valueChanged();
				        });
			        break;
			    }
		    }
		    break;
			case InternalDataClass::Float32:
			    utils::TSpinboxFactory<float>::createAndSetup(QString(F32_ENTRY_ELEMENT_TEMPLATE_ID).arg(i), value, i, this, localLayout, [this](int entryIdx, float newValue) {
				    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
				    emit valueChanged();
			    });
		    break;
			case InternalDataClass::Float64:
			    utils::TSpinboxFactory<double>::createAndSetup(QString(F32_ENTRY_ELEMENT_TEMPLATE_ID).arg(i), value, i, this, localLayout, [this](int entryIdx, double newValue) {
				    m_value.instructions[entryIdx] = PRPInstruction(m_value.instructions[entryIdx].getOpCode(), PRPOperandVal(newValue));
				    emit valueChanged();
			    });
		    break;
		}
	}

	// Remove button
	{
		auto removeEntryButton = new QPushButton(QString(), this);
		auto removeButtonIcon = QIcon(":/bmedit/remove_icon.png");
		removeEntryButton->setIcon(removeButtonIcon);
		removeEntryButton->setIconSize(QSize(25, 25));
		connect(removeEntryButton, &QPushButton::clicked, [this, instructionIndex = (i)]() {
			auto newSnapshot = m_value;

			// Remove entry
			newSnapshot.instructions.erase(newSnapshot.instructions.begin() + instructionIndex);

			// Update capacity
			const auto oldOpCode   = m_value.instructions.at(0).getOpCode();
			const auto newCapacity = m_value.instructions.at(0).getOperand().get<std::int32_t>() - 1;
			newSnapshot.instructions[0] = gamelib::prp::PRPInstruction(oldOpCode, gamelib::prp::PRPOperandVal(newCapacity));

			// Update & sync data
			m_value = newSnapshot;
			emit valueChanged();
			emit editFinished();
		});

		localLayout->addWidget(removeEntryButton);
	}

	return localLayout;
}

namespace {
	const QString Action_String   = "String";
	const QString Action_Bool     = "Bool";
	const QString Action_Char     = "Char";
	const QString Action_Int8     = "Int8";
	const QString Action_Int16    = "Int16";
	const QString Action_Int32    = "Int32";
	const QString Action_Float32  = "Float32";
	const QString Action_Float64  = "Float64";
	//TODO: Support Enum
}

void TypeRefTabPropertyWidget::showAddEntriesContextMenu()
{
	auto entryButton = qobject_cast<QPushButton*>(sender());

	QPoint globalPos = entryButton->mapToGlobal(entryButton->contentsRect().center());
	QMenu menu;
	menu.addAction(Action_Bool,     this, &TypeRefTabPropertyWidget::addEntryFromContextMenu);
	menu.addAction(Action_Char,     this, &TypeRefTabPropertyWidget::addEntryFromContextMenu);
	menu.addAction(Action_Int8,     this, &TypeRefTabPropertyWidget::addEntryFromContextMenu);
	menu.addAction(Action_Int16,    this, &TypeRefTabPropertyWidget::addEntryFromContextMenu);
	menu.addAction(Action_Int32,    this, &TypeRefTabPropertyWidget::addEntryFromContextMenu);
	menu.addAction(Action_Float32,  this, &TypeRefTabPropertyWidget::addEntryFromContextMenu);
	menu.addAction(Action_Float64,  this, &TypeRefTabPropertyWidget::addEntryFromContextMenu);
	menu.addAction(Action_String,   this, &TypeRefTabPropertyWidget::addEntryFromContextMenu);
	menu.exec(globalPos);
}

void TypeRefTabPropertyWidget::addEntryFromContextMenu()
{
	auto emitter = qobject_cast<QAction*>(sender());
	auto actionId = emitter->text();

	auto newSnapshot = m_value;

	// Increase capacity
	{
		if (newSnapshot.instructions.empty())
		{
			newSnapshot.instructions.emplace_back(gamelib::prp::PRPOpCode::Container, gamelib::prp::PRPOperandVal(int32_t(1)));
		}
		else
		{
			const auto capacity = newSnapshot.instructions.at(0).getOperand().get<std::int32_t>() + 1;
			newSnapshot.instructions[0] = gamelib::prp::PRPInstruction(gamelib::prp::PRPOpCode::Container, gamelib::prp::PRPOperandVal(capacity));
		}
	}

#define PREPARE_TYPE_ACTION_DECL(type, prpType, val) (actionId == (type)) { newSnapshot.instructions.emplace_back(gamelib::prp::PRPInstruction((prpType), gamelib::prp::PRPOperandVal((val)))); }

	if      PREPARE_TYPE_ACTION_DECL(Action_Bool,    gamelib::prp::PRPOpCode::Bool,  false)
	else if PREPARE_TYPE_ACTION_DECL(Action_Char,    gamelib::prp::PRPOpCode::Char,  ' ')
	else if PREPARE_TYPE_ACTION_DECL(Action_Int8,    gamelib::prp::PRPOpCode::Int8,  0)
	else if PREPARE_TYPE_ACTION_DECL(Action_Int16,   gamelib::prp::PRPOpCode::Int16, 0)
	else if PREPARE_TYPE_ACTION_DECL(Action_Int32,   gamelib::prp::PRPOpCode::Int32, 0)
	else if PREPARE_TYPE_ACTION_DECL(Action_Float32, gamelib::prp::PRPOpCode::Float32, 0.0f)
	else if PREPARE_TYPE_ACTION_DECL(Action_Float64, gamelib::prp::PRPOpCode::Float64, 0.0)
	else if PREPARE_TYPE_ACTION_DECL(Action_String,  gamelib::prp::PRPOpCode::String, " ")
	else
	{
		// Invalid case!
		qt_assert("Unsupported actionId!", __FILE__, __LINE__);
		return;
	}

	// Update layout
	updateLayout(newSnapshot);

	// Update & sync data
	m_value = newSnapshot;
	emit editFinished();

#undef ON_TYPE_ACTION_DECL
}