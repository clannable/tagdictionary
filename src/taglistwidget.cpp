#include <QMenu>
#include <QMimeData>
#include <QCursor>
#include <QVBoxLayout>

#include "tagtree.h"
#include "taglistwidget.h"
#include <QListWidgetItem>
#include "globals.h"

TagListWidget::TagListWidget(QString title, QWidget *parent) :
    QWidget(parent), m_title(title) {
    QFrame *f = new QFrame(this);
    f->setFrameShape(QFrame::HLine);
    // f->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    // this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    // f->setStyleSheet(".QFrame { border: 1px solid #cacaca; border-radius: 4px;}");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(3);
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
    u_list = new TagList(f);

    layout->addWidget(f);
    layout->addWidget(new QLabel(title), Qt::AlignLeft);
    // layout->addWidget(header);
    layout->addWidget(u_list);

    // u_list->hide();

    // this->setAcceptDrops(true);

    connect(u_list, &QListWidget::itemDoubleClicked, this, &TagListWidget::onItemSelected);
    // connect(u_list, &TagList::tagRemoved, this, &TagListWidget::updateTitle);
    // connect(this->expandToggle, &QPushButton::clicked, this, &TagListWidget::toggleExpanded);
}

void TagListWidget::insertTag(int id) {
    for (int i = 0; i < u_list->count(); i++) {
        if (u_list->item(i)->data(Qt::UserRole) == id)
            return;

    }
    u_list->createListItem(id);
    // u_list->sortItems();
    // this->show();
    // updateTitle();
}


void TagListWidget::setTag(TagNode* tag) {
    setEnabled(tag != nullptr && !tag->isRoot());
    u_list->scrollToTop();
}

void TagListWidget::clear() {
    u_list->clear();
    // updateTitle();
    // if (!editModeEnabled) this->hide();
}

// void TagListWidget::mousePressEvent(QMouseEvent *event) {
//     if (event->buttons() & Qt::LeftButton && header->underMouse())
//         toggleExpanded();
//     else
//         QWidget::mousePressEvent(event);
// }

// void TagListWidget::updateTitle() {
//     int count = u_list->count();
//     countLabel->setText(count == 0 ? "" : ("(" + QString::number(count) + ")"));
// }

// void TagListWidget::toggleExpanded() {
//     this->expanded = !this->expanded;

//     this->expandToggle->setIcon(QIcon::fromTheme(this->expanded ? QIcon::ThemeIcon::ListRemove : QIcon::ThemeIcon::ListAdd));
//     if (this->expanded)
//         u_list->show();
//     else
//         u_list->hide();
// }

