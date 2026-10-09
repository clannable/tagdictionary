#include "taglist.h"
#include <QMenu>
#include "globals.h"
#include <QFileInfo>
#include <qmimedata.h>
#include "tagtree.h"

TagList::TagList(QWidget* parent) : QListWidget(parent) {
    setFixedHeight(105);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setStyleSheet("TagList { background: transparent; border: none; font: 350 10pt 'Segoe UI'}");
    connect(APP_STATE, &AppState::editModeChanged, this, &TagList::onEditModeChanged);
    connect(this, &QListWidget::itemDoubleClicked, this, &TagList::onItemDoubleClicked);
}

void TagList::createListItem(int id, int index) {
    TagNode* tag = TAGS[id];
    QListWidgetItem* item = new QListWidgetItem();
    item->setData(Qt::UserRole, id);
    item->setText(QString::fromStdString(tag->key()));

    QString icon = QString::fromStdString(tag->icon());
    if (QFileInfo::exists(icon) || icon.startsWith(":/icons/"))
        item->setIcon(QIcon(icon));
    else
        item->setIcon(QIcon(":/icons/" + icon));

    if (index == -1)
        addItem(item);
    else
        insertItem(index, item);
}

void TagList::setValues(nlohmann::json::array_t ids) {
    clear();
    for (const auto& i : ids) createListItem(i.get<int>());
    scrollToTop();
}

void TagList::contextMenuEvent(QContextMenuEvent *event) {
    if (!APP_STATE->editModeEnabled()) return;
    QListWidgetItem *item = this->itemAt(event->pos());
    if (item == nullptr) return;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *removeTag = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove), "Remove \""+item->text()+"\"");
    menu->addAction(removeTag);
    connect(removeTag, &QAction::triggered, this, [item] { delete item; });
    menu->exec(QCursor::pos());
}

void TagList::onEditModeChanged(bool editModeEnabled) {
    this->setStyleSheet(editModeEnabled
        ? "TagList { background-color: #eeeeee; border: 1px solid #777777; border-radius: 4px; }"
        : "TagList { background: transparent; border: none; font: 350 10pt 'Segoe UI'}");
    this->setAcceptDrops(editModeEnabled);
    this->setDragEnabled(editModeEnabled);
    this->setDragDropMode(editModeEnabled ? DragDropMode::DragDrop : DragDropMode::NoDragDrop);
}

void TagList::removeTag(int id) {
    for (int i = 0; i < count(); i++) {
        if (item(i)->data(Qt::UserRole) == id) {
            delete item(i);
            return;
        }
    }
}

void TagList::onItemDoubleClicked(QListWidgetItem* item) {
    APP_STATE->setSelectedTag(TAGS[item->data(Qt::UserRole).toInt()]);
}
void TagList::dropEvent(QDropEvent *event) {
    if (!APP_STATE->editModeEnabled()) return;
    if (event->source() == this) {
        QListWidget::dropEvent(event);
        return;
    }
    int id = QVariant(event->mimeData()->data("application/x-tag-id")).toInt();
    QListWidgetItem* dest = itemAt(event->position().toPoint());
    int index = -1;
    if (dest != nullptr)
        index = row(dest);
    if (dropIndicatorPosition() == DropIndicatorPosition::BelowItem)
        index += 1;

    createListItem(id, index);
    if (dynamic_cast<TagList*>(event->source()) != nullptr)
        dynamic_cast<TagList*>(event->source())->removeTag(id);
}

void TagList::dragEnterEvent(QDragEnterEvent *event) {
    if (event->source() == this) {
        QListWidget::dragEnterEvent(event);
        return;
    }
    if (APP_STATE->editModeEnabled() && event->mimeData()->hasFormat("application/x-tag-id")) {
        int id = QVariant(event->mimeData()->data("application/x-tag-id")).toInt();

        if (id == APP_STATE->selectedTag()->id()) {
            event->setDropAction(Qt::IgnoreAction);
            event->ignore();
        } else {
            if (dynamic_cast<TagTree*>(event->source()) != nullptr) {
                event->setDropAction(Qt::CopyAction);
                event->accept();
            } else if (dynamic_cast<TagList*>(event->source()) != nullptr) {
                event->accept();
            } else return;
        }
    } else {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
    }
}

std::list<int> TagList::values() {
    std::list<int> ids = {};
    for (int i = 0; i < count(); i++)
        ids.push_back(item(i)->data(Qt::UserRole).toInt());
    return ids;
}

QStringList TagList::mimeTypes() const {
    QStringList types = QListWidget::mimeTypes();
    types.push_back("application/x-tag-id");
    return types;
}

QMimeData* TagList::mimeData(const QList<QListWidgetItem*> &items) const {
    QMimeData* data = QListWidget::mimeData(items);
    if (data != nullptr)
        data->setData("application/x-tag-id", QVariant(items.first()->data(Qt::UserRole)).toByteArray());
    return data;
}
