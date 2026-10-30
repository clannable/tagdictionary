#include "taglist.h"
#include <QMenu>
#include "globals.h"
#include <QFileInfo>
#include <qmimedata.h>
#include "tagtree.h"
#include <QRegularExpression>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QPainter>

class TagListDelegate : public QStyledItemDelegate {
protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        option->displayAlignment = Qt::AlignHCenter;
        option->textElideMode = Qt::ElideNone;
    }
};

TagList::TagList(QWidget* parent) : QTreeWidget(parent) {
    setFixedHeight(107);
    setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    setDropIndicatorShown(true);
    setColumnCount(2);
    setHeaderHidden(true);
    setDefaultDropAction(Qt::MoveAction);
    setItemsExpandable(false);
    setAllColumnsShowFocus(true);
    setRootIsDecorated(false);
    setItemDelegateForColumn(0, new TagListDelegate());
    setExpandsOnDoubleClick(false);
    header()->setSectionResizeMode(0, QHeaderView::Fixed);
    header()->setSectionResizeMode(1, QHeaderView::Stretch);
    header()->resizeSection(0, 10);
    connect(APP_STATE, &AppState::editModeChanged, this, &TagList::onEditModeChanged);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &TagList::onItemDoubleClicked);
    parser = new TokenParser();
}

void TagList::setValues(nlohmann::json::array_t ids) {
    clear();
    setColumnHidden(0, true);
    for (const auto& i : ids) {
        QTreeWidgetItem* item = parser->parseString(
            i.type() == nlohmann::json::value_t::string
                ? QString::fromStdString(i.get<std::string>())
                : QString::number(i.get<int>()));
        addTopLevelItem(item);
        if (item->data(0, Qt::UserRole) != Modifier::NONE)
            setColumnHidden(0, false);
    }
    header()->resizeSection(0, 10);
    expandAll();
    scrollToTop();
}

void TagList::contextMenuEvent(QContextMenuEvent *event) {
    if (!APP_STATE->editModeEnabled()) return;
    QTreeWidgetItem *item = this->itemAt(event->pos());
    if (item == nullptr) return;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QMenu *mods = menu->addMenu("Set modifier");
    mods->setAttribute(Qt::WA_DeleteOnClose);
    connect(mods->addAction("None"), &QAction::triggered, this, [this, item]() { setModifier(item, Modifier::NONE);  });
    connect(mods->addAction("IF"), &QAction::triggered, this, [this, item]() { setModifier(item, Modifier::IF_THEN);  });
    connect(mods->addAction("AND"), &QAction::triggered, this, [this, item]() { setModifier(item, Modifier::AND);  });
    connect(mods->addAction("OR"), &QAction::triggered, this, [this, item]() { setModifier(item, Modifier::OR);  });
    if (!TAGS[item->data(1, Qt::UserRole).toInt()]->children().empty()) {
        QAction* toggleAction = menu->addAction("Include sub-tags");
        toggleAction->setCheckable(true);
        toggleAction->setChecked(item->data(1, Qt::UserRole+1).toBool());
        connect(toggleAction, &QAction::toggled, this, [this, item](bool checked) { toggleIncludeSubtags(item, checked); });

    }
    menu->addSeparator();
    connect(menu->addAction(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove), "Remove \""+item->text(1)+"\""), &QAction::triggered, this, [item] {
        if (item->parent() != nullptr && item->parent()->childCount() == 1) {
            item->parent()->setText(0, "");
            item->parent()->setData(0, Qt::UserRole, Modifier::NONE);
        }
        delete item;
    });

    menu->exec(QCursor::pos());
}

void TagList::toggleIncludeSubtags(QTreeWidgetItem* item, bool checked) {
    item->setData(1, Qt::UserRole+1, checked);
    QString label = item->text(1);
    if (checked && !label.endsWith("*"))
        label += "*";
    else if (!checked && label.endsWith("*"))
        label = label.left(label.length()-1);
    item->setText(1, label);
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
    if (event->source() == this) {
        QTreeWidget::dropEvent(event);
        return;
    }
    QTreeWidgetItem* item;
    int id = QVariant(event->mimeData()->data("application/x-tag-id")).toInt();
    if (event->mimeData()->hasFormat("application/x-tag-token")) {
        item = parser->parseString(QVariant(event->mimeData()->data("application/x-tag-token")).toString());
    } else {
        item = new QTreeWidgetItem();
        item->setData(1, Qt::UserRole, id);
        item->setIcon(1, QIcon(QString::fromStdString(TAGS[id]->icon())));
        item->setText(1, QString::fromStdString(TAGS[id]->key()));
        item->setFlags(item->flags() & ~Qt::ItemIsDropEnabled);
    }
    QTreeWidgetItem* dest = itemAt(event->position().toPoint());

    int index;
    if (dest != nullptr)
        index = indexOfTopLevelItem(dest);
    else
        index = topLevelItemCount();
    if (dropIndicatorPosition() == DropIndicatorPosition::BelowItem)
        index += 1;

    insertTopLevelItem(index, item);

    if (dynamic_cast<TagList*>(event->source()) != nullptr)
        dynamic_cast<TagList*>(event->source())->removeTag(id);
    // event->accept();
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
            if (qobject_cast<TagTree*>(event->source()) != nullptr) {
                event->setDropAction(Qt::CopyAction);
                event->acceptProposedAction();
            } else if (qobject_cast<TagList*>(event->source()) != nullptr){
                QTreeWidgetItemIterator it(this);
                while (*it) {
                    if ((*it)->data(1, Qt::UserRole).toInt() == id) {
                        event->setDropAction(Qt::IgnoreAction);
                        event->ignore();
                        return;
                    }
                    it++;
                }
                event->setDropAction(Qt::MoveAction);
                event->acceptProposedAction();
            }
        }
    } else {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
    }
}

void TagList::setModifier(QTreeWidgetItem* item, Modifier mod) {
    item->setData(0, Qt::UserRole, mod);
    switch (mod) {
    case Modifier::IF_THEN:
        item->setText(0, "IF");
        break;
    case Modifier::AND:
        item->setText(0, "AND");
        break;
    case Modifier::OR:
        item->setText(0, "OR");
        break;
    default:
        item->setText(0, "");
        break;
    }
    setColumnHidden(0, false);
}
// void TagList::expandAll() {
//     QTreeWidgetItemIterator it(this);
//     while (*it) {
//         (*it)->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
//         (*it)->setExpanded(true);
//         (*it)->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
//         ++it;
//     }
// }

// void TagList::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
//     QTreeWidget::drawRow(painter, option, index);
//     if (index.parent().isValid() && !index.siblingAtRow(index.row()+1).isValid()) {
//         painter->setPen(Qt::DashLine);
//         painter->drawLine(option.widget->rect().bottomLeft(), option.widget->rect().bottomRight());
//     }
// }

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
    if (!item->data(1, Qt::UserRole+1).isNull() && item->data(1, Qt::UserRole+1).toBool()) {
        ret += "*";
    }
    switch (static_cast<Modifier>(item->data(0, Qt::UserRole).toInt())) {
    case Modifier::IF_THEN:
        ret = "?" + ret;
        break;
    case Modifier::AND:
        ret = "+" + ret;
        break;
    case Modifier::OR:
        ret = "|" + ret;
        break;
    default:
        break;
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
