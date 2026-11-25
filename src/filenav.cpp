#include "filenav.h"
#include <QHBoxLayout>
#include <regex>
#include <QPushButton>

FileNav::FileNav(QWidget *parent)
    : QWidget{parent}
{
    QHBoxLayout* row = new QHBoxLayout();
    row->setContentsMargins(0,0,0,0);
    row->setSpacing(10);
    setLayout(row);
    setMinimumHeight(40);
    numContainer = new QWidget();
    numContainer->setObjectName("numbers");
    numContainer->setLayout(new QHBoxLayout());
    numContainer->layout()->setContentsMargins(0,0,0,0);
    numContainer->layout()->setSpacing(10);

    prevButton = new QToolButton();
    prevButton->setObjectName("previous");
    prevButton->setEnabled(false);
    prevButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoPrevious));
    prevButton->setCursor(Qt::PointingHandCursor);

    nextButton = new QToolButton();
    nextButton->setObjectName("next");
    nextButton->setEnabled(false);
    nextButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoNext));
    nextButton->setCursor(Qt::PointingHandCursor);

    row->addWidget(prevButton, 0);
    row->addWidget(numContainer, 0);
    row->addWidget(nextButton, 0);

    connect(prevButton, &QToolButton::clicked, this, [this]() { emit fileSelected(currentFile-1); });
    connect(nextButton, &QToolButton::clicked, this, [this]() { emit fileSelected(currentFile+1); });
    connect(this, &FileNav::fileSelected, this, &FileNav::setSelected);
}

void FileNav::setFiles(QStringList files) {
    clear();
    for (const QString& file : files)
        addFile(file);
    setSelected(0);

    prevButton->setEnabled(currentFile > 0);
    nextButton->setEnabled(currentFile < numFiles-1);
}

void FileNav::setSelected(int index) {
    if (currentFile == index) return;
    if (currentFile < numFiles) {
        QToolButton* button = dynamic_cast<QToolButton*>(numContainer->layout()->itemAt(currentFile)->widget());
        if (button) button->setEnabled(true);
    }

    currentFile = index;

    if (numFiles != 0) {
        QToolButton* clicked = dynamic_cast<QToolButton*>(numContainer->layout()->itemAt(index)->widget());
        if (clicked) clicked->setEnabled(false);
    }
    prevButton->setEnabled(index > 0);
    nextButton->setEnabled(index < numFiles-1);
}

void FileNav::clear() {
    numFiles = 0;
    if (!numContainer) return;
    QLayoutItem* child;
    while((child = numContainer->layout()->takeAt(0))) {
        if (QWidget *w = child->widget())
            w->deleteLater();
        delete child;
    }

    prevButton->setEnabled(false);
    nextButton->setEnabled(false);

}

int FileNav::addFile(QString filePath) {
    int nextIndex = numFiles;
    if (nextIndex > 0) nextButton->setEnabled(true);
    std::regex videoRegex("\\.(mov|mp4|wmv)$", std::regex_constants::icase);
    std::regex imageRegex("\\.(jpeg|png|gif|jpg|bmp|jfif|webp)$", std::regex_constants::icase);

    QString icon;
    if (std::regex_search(filePath.toStdString(), imageRegex))
        icon = ":/icons/linear/image.png";
    else if (std::regex_search(filePath.toStdString(), videoRegex))
        icon = ":/icons/linear/video.png";

    QToolButton* button = new QToolButton();
    button->setText(QString::number(nextIndex+1));
    button->setIcon(QIcon(icon));
    button->setCursor(Qt::PointingHandCursor);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setToolTip(filePath);
    button->setEnabled(currentFile != nextIndex);
    numContainer->layout()->addWidget(button);
    connect(button, &QToolButton::clicked, this, [this, button]() {
        emit fileSelected(numContainer->layout()->indexOf(button));
    });
    numFiles++;
    return nextIndex;
}
