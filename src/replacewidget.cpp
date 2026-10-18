#include <QHBoxLayout>
#include "appstate.h"
#include "globals.h"

#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QFileInfo>
#include <QCheckBox>
#include "replacewidget.h"
ReplaceWidget::ReplaceWidget(QWidget *parent)
    : QWidget{parent}
{
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(2);
    setLayout(layout);

    QCheckBox* checkbox = new QCheckBox();
    checkbox->setChecked(false);
    checkbox->setText("");
    layout->insertWidget(0, checkbox, 1);
    checkbox->hide();
    layout->insertWidget(1, new QLabel("Replaces"), 0);
    layout->insertWidget(2, new QLabel(), 0);
    layout->insertWidget(3, new QLabel(), 1);
    layout->insertWidget(4, new QLabel("?"), 0);

    connect(APP_STATE, &AppState::editModeChanged, this, &ReplaceWidget::onEditModeChange);
    layout->itemAt(3)->widget()->setStyleSheet("font: 350 11pt 'Segoe UI'");
}


void ReplaceWidget::useParent(const TagNode *tag) {

    QString ic = QString::fromStdString(tag->icon());
    QIcon icon = QIcon(ic);
    static_cast<QLabel*>(layout()->itemAt(2)->widget())->setPixmap(icon.pixmap(QSize(10, 10)));
    static_cast<QLabel*>(layout()->itemAt(3)->widget())->setText(QString::fromStdString(tag->key()));
}

void ReplaceWidget::onEditModeChange(bool editModeEnabled) {
    if (editModeEnabled)
        layout()->itemAt(0)->widget()->show();
    else
        layout()->itemAt(0)->widget()->hide();
}

void ReplaceWidget::setReplaceChecked(bool replace) {
    static_cast<QCheckBox*>(layout()->itemAt(0)->widget())->setChecked(replace);
}

bool ReplaceWidget::isReplaceChecked() {
    return static_cast<QCheckBox*>(layout()->itemAt(0)->widget())->isChecked();
}


