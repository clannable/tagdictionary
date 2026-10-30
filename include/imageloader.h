#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include <QThread>
#include <QImage>
#include <QObject>
#include <QMovie>

class ImageLoader : public QObject
{
    Q_OBJECT

public:
    explicit ImageLoader(QObject *parent = nullptr);
    ~ImageLoader();
    void requestFile(QString filePath);

public slots:
    void loadImage(QString filePath);

signals:
    void imageReady(QImage image);

private:
    QThread t;
};

#endif // IMAGELOADER_H
