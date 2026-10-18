#include "iconpanel.h"
#include "ui_iconpanel.h"

IconPanel::IconPanel(QString iconPath, QWidget *parent)
    : QWidget(parent)
    , m_icon(iconPath)
    , ui(new Ui::IconPanel)
{
    ui->setupUi(this);
    ui->iconLabel->setPixmap(QIcon(m_icon).pixmap(QSize(20, 20)));
    ui->frame->setStyleSheet("border-style: none;");

    m_shortName = m_icon.right(m_icon.length() - (m_icon.lastIndexOf("/")+1));
    if (m_shortName.contains("."))
        m_shortName = m_shortName.sliced(0, m_shortName.lastIndexOf("."));

    setToolTip(m_shortName);
}

void IconPanel::mousePressEvent(QMouseEvent *event) { emit clicked(this); }

void IconPanel::showEvent(QShowEvent *event) {
    Q_UNUSED(event);

    ui->frame->setStyleSheet(QString("border-style: ") + (m_selected ? "solid;" : "none;"));
}

void IconPanel::setSelected(bool selected) {
    if (m_selected == selected) return;
    m_selected = selected;
    if (ui == nullptr || ui->frame == nullptr) return;
    ui->frame->setStyleSheet(QString("border-style: ") + (m_selected ? "solid;" : "none;"));
}

IconPanel::~IconPanel()
{
    delete ui;
}
