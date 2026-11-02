#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QUrl>
#include <QtWidgets/QGraphicsView>

class QGraphicsScene;
class QGraphicsVideoItem;
class QTimer;

namespace FluentQt {

class MediaPlayerBase;
class StandardMediaPlayBar;

// A video in a graphics view, with a StandardMediaPlayBar floating 11 px above its bottom edge. The bar fades
// in when the pointer enters and out 3 s after it leaves; hiding the widget pauses the video.
class FQW_API VideoWidget : public QGraphicsView
{
    Q_OBJECT

  public:
    explicit VideoWidget(QWidget *parent = nullptr);

    // The play bar's player.
    MediaPlayerBase *player() const;
    StandardMediaPlayBar *playBar() const;
    QGraphicsVideoItem *videoItem() const;
    QGraphicsScene *graphicsScene() const;

  public slots:
    void setVideo(const QUrl &url);
    void play();
    void pause();
    void stop();
    void togglePlayState();

  protected:
    void hideEvent(QHideEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    void onHideTimeOut();

    bool m_isHover = false;
    QTimer *m_timer = nullptr;
    QGraphicsVideoItem *m_videoItem = nullptr;
    QGraphicsScene *m_graphicsScene = nullptr;
    StandardMediaPlayBar *m_playBar = nullptr;
};

} // namespace FluentQt
