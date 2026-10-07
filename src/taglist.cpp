#include "taglist.h"
#include <QMenu>


TagList::TagList(QWidget* parent) : QListWidget(parent) {
    setFixedHeight(95);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
}

void TagList::contextMenuEvent(QContextMenuEvent *event) {
    if (!editModeEnabled) return;
    QListWidgetItem *item = this->itemAt(event->pos());
    if (item == nullptr) return;

    menuItem = item;

    QMenu *menu = new QMenu(this);
    QAction *removeTag = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove), "Remove \""+item->text()+"\"");
    menu->addAction(removeTag);
    connect(removeTag, &QAction::triggered, this, &TagList::onRemoveTag);
    menu->exec(QCursor::pos());
}

void TagList::onRemoveTag() {
    delete menuItem;
    menuItem = nullptr;
}

void TagList::setEditMode(bool mode) {
    this->editModeEnabled = mode;
    this->setDragDropMode(this->editModeEnabled ? DragDropMode::InternalMove : DragDropMode::NoDragDrop);
}
