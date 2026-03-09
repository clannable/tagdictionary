#include "taglistwidget.h"
#include <QMenu>
#include <QMimeData>
#include <QCursor>
#include "./taglistwidgetitem.h"

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

// void TagListWidget::dropEvent(QDropEvent *event) {

//     QString tagPath = event->mimeData()->text();
//     addItem(new TagListWidgetItem(tagPath));
//     event->acceptProposedAction();
// }

// void TagListWidget::dragEnterEvent(QDragEnterEvent *event) {
//     std::cout << "Drag enter" << this->acceptDrops() << std::flush;
//     if (event->mimeData()->hasFormat("text/plain")) {

//         /*std::cout << event->mimeData()->text().toStdString() << std::flush;
//         QString tagPath = event->mimeData()->text();
//         for (int i = 0; i < count(); i++) {
//             if ((static_cast<TagListWidgetItem*>(item(i)))->getValue() == tagPath) {
//                 std::cout << "Tag exists in list" << std::flush;
//                 event->ignore();
//                 return;
//             }
//         }*/
//         event->acceptProposedAction();
//     }
// }

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
