#include "taglistwidget.h"
#include <QMenu>
#include <QMimeData>
#include <QCursor>
#include "taglistwidgetitem.h"
#include <iostream>

TagListWidget::TagListWidget(QWidget *parent) :
    QListWidget(parent) {
}

void TagListWidget::insertTag(JsonNode *node) {
    QString tagPath = QString::fromStdString(node->getFullPath());
    for (int i = 0; i < count(); i++) {
        if (static_cast<TagListWidgetItem*>(item(i))->getValue() == tagPath)
            return;

    }
    addItem(new TagListWidgetItem(tagPath));
    sortItems();
}

void TagListWidget::insertTag(QString tagPath) {
    addItem(new TagListWidgetItem(tagPath));
    sortItems();
}

void TagListWidget::linkTagTree(const TagTree* ptr) {
    this->tagTree = ptr;
}

void TagListWidget::setTag(JsonNode* node) {
    this->currentTag = node;
}

void TagListWidget::dropEvent(QDropEvent *event) {
    if (!editModeEnabled) return;
    JsonNode* node = static_cast<TagTreeItem*>(tagTree->currentItem())->getNode();
    if (node == this->currentTag) return;
    QString tagPath = QString::fromStdString(static_cast<TagTreeItem*>(tagTree->currentItem())->getNode()->getFullPath());
    for (int i = 0; i < count(); i++)
        if ((static_cast<TagListWidgetItem*>(item(i)))->getValue() == tagPath) return;

    insertTag(tagPath);
}

void TagListWidget::dragEnterEvent(QDragEnterEvent *event) {
    // std::cout << event->source()->objectName().toStdString() << "\n" <<std::flush;
    if (editModeEnabled && event->source() == tagTree) {
        JsonNode* node = static_cast<TagTreeItem*>(tagTree->currentItem())->getNode();
        if (node != this->currentTag) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        } else {
            event->setDropAction(Qt::IgnoreAction);
            event->ignore();
        }
    } else {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
    }
}

void TagListWidget::contextMenuEvent(QContextMenuEvent *event) {
    if (!editModeEnabled) return;
    QListWidgetItem *item = itemAt(event->pos());
    if (item == nullptr) return;

    menuItem = item;

    QMenu *menu = new QMenu(this);
    QAction *removeTag = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove), "Remove \""+item->text()+"\"");
    menu->addAction(removeTag);
    connect(removeTag, &QAction::triggered, this, &TagListWidget::onRemoveTag);
    menu->exec(QCursor::pos());
}

void TagListWidget::onRemoveTag() {
    if (!editModeEnabled || menuItem == nullptr) return;

    delete menuItem;
}

void TagListWidget::setEditMode(bool mode) {
    editModeEnabled = mode;

    // setAcceptDrops(mode);
}

QList<std::string> TagListWidget::values() {
    QList<std::string> *list = new QList<std::string>();
    for (int i = 0; i < count(); i++) {
        TagListWidgetItem *item = static_cast<TagListWidgetItem*>(this->item(i));
        list->append(item->getValue().toStdString());
    }
    return *list;
}
