#include <QHBoxLayout>
#include "appstate.h"
#include "globals.h"

#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QFileInfo>
#include <QCheckBox>
#include <QMouseEvent>

#include "replacewidget.h"
ReplaceWidget::ReplaceWidget(QWidget *parent)
    : QFrame{parent}
{
    setFrameShadow(QFrame::Plain);
    setFrameStyle(QFrame::StyledPanel);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(3);
    setLayout(layout);
    QCheckBox* checkbox = new QCheckBox();
    checkbox->setChecked(false);
    checkbox->setText("");
    layout->insertWidget(0, checkbox, 0);
    checkbox->hide();
    layout->insertWidget(1, new QLabel("Replaces"), 0);
    layout->insertWidget(2, new QLabel(), 0);
    layout->insertWidget(3, new QLabel(), 0);
    layout->insertWidget(4, new QLabel("?"), 0);
    layout->itemAt(0)->setAlignment(Qt::AlignVCenter);
    layout->itemAt(1)->setAlignment(Qt::AlignVCenter);
    layout->itemAt(2)->setAlignment(Qt::AlignVCenter);
    layout->itemAt(3)->setAlignment(Qt::AlignVCenter);
    layout->itemAt(4)->setAlignment(Qt::AlignVCenter);
    connect(APP_STATE, &AppState::editModeChanged, this, &ReplaceWidget::onEditModeChange);
    connect(APP_STATE, &AppState::selectedTagChanged, this, &ReplaceWidget::setTag);
    layout->itemAt(4)->widget()->hide();
    hide();
}


void ReplaceWidget::setTag(const TagNode *tag) {
    if (tag != nullptr && tag->id() > 0 && tag->parent() != nullptr && tag->parent()->id() > 0) {
        QString ic = QString::fromStdString(tag->parent()->icon());
        QIcon icon = QIcon(ic);
        static_cast<QCheckBox*>(layout()->itemAt(0)->widget())->setChecked(tag->replacesParent());
        static_cast<QLabel*>(layout()->itemAt(2)->widget())->setPixmap(icon.pixmap(QSize(15, 15)));
        static_cast<QLabel*>(layout()->itemAt(3)->widget())->setText(QString::fromStdString(tag->parent()->key()));
        if (tag->replacesParent())
            show();
        else
            hide();
    } else {
        hide();
    }
}

void ReplaceWidget::onEditModeChange(bool editModeEnabled) {
    if (editModeEnabled) {
        if (APP_STATE->selectedTag()->parent() != nullptr && APP_STATE->selectedTag()->parent()->id() > 0)
            show();
        else hide();
        layout()->itemAt(0)->widget()->show();
        layout()->itemAt(4)->widget()->show();
        setCursor(Qt::PointingHandCursor);
    }
    else {
        if (isReplaceChecked())
            show();
        else hide();
        layout()->itemAt(0)->widget()->hide();
        layout()->itemAt(4)->widget()->hide();
        setCursor(Qt::ArrowCursor);
    }
}

void ReplaceWidget::setReplaceChecked(bool replace) {
    static_cast<QCheckBox*>(layout()->itemAt(0)->widget())->setChecked(replace);
}

bool ReplaceWidget::isReplaceChecked() {
    return static_cast<QCheckBox*>(layout()->itemAt(0)->widget())->isChecked();
}

void ReplaceWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton && APP_STATE->editModeEnabled() &&
        APP_STATE->selectedTag()->parent() != nullptr && APP_STATE->selectedTag()->parent()->id() > 0)
        static_cast<QCheckBox*>(layout()->itemAt(0)->widget())->toggle();
}

