#include "filelistwidget.h"
#include "ui_filelistwidget.h"
#include <QFileDialog>

FileListWidget::FileListWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileListWidget)
{
    ui->setupUi(this);

    connect(ui->addButton, &QToolButton::clicked, this, &FileListWidget::onAddFile);
    connect(ui->removeButton, &QToolButton::clicked, this, &FileListWidget::onRemoveFile);
    connect(ui->openButton, &QToolButton::clicked, this, &FileListWidget::onOpenFiles);
    connect(ui->fileList, &QListWidget::itemSelectionChanged, this, &FileListWidget::onSelection);
}

FileListWidget::~FileListWidget()
{
    delete ui;
}

QList<std::string> FileListWidget::getFiles() {
    QList<std::string> ret;
    for (int i = 0; i < ui->fileList->count(); i++) {
        QListWidgetItem *item = ui->fileList->item(i);
        QString str = item->text();
        str.replace("\\", "/");
        if (!str.trimmed().isEmpty())
            ret.append(str.trimmed().toStdString());
    }
    return ret;
}

void FileListWidget::setFiles(QStringList files) {
    clear();
    for (const QString& f : files) {
        QListWidgetItem *item = new QListWidgetItem(f);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->fileList->addItem(item);
    }
}

void FileListWidget::clear() {
    ui->fileList->clear();
}

void FileListWidget::onSelection() {
    selectedFile = ui->fileList->currentItem();
    ui->removeButton->setEnabled(selectedFile != nullptr);
}

void FileListWidget::addFile(QString filePath) {
    ui->fileList->addItem(filePath);
}

void FileListWidget::onAddFile() {
    QListWidgetItem *item = new QListWidgetItem();
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui->fileList->addItem(item);
    ui->fileList->editItem(item);
}

void FileListWidget::onOpenFiles() {
    QStringList selected = QFileDialog::getOpenFileNames(
        this,
        "Select files to add",
        "/home",
        "Media files (*.png *.jpg *.gif *.mp4 .mov)");

    for (const QString file : selected) {
        QString f = file;
        QListWidgetItem *item = new QListWidgetItem(f.replace("\\", "/"));
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        ui->fileList->addItem(item);
    }
}

void FileListWidget::onRemoveFile() {
    QListWidgetItem *item = ui->fileList->currentItem();
    if (item != nullptr) {
        delete item;
        ui->fileList->clearSelection();
        ui->removeButton->setEnabled(false);
    }
}
