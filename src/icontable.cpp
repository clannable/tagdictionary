#include "icontable.h"
#include "globals.h"
#include <QGridLayout>
#include <QDiriterator>

IconTable::IconTable(QWidget *parent)
    : QWidget(parent), current(nullptr), panels() {}

void IconTable::refresh() {
    while (!panels.empty()) {
        IconPanel* panel = panels.takeFirst();
        delete panel;
    }
    for (const std::string& gIcon : *ICON_LIST) {
        QString icon = QString::fromStdString(gIcon);
        IconPanel *panel = new IconPanel(icon);
        panels.append(panel);
        connect(panel, &IconPanel::selected, this, &IconTable::setCurrentItem);
    }
}

int IconTable::columnCount() const {
    if (layout() == nullptr) return 0;
    return static_cast<QGridLayout*>(layout())->columnCount();
}

IconPanel* IconTable::currentItem() const {
    return current;
}

IconPanel* IconTable::item(QString icon) {
    int index = icons.indexOf(icon);
    if (index == -1) return nullptr;
    return panels[index];
}

void IconTable::setCurrentItem(IconPanel *panel=nullptr) {
    if (current != nullptr)
        current->setSelected(false);

    if (panel != nullptr)
        panel->setSelected(true);

    current = panel;
}

void IconTable::updateLayout(int cols) {
    if (cols <= 0) return;
    QGridLayout *grid = new QGridLayout();
    grid->setContentsMargins(0,0,0,0);
    grid->setSpacing(0);
    for (int i = 0; i < panels.length(); i++) {
        IconPanel *panel = panels[i];
        grid->addWidget(panel, i/cols, i%cols);
    }
    for (int c = 0; c < cols; c++)
        grid->setColumnStretch(c, 0);
    grid->setColumnStretch(cols, 1);
    for (int r = 0; r < grid->rowCount(); r++)
        grid->setRowStretch(r, 0);

    if (this->layout() != nullptr)
        delete this->layout();
    this->setLayout(grid);
}

