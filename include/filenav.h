#ifndef FILENAV_H
#define FILENAV_H

#include <QWidget>
#include <QToolButton>

class FileNav : public QWidget
{
    Q_OBJECT
public:
    explicit FileNav(QWidget *parent = nullptr);

    void addFile(QString filePath);
    void setFiles(QStringList files);
    void clear();

public slots:
    void setSelected(int index);
signals:
    void fileSelected(int index);

private:
    int currentFile = 0;
    int numFiles = 0;
    QToolButton* prevButton;
    QToolButton* nextButton;

    QWidget* numContainer = nullptr;
};

#endif // FILENAV_H
