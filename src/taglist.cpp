#include "taglist.h"
#include <QMenu>
#include "globals.h"
#include <QFileInfo>
#include <qmimedata.h>
#include "tagtree.h"
#include <QRegularExpression>
#include <QHeaderView>

TagList::TagList(QWidget* parent) : QTreeWidget(parent) {
    setFixedHeight(107);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setDefaultDropAction(Qt::MoveAction);
    setDropIndicatorShown(true);
    setColumnCount(2);
    setHeaderHidden(true);
    // setIndentation(5);
    setRootIsDecorated(false);
    header()->setSectionResizeMode(0, QHeaderView::Fixed);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
    header()->resizeSection(0, 15);

    connect(APP_STATE, &AppState::editModeChanged, this, &TagList::onEditModeChanged);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &TagList::onItemDoubleClicked);
    parser = new TokenParser();
}

void TagList::setValues(nlohmann::json::array_t ids) {
    clear();
    for (const auto& i : ids)
        addTopLevelItem(parser->parseString(
            i.type() == nlohmann::json::value_t::string
                ? QString::fromStdString(i.get<std::string>())
                : QString::number(i.get<int>()))
        );
    scrollToTop();
}

void TagList::contextMenuEvent(QContextMenuEvent *event) {
    if (!APP_STATE->editModeEnabled()) return;
    QTreeWidgetItem *item = this->itemAt(event->pos());
    if (item == nullptr) return;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *removeTag = new QAction(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove), "Remove \""+item->text(1)+"\"");
    menu->addAction(removeTag);
    connect(removeTag, &QAction::triggered, this, [item] { delete item; });
    menu->exec(QCursor::pos());
}

void TagList::onEditModeChanged(bool editModeEnabled) {
    setAcceptDrops(editModeEnabled);
    setDragEnabled(editModeEnabled);
    setDragDropMode(editModeEnabled ? DragDropMode::DragDrop : DragDropMode::NoDragDrop);
    setStyleSheet(editModeEnabled
        ? "TagList { background-color: #eeeeee; border: 1px solid #777777; border-radius: 4px; }"
        : "");
}

void TagList::removeTag(int id) {
    QTreeWidgetItemIterator it(this);
    while (*it) {
        if ((*it)->data(1, Qt::UserRole) == id) {
            delete *it;
            return;
        }
        ++it;
    }
}

void TagList::onItemDoubleClicked(QTreeWidgetItem* item) {
    APP_STATE->setSelectedTag(TAGS[item->data(1, Qt::UserRole).toInt()]);
}

void TagList::dropEvent(QDropEvent *event) {
    if (!APP_STATE->editModeEnabled()) return;
    QTreeWidgetItem* item;
    int id = QVariant(event->mimeData()->data("application/x-tag-id")).toInt();
    if (dynamic_cast<TagList*>(event->source()) != nullptr) {
        QString tokenStr = QVariant(event->mimeData()->data("application/x-tag-token")).toString();
        item = parser->parseString(tokenStr);
    } else {
        item = new QTreeWidgetItem();
        item->setData(1, Qt::UserRole, id);
        item->setIcon(1, QIcon(QString::fromStdString(TAGS[id]->icon())));
        item->setText(1, QString::fromStdString(TAGS[id]->key()));
        item->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        item->setExpanded(true);
    }
    QTreeWidgetItem* dest = itemAt(event->position().toPoint());
    QTreeWidgetItem* parent = dest->parent();
    int index;
    switch (dropIndicatorPosition()) {
        case QAbstractItemView::OnItem:
        if (!dest->childCount()) {
            QMenu contextMenu(this);
            // event->acceptProposedAction();
            contextMenu.setAttribute(Qt::WA_DeleteOnClose);
            connect(&contextMenu, &QMenu::triggered, this, [this, item, id, dest, event](QAction *action) {
                if (dynamic_cast<TagList*>(event->source()) != nullptr)
                    dynamic_cast<TagList*>(event->source())->removeTag(id);
                insertChildTag(item, dest, static_cast<Modifier>(action->data().toInt()));

            });
            contextMenu.addAction("If -> then")->setData(Modifier::IF_THEN);
            contextMenu.addAction("And")->setData(Modifier::AND);
            contextMenu.addAction("Or")->setData(Modifier::OR);
            contextMenu.exec(QCursor::pos());
        }
        return;
        case QAbstractItemView::AboveItem:
        case QAbstractItemView::BelowItem:
            index = parent->indexOfChild(dest);
            if (dropIndicatorPosition() == QAbstractItemView::BelowItem) index += 1;
            if (parent != invisibleRootItem())
                insertChildTag(item, parent, static_cast<Modifier>(parent->data(0, Qt::UserRole).toInt()), index);
            else
                insertTopLevelItem(index, item);
            break;
        case QAbstractItemView::OnViewport:
            addTopLevelItem(item);
            break;
    }
    if (dynamic_cast<TagList*>(event->source()) != nullptr && event->source() != this)
        dynamic_cast<TagList*>(event->source())->removeTag(id);
}

void TagList::insertChildTag(QTreeWidgetItem* item, QTreeWidgetItem* parent, Modifier mod, int index) {
    if (mod == Modifier::IF_THEN) {
        item->setIcon(0, QIcon(":/icons/linear/turn-down.png"));
        parent->setText(0, "IF");
    }
    else if (mod == Modifier::AND) item->setText(0, "AND");
    else if (mod == Modifier::OR) item->setText(0, "OR");

    parent->setData(0, Qt::UserRole, mod);
    if (index == -1)
        parent->addChild(item);
    else
        parent->insertChild(index, item);
    parent->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
    parent->setExpanded(true);


}

void TagList::dragEnterEvent(QDragEnterEvent *event) {
    if (event->source() == this) {
        QTreeWidget::dragEnterEvent(event);
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
                QTreeWidgetItemIterator it(this);
                while (*it) {
                    if ((*it)->data(1, Qt::UserRole).toInt() == id) {
                        event->setDropAction(Qt::IgnoreAction);
                        event->ignore();
                        return;
                    }
                    it++;
                }
                event->accept();
            } else return;
        }
    } else {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
    }
}

std::list<std::string> TagList::values() {
    std::list<std::string> ids = {};
    for (int i = 0; i < topLevelItemCount(); i++) {
        QTreeWidgetItem* item = topLevelItem(i);
        ids.push_back(itemToString(item).toStdString());
    }
    return ids;
}

QString TagList::itemToString(QTreeWidgetItem* item) const {
    QString ret = item->data(1, Qt::UserRole).toString();
    if (item->childCount()) {
        switch (static_cast<Modifier>(item->data(0, Qt::UserRole).toInt())) {
        case Modifier::IF_THEN:
            ret += "?(";
            break;
        case Modifier::AND:
            ret += "+(";
            break;
        case Modifier::OR:
            ret += "|(";
            break;
        default:
            break;
        }
        QStringList children;
        for (int i = 0; i < item->childCount(); i++) {
            children.push_back(itemToString(item->child(i)));
        }
        ret += children.join(",") + ")";
    }
    else if (!item->data(1, Qt::UserRole+1).isNull()) {
        ret += "*";
    }
    return ret;
}

QStringList TagList::mimeTypes() const {
    QStringList types = QTreeWidget::mimeTypes();
    types.push_back("application/x-tag-id");
    types.push_back("application/x-tag-token");
    return types;
}

QMimeData* TagList::mimeData(const QList<QTreeWidgetItem*> &items) const {
    QMimeData* data = QTreeWidget::mimeData(items);
    if (data != nullptr) {
        data->setData("application/x-tag-id", QVariant(items.first()->data(1, Qt::UserRole)).toByteArray());
        data->setData("application/x-tag-token", QVariant(itemToString(items.first())).toByteArray());
    }
    return data;
}
