#include "tagtree.h"
#include "newtagdialog.h"
#include "globals.h"
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QMenu>
#include <QCursor>
#include <QMessageBox>
#include <QHeaderView>

TagTree::TagTree(QWidget* parent) : QTreeWidget(parent) {
    setExpandsOnDoubleClick(true);
    header()->setSectionResizeMode(0, QHeaderView::Stretch);
    header()->setSectionResizeMode(1, QHeaderView::Fixed);
    header()->setSectionResizeMode(2, QHeaderView::Fixed);
    header()->resizeSection(1, 20);
    header()->resizeSection(2, 20);

    connect(addAction("Expand All Tags"), &QAction::triggered, this, &QTreeView::expandAll);
    connect(addAction("Collapse All Tags"), &QAction::triggered, this, &QTreeView::collapseAll);

    connect(this, &QTreeWidget::currentItemChanged, this, &TagTree::onItemSelected);
    connect(APP_STATE, &AppState::editModeChanged, this, &TagTree::onEditModeChanged);
}

QTreeWidgetItem* TagTree::findTag(int tagId) {
    QTreeWidgetItemIterator it(this);

    while (*it) {
        if ((*it)->data(0, Qt::UserRole) == tagId)
            return *it;
        ++it;
    }
    return m_rootItem;
}

void TagTree::loadTags() {
    clear();
    m_rootItem = nullptr;
    m_items.clear();
    if (m_rootNode != nullptr)
        delete m_rootNode;

    m_rootNode = new TagNode();
    m_rootItem = new TagTreeItem(m_rootNode);
    addTopLevelItem(m_rootItem);

    for (const auto [id, tag] : TAGS)
        m_items[id] = new TagTreeItem(tag);

    for (const auto [id, item] : m_items.asKeyValueRange()) {
        if (item->tag()->parent() == nullptr)
            m_rootItem->addChild(item);
        else
            m_items[item->tag()->parent()->id()]->addChild(item);
    }

    m_items[0] = m_rootItem;
    TAGS[0] = m_rootNode;
    ICON_LIST->sort();
    ICON_LIST->unique();

    this->header()->resizeSection(1, 24);
    this->header()->resizeSection(2, 24);

    m_rootItem->setExpanded(true);
}

json TagTree::toJson() {
    return dynamic_cast<TagTreeItem*>(topLevelItem(0))->tag()->toJson();
}

void TagTree::newTag(int id) {
    NewTagDialog *dialog = new NewTagDialog(this);
    dialog->setTag(id);
    connect(dialog, &NewTagDialog::submit, this, [id, this] (TagNode* tag) { addTag(id, tag); });
    dialog->exec();
}

void TagTree::refreshTag(TagNode* tag) {
    m_items[tag->id()]->refresh();
}

void TagTree::addTag(int parentId, TagNode *tag) {
    if (!tag->icon().empty()) {
        ICON_LIST->push_back(tag->icon());
        ICON_LIST->sort();
        ICON_LIST->unique();
    }

    m_items[tag->id()] = new TagTreeItem(tag);
    if (parentId > 0)
        m_items[parentId]->addChild(m_items[tag->id()]);
    else
        m_rootItem->addChild(m_items[tag->id()]);

    sortItems(0, Qt::AscendingOrder);
    emit tagsChanged();
    APP_STATE->setSelectedTag(tag);
    expandItem(m_items[tag->id()]->parent());
    setCurrentItem(m_items[tag->id()]);
    scrollToItem(m_items[tag->id()]);
}

void TagTree::onRemoveTag(int id) {

    if (QMessageBox::question(this, "Remove Tag",
        "Are you sure you want to remove the tag \"" +
        QString::fromStdString(TAGS[id]->key()) +
        "\"? This will also remove all sub-tags inside this tag.") == QMessageBox::Yes)
    {
        if (APP_STATE->selectedTag()->id() == id) {
            setCurrentItem(nullptr);
            APP_STATE->setSelectedTag(nullptr);
        }
        TagNode* tag = TAGS[id];
        tag->parent()->removeChild(id);
        delete tag;
        delete m_items[id];
        m_items.remove(id);

        emit tagsChanged();
    }
}

void TagTree::expandTreeTo(QTreeWidgetItem* item) {
    QTreeWidgetItem* ptr = item;
    while ((ptr = ptr->parent()) != nullptr)
        expandItem(ptr);
    scrollToItem(item);
}

void TagTree::filterTree(QString search) {
    if (search.trimmed().isEmpty()) {
        resetTagVisibility();
    } else {
        for (int i = 0; i < m_rootItem->childCount(); i++)
            filterTags(search, m_rootItem->child(i));
    }
}

bool TagTree::filterTags(QString search, QTreeWidgetItem* node) {
    bool match = node->text(0).contains(search, Qt::CaseInsensitive);

    if (node->childCount() != 0) {
        for (int i = 0; i < node->childCount(); i++) {
            match = filterTags(search, node->child(i)) || match;
        }
    }

    node->setHidden(!match);
    if (match)
        node->setExpanded(true);
    return match;
}

void TagTree::resetTagVisibility() {
    QTreeWidgetItemIterator it(this);
    while (*it) {
        (*it)->setHidden(false);
        ++it;
    }
}

void TagTree::collapseRecursive(QTreeWidgetItem* item) {
    QTreeWidgetItemIterator it(item);
    while(*it) {
        (*it)->setExpanded(false);
        ++it;
    }
    if (item == m_rootItem) item->setExpanded(true);
}

void TagTree::onItemDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    TagNode* tag = static_cast<TagTreeItem*>(item)->tag();
    if (!tag->isRoot()) {
        APP_STATE->setSelectedTag(tag);
        APP_STATE->setEditModeEnabled(true);
    }
}

void TagTree::onItemSelected(QTreeWidgetItem* current, QTreeWidgetItem* previous) { Q_UNUSED(previous);
    if (APP_STATE->editModeEnabled() || current == nullptr) return;
    APP_STATE->setSelectedTag(static_cast<TagTreeItem*>(current)->tag());
}

void TagTree::onEditModeChanged(bool editModeEnabled) {
    QTreeWidgetItem* item = m_items[APP_STATE->selectedTag()->id()];
    QFont font = item->font(0);
    font.setBold(editModeEnabled);
    item->setFont(0, font);
}

void TagTree::dropEvent(QDropEvent *event) {
    if (event->source() != this) {
        event->ignore();
        return;
    }
    QTreeWidgetItem* dest = itemAt(event->position().toPoint());
    int id = QVariant(event->mimeData()->data("application/x-tag-id")).toInt();
    if (dest->data(0, Qt::UserRole).toInt() == id) {
        event->ignore();
        return;
    }

    m_items[id]->parent()->removeChild(m_items[id]);
    if (dest == nullptr || dest == m_rootItem)
        m_rootItem->addChild(m_items[id]);
    else
        dynamic_cast<TagTreeItem*>(dest)->addChild(m_items[id]);

    if (dest != nullptr)
        dest->sortChildren(0, Qt::AscendingOrder);
    emit tagsChanged();
}

QMimeData* TagTree::mimeData(const QList<QTreeWidgetItem*> &items) const {
    QMimeData* ret = QTreeWidget::mimeData(items);
    if (ret == nullptr) return ret;
    ret->setData("application/x-tag-id", QVariant(items.first()->data(0, Qt::UserRole)).toByteArray());
    return ret;
}

QStringList TagTree::mimeTypes() const {
    QStringList ret = QTreeWidget::mimeTypes();
    ret.push_back("application/x-tag-id");
    return ret;
}

void TagTree::dragMoveEvent(QDragMoveEvent *event) {
    QTreeWidget::dragMoveEvent(event);
    if (event->source() != this || !event->isAccepted()) {
        event->ignore();
        return;
    }
    int id = QVariant(event->mimeData()->data("application/x-tag-id")).toInt();

    QTreeWidgetItem *dest = itemAt(event->position().toPoint());
    int dId = dest == nullptr ? 0 : dest->data(0, Qt::UserRole).toInt();
    if (dId == id || dId == m_items[id]->parent()->data(0, Qt::UserRole).toInt())
        event->ignore();
    else
        event->accept();
}

void TagTree::setCurrentTag(TagNode* tag) {
    if (this->currentItem()->data(0, Qt::UserRole).toInt() != tag->id()) {
        expandTreeTo(m_items[tag->id()]);
        setCurrentItem(m_items[tag->id()]);
    }
}

void TagTree::contextMenuEvent(QContextMenuEvent *event) {
    QTreeWidgetItem* item = itemAt(event->pos());
    int id = item == nullptr ? 0 : item->data(0, Qt::UserRole).toInt();
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    connect(menu->addAction("New Tag..."), &QAction::triggered,
            this, [id, this] { newTag(id); });

    const bool validTagSelected = item != nullptr && item != m_rootItem;
    if (validTagSelected && !APP_STATE->editModeEnabled())
        connect(menu->addAction("Remove \"" + item->text(0) + "\""), &QAction::triggered,
                this, [id, this] { onRemoveTag(id); });

    menu->addSeparator();

    if (validTagSelected && m_items[id]->childCount()) {
        connect(menu->addAction("Expand Selected Tag"), &QAction::triggered,
                this, [item, this] { expandRecursively(indexFromItem(item)); });
        connect(menu->addAction("Collapse Selected Tag Recursively"), &QAction::triggered,
                this, [item, this] { collapseRecursive(item); });
    }
    menu->addActions(actions());
    menu->exec(QCursor::pos());
}
