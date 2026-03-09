#include "iconpanel.h"
#include "ui_iconpanel.h"
#include <iostream>

IconPanel::IconPanel(QString iconPath, QWidget *parent)
    : QWidget(parent)
    , icon(iconPath)
    , isSelected(false)
    , ui(new Ui::IconPanel)
{
    ui->setupUi(this);

    ui->iconLabel->setPixmap(QIcon(icon).pixmap(QSize(20, 20)));
    QString shortName = icon.right(icon.length() - (icon.lastIndexOf("/")+1));
    this->setToolTip(shortName);

    // baseStyleSheet = ui->frame->styleSheet();
    ui->frame->setStyleSheet("border-style: none;");
    // ui->label->setText(icon);
}

QString IconPanel::getIcon() {
    return icon;
}

void IconPanel::mousePressEvent(QMouseEvent *event) {
    emit selected(this);
}

void IconPanel::showEvent(QShowEvent *event) {
    Q_UNUSED(event);

    ui->frame->setStyleSheet(QString("border-style: ") + (isSelected ? "solid;" : "none;"));
}

void IconPanel::setSelected(bool selected) {
    isSelected = selected;
    if (ui == nullptr || ui->frame == nullptr) return;
    ui->frame->setStyleSheet(QString("border-style: ") + (isSelected ? "solid;" : "none;"));
}

IconPanel::~IconPanel()
{
    delete ui;
}
