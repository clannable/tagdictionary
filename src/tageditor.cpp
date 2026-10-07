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
    this->frequentList = new TagListWidget("Frequently Tagged With");

    ui->listContainer->layout()->addWidget(this->requiredList);
    ui->listContainer->layout()->addWidget(this->frequentList);
    ui->listContainer->layout()->addWidget(this->relatedList);

    connect(iconDialog, &IconDialog::iconSelected, this, &TagEditor::iconSelected);

    connect(ui->editButton, &QPushButton::clicked, this, &TagEditor::toggleEditMode);
    connect(ui->saveButton, &QPushButton::clicked, this, &TagEditor::save);
    connect(ui->iconButton, &QToolButton::clicked, this, &TagEditor::selectIcon);
    connect(this->requiredList, &TagListWidget::tagSelected, this, &TagEditor::onListItemSelect);
    connect(this->relatedList, &TagListWidget::tagSelected, this, &TagEditor::onListItemSelect);
    connect(this->frequentList, &TagListWidget::tagSelected, this, &TagEditor::onListItemSelect);
    connect(ui->description, &QTextBrowser::anchorClicked, this, &TagEditor::onAnchorClick);
    connect(this, &TagEditor::editModeChanged, this->relatedList, &TagListWidget::setEditMode);
    connect(this, &TagEditor::editModeChanged, this->requiredList, &TagListWidget::setEditMode);
    connect(this, &TagEditor::editModeChanged, this->frequentList, &TagListWidget::setEditMode);
}

TagEditor::~TagEditor()
{
    delete ui;
}

void TagEditor::linkTagTreeToLists(const TagTree* ptr) {
    this->requiredList->linkTagTree(ptr);
    this->relatedList->linkTagTree(ptr);
    this->frequentList->linkTagTree(ptr);
}

void TagEditor::setTag(TagNode *node) {
    currentTag = node;
    if (node == nullptr) {
        for (QWidget* child : ui->listContainer->findChildren<QWidget*>())
            child->hide();
        ui->editButton->setEnabled(false);
        ui->tagLabelEdit->setText("");
        ui->iconButton->setIcon(QIcon());
        ui->iconLabel->setPixmap(QPixmap());
        ui->description->setMarkdown("");
        ui->description->show();
        return;
    }
    if (node->isRoot()) {
        for (QWidget* child : ui->listContainer->findChildren<QWidget*>())
            child->hide();
        ui->editButton->setEnabled(false);
        ui->description->setMarkdown("");
        ui->description->show();
        ui->iconLabel->hide();
        ui->tagLabelEdit->setText("");
        ui->saveButton->setEnabled(false);
        return;
    }
    for (QWidget* child : ui->listContainer->findChildren<QWidget*>())
        child->show();
    ui->iconLabel->show();
    ui->editButton->show();
    ui->saveButton->show();
    this->relatedList->setTag(node);
    this->requiredList->setTag(node);
    this->frequentList->setTag(node);

    ui->editButton->setEnabled(true);
    ui->description->show();
    ui->descriptionEditor->hide();

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

        refreshLists();
    }

    emit editModeChanged(editModeEnabled);
}

void TagEditor::selectIcon() {
    iconDialog->setSelected(iconPath);
    iconDialog->exec();
}

void TagEditor::iconSelected(QString icon) {
    if (!icon.isEmpty()) {
        iconPath = icon;
        QIcon ic = QIcon(iconPath);
        ui->iconButton->setIcon(ic);
        if (this->editModeEnabled == false) {
            currentTag->setIcon(iconPath.toStdString());
            ui->iconLabel->setPixmap(ic.pixmap(QSize(20,20)));
            emit partialSave(currentTag);
        }
    }
}

void TagEditor::refreshLists() {
    this->relatedList->clear();
    this->requiredList->clear();
    this->frequentList->clear();

    if (currentTag == nullptr) return;

    for (const int& t : currentTag->getRequired())
        this->requiredList->insertTag(t);
    for (const int& t : currentTag->getRelated())
        this->relatedList->insertTag(t);
    for (const int& t : currentTag->getFrequent())
        this->frequentList->insertTag(t);
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
    currentTag->setFrequent(this->frequentList->values());

    emit tagSaved(currentTag, oldPath);
    toggleEditMode();
}

void TagEditor::onAnchorClick(const QUrl &link) {
    QString path = link.toString();
    if (path.startsWith("#$")) {
        emit displayLinkClicked(path.slice(2).toInt()-1);
    } else {
        ui->description->scrollToAnchor(path.slice(1));
    }
}

void TagEditor::onListItemSelect(int tagId) {
    emit listItemSelected(tagId);
}

void TagEditor::addToRelated(TagNode *node) {
    this->relatedList->insertTag(node);
}

void TagEditor::addToRequired(TagNode *node) {
    this->requiredList->insertTag(node);
}



