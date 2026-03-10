#include "newtagdialog.h"
#include "ui_newtagdialog.h"
#include <QPushButton>
#include <nlohmann/json.hpp>
#include <QFileInfo>

NewTagDialog::NewTagDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewTagDialog)
{
    ui->setupUi(this);

    iconDialog = new IconDialog(this);
    connect(iconDialog, &IconDialog::iconSelected, this, &NewTagDialog::onIconUpdate);
    ui->nameEdit->setFocus();
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

    setTabOrder(ui->nameEdit, ui->descriptionEdit);
    setTabOrder(ui->descriptionEdit, ui->buttonBox);
    connect(ui->resetIconButton, &QPushButton::clicked, this, &NewTagDialog::resetIcon);
    connect(ui->iconButton, &QToolButton::clicked, this, &NewTagDialog::onIconSelect);
    connect(ui->nameEdit, &QLineEdit::textEdited, this, &NewTagDialog::onNameChange);
}

NewTagDialog::~NewTagDialog()
{
    delete ui;
}

void NewTagDialog::onNameChange(QString text) {
    if (parentNode != nullptr && parentNode->hasChild(text.toStdString()))
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
    else
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!ui->nameEdit->text().trimmed().isEmpty());
}

void NewTagDialog::onIconSelect() {
    iconDialog->setSelected(iconPath);
    iconDialog->exec();
}

void NewTagDialog::setParentNode(JsonNode* node) {
    this->parentNode = node;
    if (node != nullptr) {
        QString icon = QString::fromStdString(node->getData().value("icon", ":/icons/flat/tag"));
        if (!QFileInfo::exists(icon) && !icon.startsWith(":/icons/"))
            icon = ":/icons/" + icon;
        iconPath = icon;
        ui->iconButton->setIcon(QIcon(iconPath));
        QString path = QString::fromStdString(node->getFullPath());
        if (path.isEmpty()) path = "/";
        ui->titleLabel->setText("Creating new tag in \"" + path + "\"");
    } else {
        ui->titleLabel->setText("Creating new tag in \"/\"");
    }
}

void NewTagDialog::resetIcon() {
    onIconUpdate(":/icons/flat/tag");
}

void NewTagDialog::onIconUpdate(QString iconPath) {
    this->iconPath = iconPath;
    ui->iconButton->setIcon(QIcon(iconPath));
}

void NewTagDialog::accept() {
    nlohmann::json data = nlohmann::json();
    data["description"] = ui->descriptionEdit->toPlainText().toStdString();
    data["icon"] = iconPath.toStdString();
    JsonNode *ret = new JsonNode(data, ui->nameEdit->text().toStdString(), parentNode);
    emit submit(ret);
    QDialog::accept();
}
