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
    connect(prevButton, &QToolButton::clicked, this, [this]() { emit fileSelected(currentFile-1); });
    row->addWidget(prevButton, 0);

    row->addWidget(numContainer, 0);
    nextButton = new QToolButton();
    nextButton->setObjectName("next");
    nextButton->setEnabled(false);
    nextButton->setIcon(QIcon::fromTheme(QIcon::ThemeIcon::GoNext));
    nextButton->setCursor(Qt::PointingHandCursor);
    connect(nextButton, &QToolButton::clicked, this, [this]() { emit fileSelected(currentFile+1); });
    row->addWidget(nextButton, 0);

    setLayout(row);

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
        if (button != nullptr)
            button->setEnabled(true);
    }

    currentFile = index;

    if (numFiles != 0) {
        QToolButton* clicked = dynamic_cast<QToolButton*>(numContainer->layout()->itemAt(index)->widget());
        if (clicked != nullptr)
            clicked->setEnabled(false);
    }
    prevButton->setEnabled(index > 0);
    nextButton->setEnabled(index < numFiles-1);
}

void FileNav::clear() {
    numFiles = 0;
    if (numContainer == nullptr) return;
    QLayoutItem* child;
    while((child = numContainer->layout()->takeAt(0)) != nullptr) {
        if (QWidget *w = child->widget())
            w->deleteLater();
        delete child;
    }

    prevButton->setEnabled(false);
    nextButton->setEnabled(false);

}

void FileNav::addFile(QString filePath) {
    int nextIndex = numFiles;
    if (nextIndex > 0)
        nextButton->setEnabled(true);
    std::regex videoRegex("\\.(mov|mp4|wmv)$", std::regex_constants::icase);
    std::regex imageRegex("\\.(jpeg|png|gif|jpg|bmp|jfif|webp)$", std::regex_constants::icase);

    QString icon;
    if (std::regex_search(filePath.toStdString(), imageRegex))
        icon = ":/icons/linear/image.png";
    else if (std::regex_search(filePath.toStdString(), videoRegex))
        icon = ":/icons/linear/video.png";

    // FileNavButton* button = new FileNavButton(nextIndex+1, icon);
    QToolButton* button = new QToolButton();
    button->setText(QString::number(nextIndex+1));
    button->setIcon(QIcon(icon));
    button->setCursor(Qt::PointingHandCursor);
    // button->setLayoutDirection(Qt::RightToLeft);
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    button->setToolTip(filePath);
    button->setEnabled(currentFile != nextIndex);
    numContainer->layout()->addWidget(button);
    connect(button, &QToolButton::clicked, this, [this, button]() { emit fileSelected(numContainer->layout()->indexOf(button)); });
    numFiles++;
}
