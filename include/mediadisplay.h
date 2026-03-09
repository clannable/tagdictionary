#ifndef MEDIADISPLAY_H
#define MEDIADISPLAY_H

#include <QWidget>
#include "./jsonnode.h"

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
    QList<std::string> getFileList() const;

public slots:
    void clear();
    void setEditMode(bool mode);

    void prevFile();
    void nextFile();
    void openFile();

    void addFile();

signals:
    void fileAdded(QString file);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private:
    Ui::MediaDisplay *ui;
    int currentPage = 0;
    QStringList files;

    void showFile(int index);

    void resizeImage();
};

#endif // MEDIADISPLAY_H
