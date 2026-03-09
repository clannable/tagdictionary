#include "icontable.h"
#include <QGridLayout>
#include <QDiriterator>
#include <iostream>

IconTable::IconTable(QWidget *parent)
    : QWidget(parent), current(nullptr), panels()
{
    QDirIterator it(":/icons/", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        if (path.lastIndexOf(".") != -1)
            path = path.slice(0, path.lastIndexOf("."));
        else continue;
        icons.append(path);
    }
    icons.sort();

    for (int i = 0; i < icons.length(); i++) {
        QString icon = icons[i];
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

