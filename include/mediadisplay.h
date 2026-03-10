#ifndef MEDIADISPLAY_H
#define MEDIADISPLAY_H

#include <QWidget>
#include "jsonnode.h"
#include <QDropEvent>
#include <QVBoxLayout>

namespace Ui {
class MediaDisplay;
}

class MediaDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit MediaDisplay(QWidget *parent = nullptr);
    ~MediaDisplay();

    void setFilesFromNode(JsonNode* node);
    QStringList getFiles() const;

public slots:
    void disableControls();
    void setEditMode(bool mode);

    void save();
    void prevFile();
    void nextFile();
    void openFile();

    void addFile();

signals:
    void fileAdded(QString file);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    Ui::MediaDisplay *ui;
    int currentPage = 0;
    QStringList files;
    bool editModeEnabled = false;

    JsonNode* node = nullptr;
    QWidget* currentWidget = nullptr;

    void showFile(int index);
    void showImage(QString filePath, bool animated);
    void showVideo(QString filePath);

    void insertFile(QString filePath);

    void resizeImage();
    bool isImage = false;
};

#endif // MEDIADISPLAY_H
