#include "taglist.h"
#include "globals.h"
#include <QMenu>
#include <QFileInfo>
#include <QMimeData>
#include <QRegularExpression>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QActionGroup>
#include <QPainter>

class TagListDelegate : public QStyledItemDelegate {
protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const override {
        QStyledItemDelegate::initStyleOption(option, index);
        if (index.column() == 0) {
            option->displayAlignment = Qt::AlignHCenter;
            option->textElideMode = Qt::ElideNone;
        }
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

    QActionGroup* mods = new QActionGroup(this);
    QAction* a_if = new QAction("IF", mods);
    a_if->setCheckable(true);
    a_if->setData(Modifier::IF);
    addAction(a_if);
    QAction* a_then = new QAction("Then (\u2937)", mods);
    a_then->setCheckable(true);
    a_then->setData(Modifier::THEN);
    addAction(a_then);
    QAction* a_and = new QAction("AND", mods);
    a_and->setCheckable(true);
    a_and->setData(Modifier::AND);
    addAction(a_and);
    QAction* a_or = new QAction("OR", mods);
    a_or->setCheckable(true);
    a_or->setData(Modifier::OR);
    addAction(a_or);
    QAction* a_sep = new QAction("Separator");
    a_sep->setCheckable(true);
    addAction(a_sep);

    connect(a_sep, &QAction::toggled, this, [this](bool checked) {
        m_contextItem->setData(0, Qt::UserRole+5, checked);
        m_contextItem->setData(1, Qt::UserRole+5, checked);
    });
    connect(mods, &QActionGroup::triggered, this, [this](QAction* action) {
        this->setModifier(m_contextItem, static_cast<Modifier>(action->data().toInt()));
    });

    QAction* toggleAction = addAction("Include sub-tags");
    toggleAction->setCheckable(true);
    connect(toggleAction, &QAction::toggled, this, [this](bool checked) { toggleIncludeSubtags(m_contextItem, checked); });
    connect(addAction("Remove item"), &QAction::triggered, this, [this]() { removeItem(m_contextItem); });
}

void TagList::setValues(nlohmann::json::array_t ids) {
    clear();
    setColumnHidden(0, true);
    for (const auto& i : ids) {
        QTreeWidgetItem* item = TokenParser::parseString(
            i.type() == nlohmann::json::value_t::string
                ? QString::fromStdString(i.get<std::string>())
                : QString::number(i.get<int>()));
        if (item != nullptr) {
            addTopLevelItem(item);
            if (item->data(0, Qt::UserRole).toInt() != Modifier::NONE)
                setColumnHidden(0, false);
        }
    }
    header()->resizeSection(0, 10);
    expandAll();
    scrollToTop();
}

void TagList::contextMenuEvent(QContextMenuEvent *event) {
    if (!APP_STATE->editModeEnabled()) return;
    if ((m_contextItem = this->itemAt(event->pos())) == nullptr) return;

    QMenu *menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    Modifier mod = static_cast<Modifier>(m_contextItem->data(0, Qt::UserRole).toInt());
    int id = m_contextItem->data(1, Qt::UserRole).toInt();
    QList<QAction*> mod_actions = actions();
    mod_actions[0]->setChecked(mod == Modifier::IF);
    mod_actions[1]->setChecked(mod == Modifier::THEN);
    mod_actions[2]->setChecked(mod == Modifier::AND);
    mod_actions[3]->setChecked(mod == Modifier::OR);
    mod_actions[4]->setChecked(m_contextItem->data(0, Qt::UserRole+5).toBool());
    mod_actions[5]->setChecked(m_contextItem->data(1, Qt::UserRole+1).toBool());
    mod_actions[5]->setEnabled(!TAGS[id]->children().empty());
    mod_actions[6]->setText("Remove \"" + QString::fromStdString(TAGS[id]->key()) + "\"");
    menu->addActions(mod_actions);
    menu->insertSeparator(mod_actions[6]);
    menu->exec(QCursor::pos());
}

void TagList::removeItem(QTreeWidgetItem* item) {
    if (item->parent() != nullptr && item->parent()->childCount() == 1) {
        item->parent()->setText(0, "");
        item->parent()->setData(0, Qt::UserRole, Modifier::NONE);
    }
    delete item;
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
    setDropIndicatorShown(editModeEnabled);
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
    checkModifiers();
}

void TagList::checkModifiers() {
    QTreeWidgetItemIterator it(this);

    while(*it) {
        if ((*it)->data(0, Qt::UserRole).toInt() != 0) {
            setColumnHidden(0, false);
            return;
        }
        it++;
    }
    setColumnHidden(0, true);
}

void TagList::onItemDoubleClicked(QTreeWidgetItem* item) {
    APP_STATE->setSelectedTag(TAGS[item->data(1, Qt::UserRole).toInt()]);
}

void TagList::drawRow(QPainter* painter, const QStyleOptionViewItem &option, const QModelIndex& index) const {
    QTreeWidget::drawRow(painter, option, index);
    if (index.data(Qt::UserRole+5).toBool()) {
        QPen pen = painter->pen();
        pen.setColor(QColor(150, 150, 150));
        pen.setStyle(Qt::SolidLine);
        pen.setCapStyle(Qt::RoundCap);
        painter->setPen(pen);
        painter->drawLine(option.rect.x(), option.rect.y(), option.rect.x()+option.rect.width(), option.rect.y());
    }
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
        item = TokenParser::parseString(QVariant(event->mimeData()->data("application/x-tag-token")).toString());
        if (!item->text(0).isNull())
            setColumnHidden(0, false);
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
            if (!event->mimeData()->hasFormat("application/x-tag-token")) {
                event->setDropAction(Qt::CopyAction);
                event->acceptProposedAction();
            } else {
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
    if (mod == static_cast<Modifier>(item->data(0, Qt::UserRole).toInt())) {
        item->setData(0, Qt::UserRole, 0);
        item->setText(0, "");
    } else {
        item->setData(0, Qt::UserRole, mod);
        switch (mod) {
        case Modifier::IF:
            item->setText(0, "IF");
            break;
        case Modifier::AND:
            item->setText(0, "AND");
            break;
        case Modifier::OR:
            item->setText(0, "OR");
            break;
        case Modifier::THEN:
            item->setText(0, "\u2937");
            break;
        default:
            item->setText(0, "");
            break;
        }
    }
    checkModifiers();
}

std::list<std::string> TagList::values() {
    std::list<std::string> ids = {};
    for (int i = 0; i < topLevelItemCount(); i++) {
        QTreeWidgetItem* item = topLevelItem(i);
        ids.push_back(itemToString(item).toStdString());
    }
    return ids;
}

void TagList::clean() {
    for (int i = 0; i < topLevelItemCount(); i++) {
        if (TAGS.find(topLevelItem(i)->data(1, Qt::UserRole).toInt()) == TAGS.end()) {
            delete topLevelItem(i);
            i--;
        }
    }
}
QString TagList::itemToString(QTreeWidgetItem* item) const {
    QString ret = item->data(1, Qt::UserRole).toString();
    if (item->data(0, Qt::UserRole+5).toBool())
        ret += "_";
    if (item->data(1, Qt::UserRole+1).toBool())
        ret += "*";

    switch (static_cast<Modifier>(item->data(0, Qt::UserRole).toInt())) {
    case Modifier::IF:
        ret = "?" + ret;
        break;
    case Modifier::AND:
        ret = "+" + ret;
        break;
    case Modifier::OR:
        ret = "|" + ret;
        break;
    case Modifier::THEN:
        ret = ">" + ret;
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
