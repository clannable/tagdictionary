#include "taglistwidget.h"
#include <QMenu>
#include <QMimeData>
#include <QCursor>

#include "taglistwidgetitem.h"
#include <QVBoxLayout>
TagListWidget::TagListWidget(QString title, QWidget *parent) :
    QWidget(parent), title(title) {
    QFrame *f = new QFrame(this);
    f->setFrameShape(QFrame::HLine);
    // f->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    // this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    // f->setStyleSheet(".QFrame { border: 1px solid #cacaca; border-radius: 4px;}");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(2);
    // header = new QWidget(f);
    // header->setCursor(Qt::PointingHandCursor);
    // QHBoxLayout* headerLayout = new QHBoxLayout(header);
    // headerLayout->setContentsMargins(0,0,0,2);
    // headerLayout->addWidget();
    // countLabel = new QLabel();
    // countLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    // headerLayout->addWidget(countLabel, 0, Qt::AlignRight);
    // expandToggle = new QPushButton();
    // expandToggle->setIconSize(QSize(16, 16));
    // expandToggle->setCursor(Qt::PointingHandCursor);
    // headerLayout->addWidget(expandToggle, 0, Qt::AlignRight);
    // expandToggle->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListRemove));
    // expandToggle->setFlat(true);
    this->list = new TagList(f);

    layout->addWidget(f);
    layout->addWidget(new QLabel(title), Qt::AlignLeft);
    // layout->addWidget(header);
    layout->addWidget(this->list);

    // this->list->hide();

    this->setAcceptDrops(true);

    connect(this->list, &QListWidget::itemDoubleClicked, this, &TagListWidget::onItemSelected);
    // connect(this->list, &TagList::tagRemoved, this, &TagListWidget::updateTitle);
    // connect(this->expandToggle, &QPushButton::clicked, this, &TagListWidget::toggleExpanded);
}

void TagListWidget::insertTag(TagNode *node) {
    for (int i = 0; i < this->list->count(); i++) {
        if (this->list->item(i)->data(Qt::UserRole) == node->getId())
            return;

    }
    this->list->addItem(new TagListWidgetItem(node));
    // this->list->sortItems();
    this->show();
    // updateTitle();
}

void TagListWidget::insertTag(int tagId) {
    this->list->addItem(new TagListWidgetItem(tagId));
    // this->list->sortItems();
    this->show();
    // updateTitle();
}

void TagListWidget::linkTagTree(const TagTree* ptr) {
    this->tagTree = ptr;
}

void TagListWidget::setTag(TagNode* node) {
    this->currentTag = node;
    setEnabled(node != nullptr);
    this->list->scrollToTop();
}

void TagListWidget::clear() {
    this->list->clear();
    // updateTitle();
    // if (!editModeEnabled) this->hide();
}

void TagListWidget::dropEvent(QDropEvent *event) {
    if (!editModeEnabled) return;
    TagNode* node = static_cast<TagTreeItem*>(tagTree->currentItem())->getNode();
    if (node == this->currentTag) return;
    for (int i = 0; i < this->list->count(); i++)
        if (this->list->item(i)->data(Qt::UserRole) == node->getId()) return;

    insertTag(node);
}

void TagListWidget::dragEnterEvent(QDragEnterEvent *event) {
    if (editModeEnabled && event->source() == tagTree) {
        TagNode* node = static_cast<TagTreeItem*>(tagTree->currentItem())->getNode();
        if (node != this->currentTag) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        } else {
            event->setDropAction(Qt::IgnoreAction);
            event->ignore();
        }
    } else {
        event->setDropAction(Qt::IgnoreAction);
        event->ignore();
    }
}

// void TagListWidget::mousePressEvent(QMouseEvent *event) {
//     if (event->buttons() & Qt::LeftButton && header->underMouse())
//         toggleExpanded();
//     else
//         QWidget::mousePressEvent(event);
// }

void TagListWidget::setEditMode(bool mode) {
    editModeEnabled = mode;
    this->list->setEditMode(mode);
    // if (editModeEnabled == false && this->list->count() == 0)
    //     this->hide();
    // else if (editModeEnabled == true)
    //     this->show();
    // setAcceptDrops(mode);
}

void TagListWidget::updateTitle() {
    int count = this->list->count();
    countLabel->setText(count == 0 ? "" : ("(" + QString::number(count) + ")"));
}

void TagListWidget::toggleExpanded() {
    this->expanded = !this->expanded;

    this->expandToggle->setIcon(QIcon::fromTheme(this->expanded ? QIcon::ThemeIcon::ListRemove : QIcon::ThemeIcon::ListAdd));
    if (this->expanded)
        this->list->show();
    else
        this->list->hide();
}

void TagListWidget::onItemSelected(QListWidgetItem *item) {
    emit tagSelected(item->data(Qt::UserRole).toInt());
}

std::list<int> TagListWidget::values() {
    std::list<int> list;
    for (int i = 0; i < this->list->count(); i++) {
        list.push_back(this->list->item(i)->data(Qt::UserRole).toInt());
    }
    return list;
}
