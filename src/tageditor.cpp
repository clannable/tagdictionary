#include "tageditor.h"
#include "ui_tageditor.h"
#include "taglistwidgetitem.h"
#include <QFileInfo>

TagEditor::TagEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TagEditor)
{
    ui->setupUi(this);
    ui->iconButton->hide();
    ui->descriptionEditor->hide();
    ui->description->document()->setDefaultStyleSheet("ul li { list-style-type: \"-\"; margin-left: 5px;}");
    iconDialog = new IconDialog(this);
    connect(iconDialog, &IconDialog::iconSelected, this, &TagEditor::iconSelected);

    connect(ui->editButton, &QPushButton::clicked, this, &TagEditor::toggleEditMode);
    connect(ui->saveButton, &QPushButton::clicked, this, &TagEditor::save);
    connect(ui->iconButton, &QToolButton::clicked, this, &TagEditor::selectIcon);
    connect(ui->requiredList, &QListWidget::itemDoubleClicked, this, &TagEditor::onListItemSelect);
    connect(ui->relatedList, &QListWidget::itemDoubleClicked, this, &TagEditor::onListItemSelect);

    connect(this, &TagEditor::editModeChanged, ui->relatedList, &TagListWidget::setEditMode);
    connect(this, &TagEditor::editModeChanged, ui->requiredList, &TagListWidget::setEditMode);
}

TagEditor::~TagEditor()
{
    delete ui;
}

void TagEditor::linkTagTreeToLists(const TagTree* ptr) {
    ui->requiredList->linkTagTree(ptr);
    ui->relatedList->linkTagTree(ptr);
}

void TagEditor::setTag(TagNode *node) {
    currentTag = node;
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
    ui->relatedList->setTag(node);
    ui->requiredList->setTag(node);
    ui->relatedList->setEnabled(node != nullptr);
    ui->requiredList->setEnabled(node != nullptr);
    ui->editButton->setEnabled(true);

    QString ic = QString::fromStdString(node->getIcon());
    if (QFileInfo::exists(ic) || ic.startsWith(":/icons/")) {
        iconPath = ic;
    } else if (!ic.startsWith(":/icons/")) {
        iconPath = ":/icons/" + ic;
    }
    QIcon icon = QIcon(iconPath);
    description = QString::fromStdString(node->getDescription());

    ui->iconLabel->setPixmap(icon.pixmap(QSize(20, 20)));
    ui->description->setMarkdown(description);
    ui->descriptionEditor->setText(description);
    ui->tagLabelEdit->setText(QString::fromStdString(node->getKey()));
    ui->iconButton->setIcon(QIcon(iconPath));

    refreshLists();
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
        ui->descriptionEditor->setText(description);
        ui->descriptionEditor->show();
        ui->iconButton->show();
        ui->iconLabel->hide();
    }
    else {
        this->setTag(this->currentTag);
    }

    emit editModeChanged(editModeEnabled);
}

void TagEditor::selectIcon() {
    iconDialog->setSelected(iconPath);
    iconDialog->exec();
}

void TagEditor::iconSelected(QString icon) {
    iconPath = icon;
    ui->iconButton->setIcon(QIcon(iconPath));
}

void TagEditor::refreshLists() {
    ui->relatedList->clear();
    ui->requiredList->clear();

    if (currentTag == nullptr) return;

    for (const std::string& t : currentTag->getRequired())
        ui->requiredList->insertTag(QString::fromStdString(t));
    for (const std::string& t : currentTag->getRelated())
        ui->relatedList->insertTag(QString::fromStdString(t));
}

void TagEditor::save() {
    QIcon icon = QIcon(iconPath);
    ui->iconLabel->setPixmap(icon.pixmap(QSize(20,20)));
    QString updatedDescription = ui->descriptionEditor->toPlainText();
    description = updatedDescription;
    ui->description->setMarkdown(updatedDescription);

    std::string oldPath = currentTag->getFullPath();

    currentTag->setKey(ui->tagLabelEdit->text());
    currentTag->setDescription(updatedDescription);
    currentTag->setIcon(iconPath);
    currentTag->setRelated(ui->relatedList->values());
    currentTag->setRequired(ui->requiredList->values());

    emit tagSaved(currentTag, oldPath);
    toggleEditMode();
}

void TagEditor::onListItemSelect(QListWidgetItem *item) {
    emit listItemSelected(static_cast<TagListWidgetItem*>(item)->getValue());
}

void TagEditor::addToRelated(TagNode *node) {
    ui->relatedList->insertTag(node);
}

void TagEditor::addToRequired(TagNode *node) {
    ui->requiredList->insertTag(node);
}



