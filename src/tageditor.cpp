#include "tageditor.h"
#include "ui_tageditor.h"
#include <QFileInfo>

TagEditor::TagEditor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TagEditor)
{
    ui->setupUi(this);
    ui->iconButton->hide();
    ui->descriptionEditor->hide();
    iconDialog = new IconDialog(this);

    this->relatedList = new TagListWidget("Related Tags");
    this->requiredList = new TagListWidget("Required Tags");
    ui->listLayout->addWidget(this->requiredList);
    ui->listLayout->addWidget(this->relatedList);
    connect(iconDialog, &IconDialog::iconSelected, this, &TagEditor::iconSelected);

    connect(ui->editButton, &QPushButton::clicked, this, &TagEditor::toggleEditMode);
    connect(ui->saveButton, &QPushButton::clicked, this, &TagEditor::save);
    connect(ui->iconButton, &QToolButton::clicked, this, &TagEditor::selectIcon);
    connect(this->requiredList, &TagListWidget::tagSelected, this, &TagEditor::onListItemSelect);
    connect(this->relatedList, &TagListWidget::tagSelected, this, &TagEditor::onListItemSelect);

    connect(this, &TagEditor::editModeChanged, this->relatedList, &TagListWidget::setEditMode);
    connect(this, &TagEditor::editModeChanged, this->requiredList, &TagListWidget::setEditMode);
}

TagEditor::~TagEditor()
{
    delete ui;
}

void TagEditor::linkTagTreeToLists(const TagTree* ptr) {
    this->requiredList->linkTagTree(ptr);
    this->relatedList->linkTagTree(ptr);
}

void TagEditor::setTag(TagNode *node) {
    currentTag = node;
    if (node == nullptr) {
        this->requiredList->clear();
        this->relatedList->clear();
        ui->editButton->setEnabled(false);
        ui->tagLabelEdit->setText("");
        ui->iconButton->setIcon(QIcon());
        ui->iconLabel->setPixmap(QPixmap());
        ui->description->setMarkdown("");
        return;
    }

    this->relatedList->setTag(node);
    this->requiredList->setTag(node);
    this->relatedList->setEnabled(node != nullptr);
    this->requiredList->setEnabled(node != nullptr);
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
        ui->descriptionEditor->hide();
        ui->description->show();
        ui->iconButton->hide();
        ui->iconLabel->show();
        this->relatedList->clear();
        this->requiredList->clear();

        refreshLists();
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
    this->relatedList->clear();
    this->requiredList->clear();

    if (currentTag == nullptr) return;

    for (const std::string& t : currentTag->getRequired())
        this->requiredList->insertTag(QString::fromStdString(t));
    for (const std::string& t : currentTag->getRelated())
        this->relatedList->insertTag(QString::fromStdString(t));
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
    currentTag->setRelated(this->relatedList->values());
    currentTag->setRequired(this->requiredList->values());

    emit tagSaved(currentTag, oldPath);
    toggleEditMode();
}

void TagEditor::onListItemSelect(QString tag) {
    emit listItemSelected(tag);
}

void TagEditor::addToRelated(TagNode *node) {
    this->relatedList->insertTag(node);
}

void TagEditor::addToRequired(TagNode *node) {
    this->requiredList->insertTag(node);
}



