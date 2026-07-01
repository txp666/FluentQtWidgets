#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QRectF>
#include <QtCore/QSize>
#include <QtCore/QStringList>
#include <QtCore/QVector>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

class QPaintEvent;
class QPainter;
class QEnterEvent;
class QMouseEvent;
class QPropertyAnimation;
class QResizeEvent;
class QWheelEvent;

namespace FluentQt {

class FQW_API FlipView : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QSize itemSize READ itemSize WRITE setItemSize)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
    Q_PROPERTY(int spacing READ spacing WRITE setSpacing)
    Q_PROPERTY(Qt::AspectRatioMode aspectRatioMode READ aspectRatioMode WRITE setAspectRatioMode)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(qreal previousButtonOpacity READ previousButtonOpacity WRITE setPreviousButtonOpacity)
    Q_PROPERTY(qreal nextButtonOpacity READ nextButtonOpacity WRITE setNextButtonOpacity)

  public:
    explicit FlipView(QWidget *parent = nullptr);
    explicit FlipView(Qt::Orientation orientation, QWidget *parent = nullptr);

    Qt::Orientation orientation() const;
    bool isHorizontal() const;
    QSize itemSize() const;
    int borderRadius() const;
    int spacing() const;
    Qt::AspectRatioMode aspectRatioMode() const;
    int currentIndex() const;
    qreal previousButtonOpacity() const;
    qreal nextButtonOpacity() const;
    int count() const;
    QImage image(int index) const;
    QSize sizeHint() const override;

  public slots:
    void setItemSize(const QSize &size);
    void setBorderRadius(int radius);
    void setSpacing(int spacing);
    void setAspectRatioMode(Qt::AspectRatioMode mode);
    void setCurrentIndex(int index);
    void scrollPrevious();
    void scrollNext();
    void addImage(const QImage &image);
    void addImage(const QPixmap &pixmap);
    bool addImage(const QString &path);
    void addImages(const QList<QImage> &images);
    void addImages(const QList<QPixmap> &pixmaps);
    void addImages(const QStringList &paths);
    void clear();

  signals:
    void currentIndexChanged(int index);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private:
    QRectF imageRect() const;
    QRect previousButtonRect() const;
    QRect nextButtonRect() const;
    void paintImage(QPainter *painter);
    void paintButton(QPainter *painter, const QRect &rect, bool nextButton, qreal opacity);
    void setPreviousButtonOpacity(qreal opacity);
    void setNextButtonOpacity(qreal opacity);
    void fadePreviousButton(qreal opacity);
    void fadeNextButton(qreal opacity);
    void updateButtonOpacityForCurrentIndex(bool animated);
    void updateCursorForPosition(const QPoint &pos);

    Qt::Orientation m_orientation = Qt::Horizontal;
    QSize m_itemSize = QSize(480, 270);
    int m_borderRadius = 0;
    int m_spacing = 0;
    Qt::AspectRatioMode m_aspectRatioMode = Qt::IgnoreAspectRatio;
    QVector<QImage> m_images;
    int m_currentIndex = -1;
    bool m_hovered = false;
    bool m_previousPressed = false;
    bool m_nextPressed = false;
    bool m_isScrolling = false;
    qreal m_previousButtonOpacity = 0;
    qreal m_nextButtonOpacity = 0;
    QPropertyAnimation *m_previousButtonOpacityAnimation = nullptr;
    QPropertyAnimation *m_nextButtonOpacityAnimation = nullptr;
};

class FQW_API HorizontalFlipView : public FlipView
{
    Q_OBJECT

  public:
    explicit HorizontalFlipView(QWidget *parent = nullptr);
};

class FQW_API VerticalFlipView : public FlipView
{
    Q_OBJECT

  public:
    explicit VerticalFlipView(QWidget *parent = nullptr);
};

} // namespace FluentQt
