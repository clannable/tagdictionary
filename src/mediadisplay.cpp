#include "mediadisplay.h"
#include "ui_mediadisplay.h"
#include <nlohmann/json.hpp>
#include <QMimeDatabase>
#include <QFileDialog>
#include <QInputDialog>
#include <iostream>
#include <QDesktopServices>
#include <QFileInfo>

MediaDisplay::MediaDisplay(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MediaDisplay)
{
    ui->setupUi(this);

    connect(ui->prevButton, &QPushButton::clicked, this, &MediaDisplay::prevFile);
    connect(ui->nextButton, &QPushButton::clicked, this, &MediaDisplay::nextFile);
    connect(ui->openButton, &QPushButton::clicked, this, &MediaDisplay::openFile);

    connect(ui->addFileButton, &QToolButton::clicked, this, &MediaDisplay::addFile);
}

MediaDisplay::~MediaDisplay()
{
    delete ui;
}

void MediaDisplay::setFilesFromNode(JsonNode *node) {
    currentPage = 0;
    files.clear();

    if (node != nullptr) {
        const nlohmann::json data = node->getData();

        if (data.contains("files") && !data["files"].empty()) {
            for (const std::string&& file : data["files"])
                files.append(QString::fromStdString(file));
        }

    }
    showFile(0);
}

QStringList MediaDisplay::getFiles() const {
    return files;
}

void MediaDisplay::save() {
    QList<std::string> newFiles = static_cast<FileListWidget*>(currentWidget)->getFiles();
    files.clear();
    for (const std::string& f : newFiles)
        files.append(QString::fromStdString(f));
}

void MediaDisplay::setEditMode(bool mode) {
    ui->addFileButton->setVisible(!mode);
    if (mode) {
        delete currentWidget;
        FileListWidget* fileList = new FileListWidget(ui->viewport);
        fileList->setFiles(this->files);

        currentWidget = fileList;
        ui->viewportLayout->insertWidget(0, currentWidget, 1);

        disableControls();
        // ui->imageLabel->setImage(QPixmap());
    } else {
        showFile(currentPage);
    }
}

void MediaDisplay::showFile(int index) {
    currentPage = index;
    if (currentWidget != nullptr)
        delete currentWidget;

    if (files.isEmpty()) {
        isImage = false;
        QLabel* error = new QLabel();
        error->setText("No files to display");
        error->setAlignment(Qt::AlignCenter);
        ui->fileCounter->setText("");
        currentWidget = error;
        disableControls();

    } else {
        QString filePath = files[currentPage];
        if (!QFileInfo::exists(filePath)) {
            isImage = false;
            QLabel* error = new QLabel();
            error->setText("Failed to load file");
            error->setAlignment(Qt::AlignCenter);

        }
        QMimeDatabase db;
        QString mimeType = db.mimeTypeForFile(filePath).name();
        ui->openButton->setEnabled(true);
        if (mimeType.startsWith("image")) {
            isImage = true;
            PixmapLabel* image = new PixmapLabel();
            image->setImage(filePath, mimeType.endsWith("gif"));
            currentWidget = image;
        } else if (mimeType.startsWith("video")) {
            isImage = false;
            VideoPlayer* video = new VideoPlayer();
            video->setVideo(filePath);
            video->play();
            currentWidget = video;
        }
        ui->fileCounter->setText(QString::number(currentPage+1) + " / " + QString::number(files.length()));
        ui->prevButton->setEnabled(currentPage > 0);
        ui->nextButton->setEnabled(currentPage < files.length()-1);
    }
    if (currentWidget != nullptr)
        ui->viewportLayout->insertWidget(0, currentWidget, 1);
    if (isImage)
        resizeImage();
}

void MediaDisplay::prevFile() {
    showFile(currentPage-1);
}

void MediaDisplay::nextFile() {
    showFile(currentPage+1);
}

void MediaDisplay::openFile() {
    QDesktopServices::openUrl("file:///" + files[currentPage]);
}

void MediaDisplay::addFile() {
    bool ok{};
    QString file = QInputDialog::getText(this, "Enter file path", "File path:", QLineEdit::Normal, "", &ok);

    if (ok && !file.isEmpty()) {
        file.replace("\\", "/");
        files.append(file);
        showFile(files.length()-1);
        emit fileAdded(file);
    }

}
void MediaDisplay::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);

    // force QLabel to follow fixed dimensions
    // based on the loaded QPixmap aspect ratio
    if (isImage)
        resizeImage();
}

void MediaDisplay::resizeImage() {
    int w = ui->viewport->width();
    int h = static_cast<PixmapLabel*>(currentWidget)->heightForWidth(w);
    static_cast<PixmapLabel*>(currentWidget)->setFixedHeight(std::min(h, ui->viewport->height()));

}

void MediaDisplay::disableControls() {
    ui->prevButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->openButton->setEnabled(false);
    /*ui->videoPlayer->clearVideo();
    ui->videoContainer->hide();
    ui->imageLabel->setImage(QPixmap());
    ui->imageContainer->show();
    ui->prevButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->openButton->setEnabled(false);*/

}
