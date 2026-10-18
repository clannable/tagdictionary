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

    connect(ui->prevButton, &QPushButton::clicked, this, [this] { showFile(--m_currentPage); });
    connect(ui->nextButton, &QPushButton::clicked, this, [this] { showFile(++m_currentPage); });
    connect(ui->openButton, &QPushButton::clicked, this, &MediaDisplay::openFile);

    connect(ui->addFileButton, &QToolButton::clicked, this, &MediaDisplay::addFile);
    connect(APP_STATE, &AppState::selectedTagChanged, this, &MediaDisplay::refresh);
    connect(APP_STATE, &AppState::editModeChanged, this, &MediaDisplay::onEditModeChange);
}

MediaDisplay::~MediaDisplay()
{
    delete ui;
}

void MediaDisplay::refresh() {
    m_currentPage = 0;
    m_files.clear();
    ui->addFileButton->setEnabled(APP_STATE->selectedTag() != nullptr && !APP_STATE->selectedTag()->isRoot());
    if (APP_STATE->selectedTag() != nullptr) {
        for (const std::string &file : APP_STATE->selectedTag()->files())
            m_files.append(QString::fromStdString(file));
    }
    showFile(0);
}

void MediaDisplay::save() {
    m_files = static_cast<FileListWidget*>(u_displayWidget)->values();
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

        disableControls();
    } else {
        showFile(m_currentPage);
    }
}

void MediaDisplay::showFile(int index) {
    if (u_displayWidget != nullptr)
        delete u_displayWidget;

    if (m_files.isEmpty()) {
        m_isImage = false;
        QLabel* error = new QLabel();
        error->setText("No files to display");
        error->setAlignment(Qt::AlignCenter);
        ui->fileCounter->setText("");
        u_displayWidget = error;
        disableControls();

    } else {
        try {
            QString filePath = m_files[m_currentPage];
            QMimeDatabase db;
            QString mimeType = db.mimeTypeForFile(filePath).name();
            ui->openButton->setEnabled(true);
            if (mimeType.startsWith("image")) {
                m_isImage = true;
                PixmapLabel* image = new PixmapLabel();
                image->setImage(filePath, mimeType.endsWith("gif"));
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

        ui->fileCounter->setText(QString::number(m_currentPage+1) + " / " + QString::number(m_files.length()));
        ui->prevButton->setEnabled(m_currentPage > 0);
        ui->nextButton->setEnabled(m_currentPage < m_files.length()-1);
    }
    if (u_displayWidget != nullptr)
        ui->viewportLayout->insertWidget(0, u_displayWidget, 1);
    if (m_isImage)
        resizeImage();
}

void MediaDisplay::setFile(int index) {
    showFile(index);
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

    for (const QString file : selected) {
        QString f = file;
        insertFile(f);
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
    else
        showFile(m_files.length()-1);

    APP_STATE->selectedTag()->addFile(filePath.toStdString());
    APP_STATE->signalSelectedTagUpdated();
}
void MediaDisplay::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);

    // force QLabel to follow fixed dimensions
    // based on the loaded QPixmap aspect ratio
    if (m_isImage)
        resizeImage();
}

void MediaDisplay::dragEnterEvent(QDragEnterEvent *event) {
    QMimeDatabase db;
    const QMimeData* data = event->mimeData();
    if (APP_STATE->selectedTag() == nullptr || APP_STATE->selectedTag()->isRoot() || event->source() != nullptr) {
        event->ignore();
    } else if (data->hasUrls()) {
        QString filePath = data->urls()[0].toString(QUrl::DecodeReserved | QUrl::PrettyDecoded);
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
    QUrl url = event->mimeData()->urls().first();
    QString filePath = event->mimeData()->urls().first().toString(QUrl::DecodeReserved | QUrl::PrettyDecoded);
    if (filePath.startsWith("file:///"))
        filePath = filePath.slice(8);
    if (!filePath.isEmpty())
        insertFile(filePath);

}
void MediaDisplay::resizeImage() {
    int w = ui->viewport->width();
    int h = static_cast<PixmapLabel*>(u_displayWidget)->heightForWidth(w);
    static_cast<PixmapLabel*>(u_displayWidget)->setFixedHeight(std::min(h, ui->viewport->height()));

}

void MediaDisplay::disableControls() {
    ui->prevButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->openButton->setEnabled(false);
}
