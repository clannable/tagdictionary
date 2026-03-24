#include "taglistwidget.h"
#include <QMenu>
#include <QMimeData>
#include <QCursor>

#include "taglistwidgetitem.h"
#include <QVBoxLayout>
TagListWidget::TagListWidget(QString title, QWidget *parent) :
    QWidget(parent), title(title) {
    QFrame *f = new QFrame(this);
    f->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    f->setStyleSheet(".QFrame { border: 1px solid #cacaca; border-radius: 4px;}");
    QVBoxLayout* layout = new QVBoxLayout(f);
    QVBoxLayout* l = new QVBoxLayout();
    l->setContentsMargins(0,0,0,0);
    l->addWidget(f);
    this->setLayout(l);
    layout->setContentsMargins(2,2,2,2);
    layout->setSpacing(0);
    header = new QWidget(f);
    header->setCursor(Qt::PointingHandCursor);
    QHBoxLayout* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(0,0,0,2);
    headerLayout->addWidget(new QLabel(title), 1, Qt::AlignLeft);
    countLabel = new QLabel();
    countLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    headerLayout->addWidget(countLabel, 0, Qt::AlignRight);
    expandToggle = new QPushButton();
    expandToggle->setIconSize(QSize(16, 16));
    expandToggle->setCursor(Qt::PointingHandCursor);
    headerLayout->addWidget(expandToggle, 0, Qt::AlignRight);
    expandToggle->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::ListAdd));
    expandToggle->setFlat(true);
    this->list = new TagList(f);
    this->list->setMaximumHeight(200);
    this->list->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    layout->addWidget(header);
    layout->addWidget(this->list);

    this->list->hide();

    this->setAcceptDrops(true);

    connect(this->list, &QListWidget::itemDoubleClicked, this, &TagListWidget::onItemSelected);
    connect(this->list, &TagList::tagRemoved, this, &TagListWidget::updateTitle);
    connect(this->expandToggle, &QPushButton::clicked, this, &TagListWidget::toggleExpanded);
}

void TagListWidget::insertTag(TagNode *node) {
    QString tagPath = QString::fromStdString(node->getFullPath());
    for (int i = 0; i < this->list->count(); i++) {
        if (static_cast<TagListWidgetItem*>(this->list->item(i))->getValue() == tagPath)
            return;

    }
    this->list->addItem(new TagListWidgetItem(tagPath));
    this->list->sortItems();
    updateTitle();
}

void TagListWidget::insertTag(QString tagPath) {
    this->list->addItem(new TagListWidgetItem(tagPath));
    this->list->sortItems();
    updateTitle();
}

void TagListWidget::linkTagTree(const TagTree* ptr) {
    this->tagTree = ptr;
}

void TagListWidget::setTag(TagNode* node) {
    this->currentTag = node;
}

void TagListWidget::clear() {
    this->list->clear();
    updateTitle();
}

void TagListWidget::dropEvent(QDropEvent *event) {
    if (!editModeEnabled) return;
    TagNode* node = static_cast<TagTreeItem*>(tagTree->currentItem())->getNode();
    if (node == this->currentTag) return;
    QString tagPath = QString::fromStdString(static_cast<TagTreeItem*>(tagTree->currentItem())->getNode()->getFullPath());
    for (int i = 0; i < this->list->count(); i++)
        if ((static_cast<TagListWidgetItem*>(this->list->item(i)))->getValue() == tagPath) return;

    insertTag(tagPath);
}

void TagListWidget::dragEnterEvent(QDragEnterEvent *event) {
    // std::cout << event->source()->objectName().toStdString() << "\n" <<std::flush;
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

void TagListWidget::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton && header->underMouse())
        toggleExpanded();
    else
        QWidget::mousePressEvent(event);
}

void TagListWidget::setEditMode(bool mode) {
    editModeEnabled = mode;
    this->list->setEditMode(mode);
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
    emit tagSelected(static_cast<TagListWidgetItem*>(item)->getValue());
}

std::list<std::string> TagListWidget::values() {
    std::list<std::string> list;
    for (int i = 0; i < this->list->count(); i++) {
        TagListWidgetItem *item = static_cast<TagListWidgetItem*>(this->list->item(i));
        list.push_back(item->getValue().toStdString());
    }
    return list;
}
