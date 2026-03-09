#include "PixmapLabel.h"
#include <QPainter>
#include <QPaintEvent>

PixmapLabel::PixmapLabel(QWidget* parent) :
    QLabel(parent),
    m_cols(0),
    m_rows(0)
{
    // setMinimumSize(sizeHint());
    // setAlignment(Qt::AlignCenter);
    // set borders on the QLabel
    // setScaledContents(true);

    // setStyleSheet("QLabel{border: 1px solid black; background: gray;}");
}

bool PixmapLabel::hasImage() {
    return !m_pixmap.isNull();
}
void PixmapLabel::setImage(const QPixmap& image)
{
    m_pixmap = image;

    if (m_movie != nullptr) {
        m_movie->stop();
        disconnect(m_movie, &QMovie::frameChanged, this, &PixmapLabel::onMovieUpdate);

        delete m_movie;
        m_movie = nullptr;
    }

    m_cols = m_pixmap.width();
    m_rows = m_pixmap.height();

    update();
}

void PixmapLabel::onMovieUpdate() {
    update();
}

void PixmapLabel::setMovieFile(QString filePath) {

    if (!filePath.isEmpty()) {
        QPixmap pm(filePath);
        m_cols = pm.width();
        m_rows = pm.height();
        m_pixmap = QPixmap();

        m_movie = new QMovie(filePath);
        connect(m_movie, &QMovie::frameChanged, this, &PixmapLabel::onMovieUpdate);
        m_movie->start();

    } else {
        m_cols = 0;
        m_rows = 0;
    }

    update();
}

/* virtual */ QSize PixmapLabel::sizeHint() const
{
    // return QSize(parentWidget()->width(), parentWidget()->height());
    // // if (movie() != nullptr)
    // //     return movie()->scaledSize();
    if (m_cols != 0)
    {
        int width = this->parentWidget()->width();
        // if (width > this->width())
        //     width = this->width();
        return QSize(width, heightForWidth(width));
    }
    else
    {
        return QSize(200, 200);
    }
}



/* virtual */ int PixmapLabel::heightForWidth(int width) const
{
    return (m_cols != 0) ? width * m_rows / m_cols: this->parentWidget()->height();
}



/* virtual */ void PixmapLabel::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);

    QPixmap pm = m_pixmap;
    if (m_pixmap.isNull() && m_movie != nullptr)
        pm = m_movie->currentPixmap();

    if (pm.isNull())
        return;

    // Create painter
    QPainter painter(this);

    // Get current pixmap size
    QSize pixmapSize = pm.size();

    // Scale size to painted rect
    pixmapSize.scale(event->rect().size(), Qt::KeepAspectRatio);

    // Scale the pixmap
    QPixmap pixmapScaled = pm.scaled(pixmapSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Draw the pixmap
    painter.drawPixmap(QPoint((this->width()/2) - (pixmapScaled.width()/2), (this->height()/2) - (pixmapScaled.height()/2)), pixmapScaled);
}
