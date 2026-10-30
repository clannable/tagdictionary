#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QMovie>
#include "imageloader.h"

class PixmapLabel : public QLabel
{
public:

    PixmapLabel(QWidget* parent = nullptr);
    ~PixmapLabel();

    virtual QSize sizeHint() const override;

    bool hasImage();

    void setFile(const QString& filePath);
    int heightForWidth(int width) const override;

public slots:
    void setImage(QImage image);
    void onMovieUpdate();
protected:

    void paintEvent(QPaintEvent* event) override;

private:

    QPixmap m_pixmap;
    int m_cols;
    int m_rows;
    QMovie* m_movie = nullptr;
    ImageLoader* m_loader;
};
#endif // PIXMAPLABEL_H
