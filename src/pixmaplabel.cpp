#include "pixmaplabel.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMimeDatabase>

PixmapLabel::PixmapLabel(QWidget* parent) :
    QLabel(parent),
    m_cols(0),
    m_rows(0)
{
    m_loader = new ImageLoader();
    setAlignment(Qt::AlignCenter);
    connect(m_loader, &ImageLoader::imageReady, this, &PixmapLabel::setImage);
}

PixmapLabel::~PixmapLabel() {
    if (m_movie != nullptr)
        delete m_movie;
    delete m_loader;
}

bool PixmapLabel::hasImage() {
    return !m_pixmap.isNull();
}
void PixmapLabel::setFile(const QString& filePath)
{
    m_pixmap = QPixmap();

    if (filePath.endsWith(".gif")) {
        if (m_movie != nullptr) {
            disconnect(m_movie, &QMovie::updated, this, &PixmapLabel::onMovieUpdate);
            m_movie->stop();
            delete m_movie;
            m_movie = nullptr;
        }
        m_movie = new QMovie(filePath);
        connect(m_movie, &QMovie::updated, this, &PixmapLabel::onMovieUpdate);
        QPixmap pm = m_movie->currentPixmap();
        m_cols = pm.width();
        m_rows = pm.height();
        m_movie->start();
    } else {
        setText("Loading...");
        m_loader->requestFile(filePath);
    }

    update();
}

void PixmapLabel::setImage(QImage image) {
    setText("");
    m_pixmap = QPixmap::fromImage(image);
    if (m_pixmap.isNull())
        setText("Failed to load file");
    m_cols = m_pixmap.width();
    m_rows = m_pixmap.height();

    if (m_movie != nullptr) {
        disconnect(m_movie, &QMovie::updated, this, &PixmapLabel::onMovieUpdate);
        m_movie->stop();
        delete m_movie;
        m_movie = nullptr;
    }
    update();
}


void PixmapLabel::onMovieUpdate() {
    update();
}

/* virtual */ QSize PixmapLabel::sizeHint() const
{
    if (m_cols != 0)
    {
        int width = this->parentWidget()->width();
        return QSize(width, heightForWidth(width));
    }
    else
    {
        return QSize(200, 200);
    }
}



/* virtual */ int PixmapLabel::heightForWidth(int width) const
{
    if (m_cols == 0) return parentWidget()->height();

    int scaleHeight = width * m_rows / m_cols;
    return m_rows < scaleHeight ? m_rows : scaleHeight;
}



/* virtual */ void PixmapLabel::paintEvent(QPaintEvent* event)
{
    QLabel::paintEvent(event);

    QPixmap pm = m_pixmap;
    if (m_pixmap.isNull() && m_movie != nullptr)
        pm = m_movie->currentPixmap();

    // Create painter
    QPainter painter(this);

    if (pm.isNull()) return;

    // Get current pixmap size
    QSize pixmapSize = pm.size();


    // Scale size to painted rect
    pixmapSize.scale(event->rect().size(), Qt::KeepAspectRatio);

    // Scale the pixmap
    QPixmap pixmapScaled = pm.scaled(pixmapSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // Draw the pixmap
    painter.drawPixmap(QPoint((this->width()/2) - (pixmapScaled.width()/2), (this->height()/2) - (pixmapScaled.height()/2)), pixmapScaled);
}
