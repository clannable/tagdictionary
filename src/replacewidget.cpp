#include <QHBoxLayout>
#include "appstate.h"
#include "globals.h"

#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QFileInfo>
#include <QComboBox>
#include <QMouseEvent>

#include "replacewidget.h"
ReplaceWidget::ReplaceWidget(QWidget *parent)
    : QFrame{parent}
{
    setFrameShadow(QFrame::Plain);
    setFrameStyle(QFrame::StyledPanel);

    // QVBoxLayout* vLayout = new QVBoxLayout();
    // vLayout->setContentsMargins(0,0,0,0);
    // vLayout->setSpacing(3);
    // setLayout(vLayout);

    // QFrame* f = new QFrame(this);
    // f->setFrameStyle(QFrame::NoFrame);
    // f->setFrameShadow(QFrame::Plain);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(3);
    setLayout(layout);
    // f->setLayout(layout);

    u_options = new QComboBox();
    u_options->addItems({" ", "Replaces", "Requires"});
    u_options->setEditable(false);
    u_options->setFrame(false);
    u_options->setCurrentIndex(0);
    u_options->hide();
    u_relLabel = new QLabel();
    u_relLabel->setAlignment(Qt::AlignVCenter);
    u_relLabel->setFont(font());
    u_iconLabel = new QLabel();
    u_iconLabel->setAlignment(Qt::AlignVCenter);

    u_parentLabel = new QLabel();
    u_parentLabel->setAlignment(Qt::AlignVCenter);
    u_parentLabel->setFont(font());
    layout->addWidget(u_options);
    layout->addWidget(u_relLabel);
    layout->addWidget(u_iconLabel);
    layout->addWidget(u_parentLabel);
    // layout->addWidget(f);

    hide();

    connect(APP_STATE, &AppState::editModeChanged, this, &ReplaceWidget::onEditModeChange);
    connect(APP_STATE, &AppState::selectedTagChanged, this, &ReplaceWidget::setTag);
}


void ReplaceWidget::setTag(TagNode *tag) {
    if (!tag || !tag->parent() || tag->parent()->isRoot()) hide();
    else {
        u_iconLabel->setPixmap(QIcon(QString::fromStdString(tag->parent()->icon())).pixmap(QSize(15, 15)));
        u_parentLabel->setText(QString::fromStdString(tag->parent()->key()));
        setValue(tag->parentRelation());
    }
}

void ReplaceWidget::onEditModeChange(bool editModeEnabled) {
    if (editModeEnabled) {
        if (APP_STATE->selectedTag()->parent() && !APP_STATE->selectedTag()->parent()->isRoot()) show();
        else hide();
        u_options->show();
        u_relLabel->hide();
        setStyleSheet("");
    } else {
        setValue(APP_STATE->selectedTag()->parentRelation());
        u_options->hide();
        u_relLabel->show();
    }
}

void ReplaceWidget::setValue(TagNode::ParentLink val) {
    u_options->setCurrentIndex(val);
    switch (val) {
    case TagNode::None:
        u_relLabel->setText("");
        setStyleSheet("");
        hide();
        break;
    case TagNode::Replace:
        u_relLabel->setText("Replaces");
        setStyleSheet(".ReplaceWidget { background-color: rgba(72, 116, 67, 125); }");
        show();
        break;
    case TagNode::Require:
        u_relLabel->setText("Requires");
        setStyleSheet(".ReplaceWidget { background-color: rgba(154, 51, 51, 125); }");
        show();
        break;
    }
}

TagNode::ParentLink ReplaceWidget::value() {
    return static_cast<TagNode::ParentLink>(u_options->currentIndex());
}

