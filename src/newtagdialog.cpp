#include "newtagdialog.h"
#include "ui_newtagdialog.h"
#include <QPushButton>
#include <nlohmann/json.hpp>
#include <QFileInfo>
#include "globals.h"

NewTagDialog::NewTagDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewTagDialog)
{
    ui->setupUi(this);
    ui->nameEdit->setFocus();
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

    setTabOrder(ui->nameEdit, ui->descriptionEdit);
    setTabOrder(ui->descriptionEdit, ui->buttonBox);
    connect(ui->resetIconButton, &QPushButton::clicked, this, &NewTagDialog::resetIcon);
    connect(ui->iconButton, &QToolButton::clicked, this, &NewTagDialog::selectIcon);
    connect(ui->nameEdit, &QLineEdit::textEdited, this, &NewTagDialog::onNameChange);
}

NewTagDialog::~NewTagDialog()
{
    delete ui;
}

void NewTagDialog::onNameChange(QString text) {
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(text.trimmed().isEmpty() || !m_invalidNames.contains(text.trimmed()));
}

void NewTagDialog::selectIcon() {
    IconDialog* iconDialog = new IconDialog(this);
    connect(iconDialog, &IconDialog::iconSelected, this, &NewTagDialog::updateIcon);
    iconDialog->setSelected(m_icon);
    iconDialog->exec();
}

void NewTagDialog::setTag(int id) {
    TagNode* tag = TAGS[id];
    for (int c : tag->children())
        m_invalidNames.push_back(QString::fromStdString(TAGS[c]->key()));

    if (tag != nullptr) {
        std::string parentIcon = tag->icon();
        QString icon = QString::fromStdString(parentIcon.empty() ? ":/icons/flat/tag" : parentIcon);
        m_icon = icon;
        ui->iconButton->setIcon(QIcon(m_icon));
        QString path = QString::fromStdString(tag->getFullPath());
        if (path.isEmpty()) path = "/";
        ui->titleLabel->setText("Creating new tag in \"" + path + "\"");
    } else {
        ui->titleLabel->setText("Creating new tag in \"/\"");
    }
}

void NewTagDialog::resetIcon() {
    updateIcon(":/icons/flat/tag");
}

void NewTagDialog::updateIcon(QString icon) {
    m_icon = icon;
    ui->iconButton->setIcon(QIcon(m_icon));
}

void NewTagDialog::accept() {
    TagNode *ret = new TagNode(json({
        {"description", ui->descriptionEdit->toPlainText().toStdString()},
        {"icon", m_icon.toStdString() }
    }), ui->nameEdit->text().toStdString());
    emit submit(ret);
    QDialog::accept();
}
