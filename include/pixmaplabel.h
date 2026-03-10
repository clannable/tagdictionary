#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H

#include <QLabel>
#include <QPixmap>
#include <QMovie>

class PixmapLabel : public QLabel
{
public:

    PixmapLabel(QWidget* parent = nullptr);
    ~PixmapLabel();

    virtual QSize sizeHint() const override;

    bool hasImage();

    void setImage(const QString& filePath, bool animated);
    int heightForWidth(int width) const override;

public slots:
    void onMovieUpdate();
protected:

    void paintEvent(QPaintEvent* event) override;

private:

    QPixmap m_pixmap;
    int m_cols;
    int m_rows;
    QMovie* m_movie = nullptr;

    // void onMovieUpdate(int frame);
};
#endif // PIXMAPLABEL_H
