#include "tagtree.h"
#include "newtagdialog.h"
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <iostream>
#include <QMenu>
#include <QCursor>
#include <QMessageBox>

TagTree::TagTree(QWidget* parent) :
    QTreeWidget(parent),
    editModeEnabled(false)
{
    connect(this, &QTreeWidget::itemClicked, this, &QTreeWidget::expandItem);
}

TagTree::~TagTree() {
    delete rootNode;
}

TagTreeItem* TagTree::findTag(QString tagPath) {
    QTreeWidgetItemIterator it(this);
    while (*it) {
        TagTreeItem* item = static_cast<TagTreeItem*>(*it);
        if (item->getNode()->getFullPath() == tagPath)
            return item;
    }
    return nullptr;
}

void TagTree::fromJson(nlohmann::json json) {
    rootNode = JsonNode::createRoot(json);

    for (auto& [key, node] : rootNode->getChildren()) {
        TagTreeItem *item = new TagTreeItem(node);
        addTopLevelItem(item);
        this->createChildren(item, node);
    }
}

json TagTree::toJson() {
    json root = json();

    for (int t = 0; t < topLevelItemCount(); t++) {
        std::string key = topLevelItem(t)->text(0).toStdString();
        root[key] = static_cast<TagTreeItem*>(topLevelItem(t))->getNode()->toJson();
    }
    return root;
}

void TagTree::onCreateTag() {
    NewTagDialog *dialog = new NewTagDialog(this);
    dialog->setParentNode(menuItem != nullptr ? menuItem->getNode() : rootNode);
    connect(dialog, &NewTagDialog::submit, this, &TagTree::onNewTag);
    dialog->exec();
    disconnect(dialog, &NewTagDialog::submit, this, &TagTree::onNewTag);
}

void TagTree::onNewTag(JsonNode *node) {
    TagTreeItem* tag = new TagTreeItem(node);

    if (menuItem != nullptr) {
        menuItem->getNode()->addChild(node);
        menuItem->addChild(tag);
        expandItem(menuItem);
    } else {
        rootNode->addChild(node);
        this->addTopLevelItem(tag);
    }
    sortItems(0, Qt::AscendingOrder);

    if (!editModeEnabled)
        setCurrentItem(tag);
    menuItem = nullptr;

    emit tagsChanged();
}

void TagTree::onRemoveTag() {
    JsonNode* node = menuItem->getNode();

    if (QMessageBox::question(this, "Remove Tag",
        "Are you sure you want to remove the tag \"" +
        QString::fromStdString(node->getKey()) +
        "\"? This will also remove all sub-tags inside this tag.") == QMessageBox::Yes)
    {
        JsonNode *parent = node->getParent();
        node->getParent()->removeChildAt(node->getKey());
        delete menuItem;
        emit tagsChanged();
    }


}

void TagTree::createChildren(TagTreeItem* item, JsonNode *node) {
    for (auto& [k, c] : node->getChildren()) {
        TagTreeItem *child = new TagTreeItem(c);
        item->addChild(child);
        this->createChildren(child, c);
    }
}

void TagTree::dropEvent(QDropEvent *event) {
    if (event->source() != this) return;
    TagTreeItem* item = static_cast<TagTreeItem*>(currentItem());

    QTreeWidget::dropEvent(event);
    if (event->isAccepted()) {
        if (item->parent() != nullptr) {
            TagTreeItem* parent = static_cast<TagTreeItem*>(item->parent());
            item->getNode()->setParent(parent->getNode());
        } else {
            item->getNode()->setParent(rootNode);
        }
        item->setData(0, Qt::UserRole, QString::fromStdString(item->getNode()->getFullPath()));
        // std::cout << item->text(0).toStdString() << "\n" << std::flush;
        sortItems(0, Qt::AscendingOrder);
        emit tagsChanged();
    }
}

// void TagTree::mousePressEvent(QMouseEvent *event) {
//     QTreeWidget::mousePressEvent(event);
//     QTreeWidgetItem *item = this->itemAt(event->pos());
//     this->expandItem(item);
//     if (dragEnabled()) {
//         QTreeWidgetItem *item = this->itemAt(event->pos());
//         if (item != nullptr) {
//             dragItem = static_cast<TagTreeItem*>(item);
//             dragStartPosition = event->pos();
//         }
//     }
// }

// void TagTree::mouseMoveEvent(QMouseEvent *event) {
//     if (!editModeEnabled)
//         QTreeWidget::mouseMoveEvent(event);
//     if (!this->dragEnabled() || !(event->buttons() & Qt::LeftButton) || dragItem == nullptr) return;

//     if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance()) return;

//     QDrag *drag = new QDrag(this);
//     QMimeData *mimeData = new QMimeData;

//     mimeData->setText(QString::fromStdString(dragItem->getNode()->getFullPath()));
//     drag->setMimeData(mimeData);

//     Qt::DropAction dropAction = drag->exec(Qt::CopyAction);
//     dragItem = nullptr;
//     dragStartPosition = QPoint();
// }

// void TagTree::mouseReleaseEvent(QMouseEvent *event) {
//     dragItem = nullptr;
//     dragStartPosition = QPoint();
//     QTreeWidget::mouseReleaseEvent(event);
// }

void TagTree::contextMenuEvent(QContextMenuEvent *event) {
    menuItem = static_cast<TagTreeItem*>(itemAt(event->pos()));
    QMenu *menu = new QMenu(this);
    QAction *addAction = menu->addAction("Create New Tag");
    connect(addAction, &QAction::triggered, this, &TagTree::onCreateTag);

    if (menuItem != nullptr) {
        QString label = menuItem->text(0);
        if (!editModeEnabled) {
            QAction *removeAction = menu->addAction("Remove \"" + label + "\"");
            connect(removeAction, &QAction::triggered, this, &TagTree::onRemoveTag);
        } else {
            menu->addSeparator();
            QAction *requiredAction = menu->addAction("Add \"" + label + "\" to Required Tags");
            connect(requiredAction, &QAction::triggered, this, &TagTree::signalRequired);
            QAction *relatedAction = menu->addAction("Add \"" + label + "\" to Related Tags");
            connect(relatedAction, &QAction::triggered, this, &TagTree::signalRelated);
        }
    }
    menu->exec(QCursor::pos());
}

void TagTree::signalRelated() {
    emit addToRelated(menuItem->getNode());
    menuItem = nullptr;
}

void TagTree::signalRequired() {
    emit addToRequired(menuItem->getNode());
    menuItem = nullptr;
}

void TagTree::setEditMode(bool mode) {
    editModeEnabled = mode;
}


