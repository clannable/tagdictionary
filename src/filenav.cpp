#include "filenav.h"
#include <QHBoxLayout>

FileNav::FileNav(QWidget *parent)
    : QWidget{parent}
{
    QHBoxLayout* row = new QHBoxLayout();

    prevButton = new QToolButton();
    prevButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious));
    connect(prevButton, &QToolButton::clicked, this, [this]() { emit fileSelected(--currentFile); });
    row->addWidget(prevButton, 0);

    nextButton = new QToolButton();
    nextButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoNext));
    connect(nextButton, &QToolButton::clicked, this, [this]() { emit fileSelected(++currentFile); });
    row->addWidget(nextButton, 0);

    connect(this, &FileNav::fileSelected, this, &FileNav::onSelectionChange);
}

void FileNav::onSelectionChange(int index) {
    int count = layout()->count() - 2;
    prevButton->setEnabled(index > 0);
    nextButton->setEnabled(index >= 0 && index < count-1);
}
