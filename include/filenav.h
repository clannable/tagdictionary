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
    void clear();

signals:
    void fileSelected(int index);

private:
    int currentFile = -1;

    QToolButton* prevButton;
    QToolButton* nextButton;

private slots:
    void onSelectionChange(int index);
};

#endif // FILENAV_H
