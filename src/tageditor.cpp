#include "tageditor.h"
#include "ui_tageditor.h"
#include "./taglistwidgetitem.h"
#include <iostream>

TagEditor::TagEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TagEditor)
{
    ui->setupUi(this);
    ui->iconButton->hide();
    ui->descriptionEditor->hide();
    iconDialog = new IconDialog(this);
    connect(iconDialog, &IconDialog::iconSelected, this, &TagEditor::iconSelected);

    connect(ui->editButton, &QPushButton::clicked, this, &TagEditor::toggleEditMode);
    connect(ui->saveButton, &QPushButton::clicked, this, &TagEditor::save);
    connect(ui->iconButton, &QToolButton::clicked, this, &TagEditor::selectIcon);
    connect(ui->requiredList, &QListWidget::itemDoubleClicked, this, &TagEditor::onListItemSelect);
    connect(ui->relatedList, &QListWidget::itemDoubleClicked, this, &TagEditor::onListItemSelect);

    connect(this, &TagEditor::editModeChanged, ui->relatedList, &TagListWidget::setEditMode);
    connect(this, &TagEditor::editModeChanged, ui->requiredList, &TagListWidget::setEditMode);

    std::cout << "Initialized TagEditor\n" << std::flush;
}

TagEditor::~TagEditor()
{
    delete ui;
}

void TagEditor::setTag(JsonNode *node) {
    if (node == nullptr) {
        ui->requiredList->clear();
        ui->relatedList->clear();
        ui->editButton->setEnabled(false);
        ui->tagLabelEdit->setText("");
        ui->iconButton->setIcon(QIcon());
        ui->iconLabel->setPixmap(QPixmap());
        ui->description->setMarkdown("");
        return;
    }
    json tag = node->getData();
    ui->editButton->setEnabled(true);

    iconPath = ":/icons/" + QString::fromStdString(tag.value("icon", ""));
    QIcon icon = QIcon(iconPath);

    ui->iconLabel->setPixmap(icon.pixmap(QSize(20, 20)));
    ui->description->setMarkdown(QString::fromStdString(tag.value("description", "")));
    ui->tagLabelEdit->setText(QString::fromStdString(node->getKey()));
    ui->iconButton->setIcon(QIcon(iconPath));
    ui->relatedList->clear();
    ui->requiredList->clear();

    if (tag.contains("required") && !tag["required"].empty()) {
        for (const std::string&& t : tag["required"])
            ui->requiredList->insertTag(QString::fromStdString(t));
    }
    if (tag.contains("related") && !tag["related"].empty()) {
        for (const std::string&& t : tag["related"])
            ui->relatedList->insertTag(QString::fromStdString(t));
    }
}

void TagEditor::toggleEditMode() {
    editModeEnabled = !editModeEnabled;

    ui->tagLabelEdit->setEnabled(editModeEnabled);
    ui->saveButton->setEnabled(editModeEnabled);
    ui->editButton->setIcon(QIcon::fromTheme(editModeEnabled
        ? QIcon::ThemeIcon::EditClear
        : QIcon::ThemeIcon::MailMessageNew
    ));
    ui->editButton->setText(editModeEnabled ? "Cancel" : "Edit Tag");
    if (editModeEnabled) {
        ui->description->hide();
        ui->descriptionEditor->setText(ui->description->toPlainText());
        ui->descriptionEditor->show();
        ui->iconButton->show();
        ui->iconLabel->hide();
    }
    else {
        ui->descriptionEditor->hide();
        ui->description->show();
        ui->iconButton->hide();
        ui->iconLabel->show();
    }

    emit editModeChanged(editModeEnabled);
}

void TagEditor::selectIcon() {
    iconDialog->setSelected(iconPath);
    iconDialog->exec();
}

void TagEditor::iconSelected(QString icon) {
    iconPath = QString(!icon.startsWith(":/icons/") ? ":/icons/" : "") + icon;
    ui->iconButton->setIcon(QIcon((!iconPath.startsWith(":/icons/") ? ":/icons/" : "") + iconPath));
}

void TagEditor::save() {
    QIcon icon = QIcon(iconPath);
    ui->iconLabel->setPixmap(icon.pixmap(QSize(20,20)));
    QString updatedDescription = ui->descriptionEditor->toPlainText();
    ui->description->setMarkdown(updatedDescription);

    nlohmann::json nodeJson = nlohmann::json();
    nodeJson["key"] = ui->tagLabelEdit->text().toStdString();
    nodeJson["description"] = updatedDescription.toStdString();
    nodeJson["icon"] = (iconPath.startsWith(":/icons/") ? iconPath.slice(8) : iconPath).toStdString();

    nodeJson["related"] = json(ui->relatedList->values().toList());
    nodeJson["required"] = json(ui->requiredList->values().toList());

    emit tagSaved(nodeJson);
    toggleEditMode();
}

void TagEditor::onListItemSelect(QListWidgetItem *item) {
    emit listItemSelected(static_cast<TagListWidgetItem*>(item)->getValue());
}

void TagEditor::addToRelated(JsonNode *node) {
    ui->relatedList->insertTag(node);
}

void TagEditor::addToRequired(JsonNode *node) {
    ui->requiredList->insertTag(node);
}



