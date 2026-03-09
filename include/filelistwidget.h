#ifndef FILELISTWIDGET_H
#define FILELISTWIDGET_H

#include <QListWidgetItem>

namespace Ui {
class FileListWidget;
}

class FileListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FileListWidget(QWidget *parent = nullptr);
    ~FileListWidget();
    void setFiles(QStringList files);

    void clear();
    void addFile(QString filePath);
    QList<std::string> getFiles();

public slots:
    void onAddFile();
    void onRemoveFile();
    void onSelection();
    void onOpenFiles();

signals:
    void filesChanged(QStringList files);

private:
    Ui::FileListWidget *ui;
    QListWidgetItem *selectedFile;
};

#endif // FILELISTWIDGET_H
