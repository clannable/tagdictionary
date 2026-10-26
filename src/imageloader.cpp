#include "imageloader.h"
#include <QMetaObject>

ImageLoader::ImageLoader(QObject *parent)
    : QObject{parent}
{
    moveToThread(&t);
    t.start();
}

ImageLoader::~ImageLoader() {
    t.quit();
    t.wait();
}

void ImageLoader::requestFile(QString filePath) {
    QMetaObject::invokeMethod(this, "loadImage", Q_ARG(QString, filePath));
}

void ImageLoader::loadImage(QString filePath) {
    QImage image(filePath);
    emit imageReady(image);
}
