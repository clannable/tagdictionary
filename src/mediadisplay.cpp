#include "mediadisplay.h"
#include "ui_mediadisplay.h"
#include <nlohmann/json.hpp>
#include <QMimeDatabase>
#include <QFileDialog>
#include <QInputDialog>
#include "filelistwidget.h"
#include "videoplayer.h"
#include "pixmaplabel.h"
#include <QMimeData>
#include <QDesktopServices>
#include <QFileInfo>
#include "globals.h"

MediaDisplay::MediaDisplay(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MediaDisplay)
{
    ui->setupUi(this);

    connect(ui->openButton, &QToolButton::clicked, this, &MediaDisplay::openFile);
    connect(ui->addFileButton, &QToolButton::clicked, this, &MediaDisplay::addFile);
    connect(ui->fileNav, &FileNav::fileSelected, this, &MediaDisplay::setFile);
    connect(APP_STATE, &AppState::selectedTagChanged, this, &MediaDisplay::refresh);
    connect(APP_STATE, &AppState::editModeChanged, this, &MediaDisplay::onEditModeChange);
}

MediaDisplay::~MediaDisplay() {
    delete ui;
}

void MediaDisplay::refresh() {
    m_currentPage = 0;
    m_files.clear();
    ui->addFileButton->setEnabled(APP_STATE->selectedTag() != nullptr && APP_STATE->selectedTag()->id() > 0);
    if (APP_STATE->selectedTag() != nullptr) {
        for (const std::string &file : APP_STATE->selectedTag()->files())
            m_files.append(QString::fromStdString(file));
    }
    if (APP_STATE->selectedTag() == nullptr || APP_STATE->selectedTag()->id() <= 0)
        ui->fileNav->hide();
    else
        ui->fileNav->show();
    ui->fileNav->setFiles(m_files);
    showFile(0);
}

void MediaDisplay::save() {
    m_files = static_cast<FileListWidget*>(u_displayWidget)->values();
    ui->fileNav->setFiles(m_files);
    m_currentPage = 0;
}

void MediaDisplay::onEditModeChange(bool editModeEnabled) {
    editModeEnabled = editModeEnabled;
    ui->addFileButton->setVisible(!editModeEnabled);
    if (editModeEnabled) {
        delete u_displayWidget;
        FileListWidget* fileList = new FileListWidget(ui->viewport);
        fileList->setFiles(APP_STATE->selectedTag()->files());

        u_displayWidget = fileList;
        ui->viewportLayout->insertWidget(0, u_displayWidget, 1);
        ui->fileNav->hide();
        ui->openButton->setEnabled(false);
    } else {
        ui->fileNav->show();
        ui->fileNav->setSelected(m_currentPage);
        showFile(m_currentPage);
    }
}

void MediaDisplay::showFile(int index) {
    if (u_displayWidget)
        delete u_displayWidget;

    if (m_files.isEmpty()) {
        m_isImage = false;
        ui->openButton->setEnabled(false);

        QLabel* error = new QLabel();
        error->setText("No files to display");
        error->setAlignment(Qt::AlignCenter);
        u_displayWidget = error;
    } else {
        m_currentPage = index;
        try {
            QString filePath = m_files[m_currentPage];
            QMimeDatabase db;
            QString mimeType = db.mimeTypeForFile(filePath).name();
            ui->openButton->setEnabled(true);
            if (mimeType.startsWith("image")) {
                m_isImage = true;
                PixmapLabel* image = new PixmapLabel();
                image->setFile(filePath);
                u_displayWidget = image;
            } else if (mimeType.startsWith("video")) {
                m_isImage = false;
                VideoPlayer* video = new VideoPlayer();
                video->setVideo(filePath);
                video->play();
                u_displayWidget = video;
            }
        } catch (...) {
            m_isImage = false;
            QLabel* error = new QLabel();
            error->setText("Failed to load file");
            error->setAlignment(Qt::AlignCenter);
            u_displayWidget = error;
        }
    }
    if (u_displayWidget)
        ui->viewportLayout->insertWidget(0, u_displayWidget, 1);
    // if (m_isImage)
        // resizeImage();
}

void MediaDisplay::setFile(int index) {
    showFile(index);
    ui->fileNav->setSelected(index);
}

void MediaDisplay::openFile() {
    QDesktopServices::openUrl("file:///" + m_files[m_currentPage]);
}

void MediaDisplay::addFile() {
    QStringList selected = QFileDialog::getOpenFileNames(
        this,
        "Select files to add",
        QString::fromStdString(LAST_IMAGE_FOLDER_PATH),
        "Media files (*.png *.jpg *.gif *.mp4 .mov)");

    if (!selected.empty())
        LAST_IMAGE_FOLDER_PATH = QFileInfo(selected.last()).absoluteDir().path().toStdString();

    for (const QString &file : selected) {
        QString f = file;
        insertFile(f);
        ui->fileNav->setSelected(m_currentPage);
        QListWidgetItem *item = new QListWidgetItem(f.replace("\\", "/"));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    }
}

void MediaDisplay::insertFile(QString filePath) {
    filePath.replace("\\", "/");
    if (m_files.contains(filePath)) return;

    m_files.append(filePath);
    if (APP_STATE->editModeEnabled())
        static_cast<FileListWidget*>(u_displayWidget)->addFile(filePath);
    else {
        showFile(m_files.length()-1);
        ui->fileNav->setSelected(ui->fileNav->addFile(filePath));
    }

    APP_STATE->selectedTag()->addFile(filePath.toStdString());
    APP_STATE->signalSelectedTagUpdated();
}

void MediaDisplay::dragEnterEvent(QDragEnterEvent *event) {
    QMimeDatabase db;
    const QMimeData* data = event->mimeData();
    // Ignore drag events while selected tag is root, and ignore all drags that do not come from outside the application
    if (!APP_STATE->selectedTag() || APP_STATE->selectedTag()->isRoot() || event->source()) {
        event->ignore();
    } else if (data->hasUrls()) {
        // Only accept file drags that are images or videos
        QString filePath = data->urls().front().toString(QUrl::DecodeReserved | QUrl::PrettyDecoded);
        QString mimeType = db.mimeTypeForFile(filePath).name();
        if (mimeType.startsWith("image") || mimeType.startsWith("video")) {
            event->setDropAction(Qt::CopyAction);
            event->accept();
        } else
            event->ignore();
    } else {
        event->ignore();
    }
}

void MediaDisplay::dropEvent(QDropEvent *event) {
    QUrl url = event->mimeData()->urls()[0];
    QString filePath = url.toString(QUrl::DecodeReserved | QUrl::PrettyDecoded);
    if (filePath.startsWith("file:///"))
        filePath = filePath.slice(8);
    if (!filePath.isEmpty()) {
        insertFile(filePath);
    }
}
