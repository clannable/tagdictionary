#include "taglist.h"
#include <QMenu>

TagList::TagList(QWidget* parent) : QListWidget(parent) {}

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
    emit tagRemoved();
}

void TagList::setEditMode(bool mode) {
    this->editModeEnabled = mode;
}

