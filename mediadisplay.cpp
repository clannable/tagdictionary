#include "mediadisplay.h"
#include "ui_mediadisplay.h"
#include <nlohmann/json.hpp>
#include <QMimeDatabase>
#include <QFileDialog>
#include <QInputDialog>
#include <iostream>
#include <QDesktopServices>

MediaDisplay::MediaDisplay(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MediaDisplay)
{
    ui->setupUi(this);

    ui->fileListEditor->hide();
    ui->videoContainer->hide();

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
    ui->fileListEditor->clear();
    ui->fileListEditor->hide();

    if (node == nullptr) return;
    const nlohmann::json data = node->getData();

    if (data.contains("files") && !data["files"].empty()) {
        for (const std::string&& file : data["files"])
            files.append(QString::fromStdString(file));
    } else {
        clear();
    }

    ui->fileListEditor->setFiles(files);

    showFile(0);
}

QList<std::string> MediaDisplay::getFileList() const {
    return ui->fileListEditor->getFiles();
}

void MediaDisplay::setEditMode(bool mode) {

    if (mode) {
        ui->videoPlayer->stop();
        ui->videoContainer->hide();
        ui->imageContainer->hide();
        ui->fileListEditor->show();
        ui->prevButton->setEnabled(false);
        ui->openButton->setEnabled(false);
        ui->nextButton->setEnabled(false);
        ui->fileCounter->setText("");
        // ui->imageLabel->setImage(QPixmap());
    } else {
        ui->fileListEditor->hide();
        showFile(currentPage);
    }
}

void MediaDisplay::showFile(int index) {
    currentPage = index;
    if (files.isEmpty()) {
        ui->videoContainer->hide();
        ui->imageLabel->setImage(QPixmap());
        ui->imageContainer->show();
        return;
    }
    QMimeDatabase db;
    QString filePath = files[currentPage];
    QString mimeType = db.mimeTypeForFile(filePath).name();
    ui->openButton->setEnabled(true);
    if (mimeType.startsWith("image")) {
        if (mimeType.endsWith("gif"))
            ui->imageLabel->setMovieFile(filePath);
        else
            ui->imageLabel->setImage(QPixmap(filePath));

        ui->videoPlayer->clearVideo();
        ui->videoContainer->hide();
        resizeImage();
        ui->imageContainer->show();

    } else if (mimeType.startsWith("video")) {
        ui->imageContainer->hide();
        ui->imageLabel->setImage(QPixmap());
        ui->videoContainer->show();
        ui->videoPlayer->stop();

        ui->videoPlayer->setVideo(filePath);
        ui->videoPlayer->play();
    }
    ui->fileCounter->setText(QString::number(currentPage+1) + " / " + QString::number(files.length()));
    ui->prevButton->setEnabled(currentPage > 0);
    ui->nextButton->setEnabled(currentPage < files.length()-1);
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
        // QString file = QFileDialog::getOpenFileName(this,
        //     "Select file to add",
        //     "/home",
        //     "Media files (*.png *.jpg *.gif *.mp4 .mov)");
        file.replace("\\", "/");
        files.append(file);
        showFile(files.length()-1);
        ui->fileListEditor->addFile(file);
        emit fileAdded(file);
    }

}
void MediaDisplay::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event);

    // force QLabel to follow fixed dimensions
    // based on the loaded QPixmap aspect ratio
    if (ui->imageLabel->hasImage())
        resizeImage();
}

void MediaDisplay::resizeImage() {
    int w = ui->imageContainer->width();
    int h = ui->imageLabel->heightForWidth(w);
    ui->imageLabel->setFixedHeight(std::min(h, ui->imageContainer->height()));

}

void MediaDisplay::clear() {
    ui->videoPlayer->clearVideo();
    ui->videoContainer->hide();
    ui->imageLabel->setImage(QPixmap());
    ui->imageContainer->show();
    ui->prevButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->openButton->setEnabled(false);
    ui->fileCounter->setText("");
}
