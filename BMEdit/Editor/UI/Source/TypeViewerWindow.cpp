#include "TypeViewerWindow.h"
#include "ui_TypeViewerWindow.h"

#include <QStringListModel>
#include <QComboBox>

#include <Models/TypePropertiesDataModel.h>
#include <Delegates/TypePropertyItemDelegate.h>

#include <GameLib/TypeRegistry.h>
#include <GameLib/TypeEnum.h>
#include <GameLib/Type.h>



TypeViewerWindow::TypeViewerWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TypeViewerWindow)
{
    ui->setupUi(this);

	///--------------------------------------
	/// MODELS
	///--------------------------------------
    QStringList allAvailableTypes;

    gamelib::TypeRegistry::getInstance().forEachType([&allAvailableTypes](const gamelib::Type *type) {
    	allAvailableTypes.push_back(QString::fromStdString(type->getName()));
    });

	///--------------------------------------
	/// INIT WIDGETS
	///--------------------------------------
	ui->typesListView->setModel(new QStringListModel(allAvailableTypes, this));
	ui->typesListView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);

	ui->typePropertiesView->setModel(new models::TypePropertiesDataModel(this));
	//ui->typePropertiesView->setItemDelegate(new delegates::TypePropertyItemDelegate(this));
	ui->typePropertiesView->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
	ui->typePropertiesView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

	///--------------------------------------
	/// SIGNALS
	///--------------------------------------
    connect(ui->closeButton, &QPushButton::clicked, [=]() { close(); });
    connect(ui->typesListView, &QListView::activated, [=](const QModelIndex &newIndex) {
    	auto model = reinterpret_cast<models::TypePropertiesDataModel *>(ui->typePropertiesView->model());

		if (!newIndex.data().isValid()) {
			model->resetType();
			return;
		}

	    model->setType(newIndex.data().value<QString>());

		//TODO: Nice try but memory leaks :(
//		auto typeInfo = gamelib::TypeRegistry::getInstance().findTypeByName(newIndex.data().value<QString>().toStdString());
//		if (typeInfo && typeInfo->getKind() == gamelib::TypeKind::ENUM)
//		{
//			QStringList possibleEnumValues;
//
//			auto enumTypeInfo = reinterpret_cast<const gamelib::TypeEnum *>(typeInfo);
//			for (const auto &value: enumTypeInfo->getPossibleValues())
//			{
//				possibleEnumValues.push_back(QString::fromStdString(value.name));
//			}
//
//			auto enumValuesCombo = new QComboBox(ui->typePropertiesView);
//			enumValuesCombo->setModel(new QStringListModel(possibleEnumValues));
//			ui->typePropertiesView->setIndexWidget(model->index(0, 1), enumValuesCombo);
//		}
	});
}

TypeViewerWindow::~TypeViewerWindow()
{
    delete ui;
}
