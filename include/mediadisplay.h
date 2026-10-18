#ifndef MEDIADISPLAY_H
#define MEDIADISPLAY_H

#include <QWidget>
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

    QStringList files() const { return m_files; }

public slots:
    void disableControls();


    void save();
    void openFile();
    void setFile(int index);

    void addFile();

    void refresh();

signals:
    void fileAdded(QString file);

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void dragEnterEvent(QDragEnterEvent *event) override;
    virtual void dropEvent(QDropEvent *event) override;

private:
    Ui::MediaDisplay* ui;
    QWidget* u_displayWidget = nullptr;

    int m_currentPage = 0;
    QStringList m_files;
    bool m_isImage = false;


    void showFile(int index);
    void showImage(QString filePath, bool animated);
    void showVideo(QString filePath);

    void insertFile(QString filePath);

    void resizeImage();


private slots:
    void onEditModeChange(bool editModeEnabled);
};

#endif // MEDIADISPLAY_H
