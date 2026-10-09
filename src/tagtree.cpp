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
    this->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    this->header()->setSectionResizeMode(1, QHeaderView::Fixed);
    this->header()->setSectionResizeMode(2, QHeaderView::Fixed);
    this->header()->resizeSection(1, 20);
    this->header()->resizeSection(2, 20);
    connect(this, &QTreeWidget::currentItemChanged, this, &TagTree::onItemSelected);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &TagTree::onItemDoubleClicked);
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

void TagTree::fromJson(nlohmann::json json) {
    if (m_rootNode != nullptr)
        delete m_rootNode;

    m_rootNode = TagNode::createRoot(json);

    if (topLevelItemCount() != 0) {
        removeItemWidget(m_rootItem, 0);
    }

    m_rootItem = new TagTreeItem(m_rootNode);
    addTopLevelItem(m_rootItem);

    this->createChildren(m_rootItem, m_rootNode);
    ICON_LIST->sort();
    ICON_LIST->unique();


    this->header()->resizeSection(1, 24);
    this->header()->resizeSection(2, 24);

    m_rootItem->setExpanded(true);
}

void TagTree::createChildren(TagTreeItem* branch, TagNode *tag) {
    for (TagNode* child : tag->children()) {
        TagTreeItem* leaf = new TagTreeItem(child);
        ICON_LIST->push_back(child->icon());
        branch->addChild(leaf);
        this->createChildren(leaf, child);
    }
}

json TagTree::toJson() {
    return static_cast<TagTreeItem*>(topLevelItem(0))->tag()->toJson();
}

void TagTree::onCreateTag(TagTreeItem* item = nullptr) {
    NewTagDialog *dialog = new NewTagDialog(this);
    dialog->setTag(item != nullptr ? item->tag() : m_rootNode);
    connect(dialog, &NewTagDialog::submit, this, [item, this] (TagNode* tag) { onNewTag(item, tag); });
    dialog->exec();
}

void TagTree::onNewTag(TagTreeItem* srcItem, TagNode *tag) {
    TagTreeItem* item = new TagTreeItem(tag);

    ICON_LIST->push_back(tag->icon());
    ICON_LIST->sort();
    ICON_LIST->unique();

    if (srcItem != nullptr) {
        tag->setParent(srcItem->tag());
        srcItem->addChild(item);
        expandItem(srcItem);
    } else {
        tag->setParent(m_rootNode);
        m_rootItem->addChild(item);
    }
    sortItems(0, Qt::AscendingOrder);
    emit tagsChanged();
    APP_STATE->setSelectedTag(tag);
}

void TagTree::onRemoveTag(TagTreeItem* item) {

    if (QMessageBox::question(this, "Remove Tag",
        "Are you sure you want to remove the tag \"" +
        QString::fromStdString(item->tag()->key()) +
        "\"? This will also remove all sub-tags inside this tag.") == QMessageBox::Yes)
    {
        delete item;
        for (const auto& [key, value] : TAGS) {
            value->cleanSublists();
        }
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
        for (int i = 0; i < m_rootItem->childCount(); i++) {
            filterTags(search, m_rootItem->child(i));
        }
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

void TagTree::setExpandedRecursive(bool expanded, QTreeWidgetItem* item) {
    QTreeWidgetItemIterator it(item);
    while(*it) {
        (*it)->setExpanded(expanded);
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
    if (APP_STATE->editModeEnabled()) return;
    APP_STATE->setSelectedTag(static_cast<TagTreeItem*>(current)->tag());
}

void TagTree::onEditModeChanged(bool editModeEnabled) {
    QTreeWidgetItem* item = APP_STATE->selectedTag()->leaf();
    QFont font = item->font(0);
    font.setBold(editModeEnabled);
    item->setFont(0, font);
}

void TagTree::dropEvent(QDropEvent *event) {
    if (event->source() != this) return;

    QTreeWidgetItem* dest = itemAt(event->position().toPoint());
    int id = QVariant(event->mimeData()->data("application/x-tag-id")).toInt();
    TagNode* tag = TAGS[id];

    if (dest == nullptr) {
        m_rootItem->addChild(tag->leaf());
        tag->setParent(m_rootNode);
    } else {
        dest->addChild(tag->leaf());
        tag->setParent(static_cast<TagTreeItem*>(dest)->tag());
    }

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
    if (event->source() != this) event->ignore();

    int id = QVariant(event->mimeData()->data("application/x-tag-id")).toInt();

    QTreeWidgetItem *hoverItem = itemAt(event->position().toPoint());
    QTreeWidgetItem *selected = TAGS[id]->leaf();

    event->accept();

    if (dropIndicatorPosition() == QAbstractItemView::OnViewport && hoverItem == nullptr && selected->parent() == nullptr)
        event->ignore();

    if (hoverItem == selected->parent() || hoverItem == selected)
        event->ignore();
}

void TagTree::contextMenuEvent(QContextMenuEvent *event) {
    TagTreeItem* item = static_cast<TagTreeItem*>(itemAt(event->pos()));
    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *addAction = menu->addAction("New Tag...");
    connect(addAction, &QAction::triggered, this, [item, this] { onCreateTag(item); });

    bool validTagSelected = item != nullptr && item != m_rootItem;
    if (validTagSelected) {
        QString label = item->text(0);
        if (!APP_STATE->editModeEnabled()) {
            QAction *removeAction = menu->addAction("Remove \"" + label + "\"");
            connect(removeAction, &QAction::triggered, this, [item, this] { onRemoveTag(item); });
        }
    }
    menu->addSeparator();

    if (validTagSelected) {
        QAction *expandSelected = menu->addAction("Expand Selected Tag");
        connect(expandSelected, &QAction::triggered, this, [item, this] { setExpandedRecursive(true, item); });
        QAction *collapseSelected = menu->addAction("Collapse Selected Tag Recursively");
        connect(collapseSelected, &QAction::triggered, this, [item, this] { setExpandedRecursive(false, item); });
    }
    QAction *expandAll = menu->addAction("Expand Tree");
    connect(expandAll, &QAction::triggered, this, [this] { setExpandedRecursive(true, m_rootItem); });
    QAction *collapseAll = menu->addAction("Collapse Tree");
    connect(collapseAll, &QAction::triggered, this, [this] { setExpandedRecursive(false, m_rootItem); });
    menu->exec(QCursor::pos());
}
