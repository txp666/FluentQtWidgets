#include <FluentQtWidgets/Multimedia/VideoWidget.h>

#include <FluentQtWidgets/Multimedia/MediaPlayBar.h>
#include <FluentQtWidgets/Multimedia/MediaPlayer.h>
#include <FluentQtWidgets/StyleSheet.h>

#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtMultimediaWidgets/QGraphicsVideoItem>
#include <QtWidgets/QGraphicsScene>

namespace FluentQt {

VideoWidget::VideoWidget(QWidget *parent) : QGraphicsView(parent)
{
    m_timer = new QTimer(this);
    m_videoItem = new QGraphicsVideoItem();
    m_graphicsScene = new QGraphicsScene(this);
    m_playBar = new StandardMediaPlayBar(this);

    setMouseTracking(true);
    setScene(m_graphicsScene);
    m_graphicsScene->addItem(m_videoItem);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    player()->setVideoOutput(m_videoItem);
    FluentStyleSheet::setRole(this, QStringLiteral("MediaPlayer"));

    connect(m_timer, &QTimer::timeout, this, &VideoWidget::onHideTimeOut);
}

MediaPlayerBase *VideoWidget::player() const { return m_playBar->player(); }

StandardMediaPlayBar *VideoWidget::playBar() const { return m_playBar; }

QGraphicsVideoItem *VideoWidget::videoItem() const { return m_videoItem; }

QGraphicsScene *VideoWidget::graphicsScene() const { return m_graphicsScene; }

void VideoWidget::setVideo(const QUrl &url)
{
    player()->setSource(url);
    fitInView(m_videoItem, Qt::KeepAspectRatio);
}

void VideoWidget::play() { m_playBar->play(); }

void VideoWidget::pause() { m_playBar->pause(); }

void VideoWidget::stop() { m_playBar->stop(); }

void VideoWidget::togglePlayState()
{
    if (player()->isPlaying()) {
        pause();
    } else {
        play();
    }
}

void VideoWidget::hideEvent(QHideEvent *event)
{
    pause();
    event->accept();
}

void VideoWidget::wheelEvent(QWheelEvent *) {}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void VideoWidget::enterEvent(QEnterEvent *)
#else
void VideoWidget::enterEvent(QEvent *)
#endif
{
    m_isHover = true;
    m_playBar->fadeIn();
}

void VideoWidget::leaveEvent(QEvent *)
{
    m_isHover = false;
    m_timer->start(3000);
}

void VideoWidget::onHideTimeOut()
{
    if (!m_isHover) {
        m_playBar->fadeOut();
    }
}

void VideoWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    m_videoItem->setSize(QSizeF(size()));
    fitInView(m_videoItem, Qt::KeepAspectRatio);
    m_playBar->move(11, height() - m_playBar->height() - 11);
    m_playBar->setFixedSize(width() - 22, m_playBar->height());
}

} // namespace FluentQt
