#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QSize>
#include <QtCore/QRect>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

class QPainter;

namespace FluentQt {

class FQW_API AcrylicBrush
{
  public:
    explicit AcrylicBrush(QWidget *device = nullptr, int blurRadius = 30,
                          const QColor &tintColor = QColor(242, 242, 242, 150),
                          const QColor &luminosityColor = QColor(255, 255, 255, 10),
                          qreal noiseOpacity = 0.03);

    QWidget *device() const;
    void setDevice(QWidget *device);

    int blurRadius() const;
    QColor tintColor() const;
    QColor luminosityColor() const;
    qreal noiseOpacity() const;
    QPainterPath clipPath() const;
    QSize blurPicSize() const;
    QImage image() const;
    QImage originalImage() const;
    bool isNull() const;
    bool isAvailable() const;

    void setBlurRadius(int radius);
    void setTintColor(const QColor &color);
    void setLuminosityColor(const QColor &color);
    void setNoiseOpacity(qreal opacity);
    void setClipPath(const QPainterPath &path);
    void setBlurPicSize(const QSize &size);
    void grabImage(const QRect &globalRect);
    void setImage(const QImage &image);
    void setPixmap(const QPixmap &pixmap);
    bool setImage(const QString &path);
    bool setImagePath(const QString &path);
    QImage textureImage() const;
    void paint(QPainter *painter = nullptr);

  private:
    void invalidateCache();
    QImage blurredImage(const QImage &image) const;

    QWidget *m_device = nullptr;
    QImage m_originalImage;
    QImage m_image;
    QImage m_cachedImage;
    QSize m_blurPicSize;
    int m_blurRadius = 30;
    QColor m_tintColor = QColor(242, 242, 242, 150);
    QColor m_luminosityColor = QColor(255, 255, 255, 10);
    qreal m_noiseOpacity = 0.03;
    QPainterPath m_clipPath;
};

class FQW_API AcrylicLabel : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int blurRadius READ blurRadius WRITE setBlurRadius NOTIFY blurRadiusChanged)
    Q_PROPERTY(QColor tintColor READ tintColor WRITE setTintColor NOTIFY tintColorChanged)
    Q_PROPERTY(Qt::AspectRatioMode aspectRatioMode READ aspectRatioMode WRITE setAspectRatioMode)

  public:
    explicit AcrylicLabel(QWidget *parent = nullptr);
    AcrylicLabel(int blurRadius, const QColor &tintColor, QWidget *parent = nullptr);
    explicit AcrylicLabel(const QImage &image, QWidget *parent = nullptr);
    explicit AcrylicLabel(const QPixmap &pixmap, QWidget *parent = nullptr);
    explicit AcrylicLabel(const QString &imagePath, QWidget *parent = nullptr);

    int blurRadius() const;
    QColor tintColor() const;
    Qt::AspectRatioMode aspectRatioMode() const;
    QImage image() const;
    QPixmap pixmap() const;
    bool isNull() const;
    QSize sizeHint() const override;

  public slots:
    void setBlurRadius(int radius);
    void setTintColor(const QColor &color);
    void setAspectRatioMode(Qt::AspectRatioMode mode);
    void setImage(const QImage &image);
    bool setImage(const QString &path);
    void setPixmap(const QPixmap &pixmap);
    bool setImagePath(const QString &path);

  signals:
    void blurRadiusChanged(int radius);
    void tintColorChanged(const QColor &color);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    void invalidateCache();
    QImage scaledImageForPaint(qreal devicePixelRatio) const;
    QImage blurredImage(const QImage &image) const;

    QImage m_image;
    mutable QImage m_cachedImage;
    mutable QSize m_cachedDeviceSize;
    mutable qreal m_cachedDevicePixelRatio = 1.0;
    mutable int m_cachedBlurRadius = -1;
    int m_blurRadius = 15;
    QColor m_tintColor = QColor(105, 114, 168, 102);
    Qt::AspectRatioMode m_aspectRatioMode = Qt::KeepAspectRatioByExpanding;
};

} // namespace FluentQt
