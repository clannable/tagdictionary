#include "icontable.h"
#include "globals.h"
#include <QGridLayout>
#include <QDiriterator>

IconTable::IconTable(QWidget *parent) : QWidget(parent) {}

void IconTable::refresh() {
    while (!u_panels.empty()) {
        IconPanel* panel = u_panels.takeFirst();
        delete panel;
    }
    for (const std::string& gIcon : *ICON_LIST) {
        QString icon = QString::fromStdString(gIcon);
        IconPanel *panel = new IconPanel(icon);
        u_panels.append(panel);
        connect(panel, &IconPanel::clicked, this, &IconTable::setCurrentItem);
    }
}

int IconTable::columnCount() const {
    if (layout() == nullptr) return 0;
    return static_cast<QGridLayout*>(layout())->columnCount();
}

IconPanel* IconTable::item(QString icon) {
    int index = m_icons.indexOf(icon);
    if (index == -1) return nullptr;
    return u_panels[index];
}

void IconTable::setCurrentItem(IconPanel *panel) {
    if (u_currentPanel == panel) return;
    if (u_currentPanel != nullptr)
        u_currentPanel->setSelected(false);

    if (panel != nullptr)
        panel->setSelected(true);

    u_currentPanel = panel;
}

void IconTable::updateLayout(int cols) {
    if (cols <= 0) return;
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0,0,0,0);
    grid->setSpacing(0);
    for (int i = 0; i < u_panels.length(); i++)
        grid->addWidget(u_panels[i], i/cols, i%cols);

    for (int c = 0; c < cols; c++)
        grid->setColumnStretch(c, 0);
    grid->setColumnStretch(cols, 1);
    for (int r = 0; r < grid->rowCount(); r++)
        grid->setRowStretch(r, 0);

    if (layout() != nullptr)
        delete layout();
    this->setLayout(grid);
}

