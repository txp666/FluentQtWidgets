#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QMutex>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtCore/QString>
#include <QtCore/QThread>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
#include <QtWidgets/QLabel>

class QPainter;

namespace FluentQt {

// Port of qfluentwidgets.common.image_utils.gaussianBlur: scipy.ndimage.gaussian_filter with sigma
// blurRadius (kernel radius int(4 * sigma + 0.5), "reflect" edges) on each colour channel, rows then
// columns, each pass truncated to 8 bits like scipy's uint8 output; alpha stays as it is. The colour
// channels are then multiplied by brightFactor. A valid blurPicSize first shrinks a larger image to
// fit inside it. Returns an RGB888 image, or RGBA8888 when the source has alpha. Python returns a
// QPixmap; this returns a QImage so that it can run on a worker thread.
FQW_API QImage gaussianBlur(const QImage &image, qreal blurRadius = 18, qreal brightFactor = 1,
                            const QSize &blurPicSize = QSize());
FQW_API QImage gaussianBlur(const QString &imagePath, qreal blurRadius = 18, qreal brightFactor = 1,
                            const QSize &blurPicSize = QSize());

// Blurs an image file on a background thread, at 0.85 brightness, like Python's BlurCoverThread.
class FQW_API BlurCoverThread : public QThread
{
    Q_OBJECT

  public:
    explicit BlurCoverThread(QObject *parent = nullptr);
    ~BlurCoverThread() override;

    QString imagePath() const;
    int blurRadius() const;
    QSize maxSize() const;

    // An invalid maxSize keeps the previous one (Python's `maxSize or self.maxSize`). A request made
    // while a blur is running is blurred once that one is done.
    void blur(const QString &imagePath, int blurRadius = 6, const QSize &maxSize = QSize(450, 450));

  signals:
    // Python emits a QPixmap; pixmaps cannot be made off the GUI thread in C++.
    void blurFinished(const QImage &image);

  protected:
    void run() override;

  private:
    mutable QMutex m_mutex;
    QString m_imagePath;
    int m_blurRadius = 7;
    QSize m_maxSize;
    bool m_pending = false;
};

// The tint, luminosity and noise layers drawn over an AcrylicLabel's blurred image.
class FQW_API AcrylicTextureLabel : public QLabel
{
    Q_OBJECT

  public:
    AcrylicTextureLabel(const QColor &tintColor, const QColor &luminosityColor, qreal noiseOpacity = 0.03,
                        QWidget *parent = nullptr);

    QColor tintColor() const;
    QColor luminosityColor() const;
    qreal noiseOpacity() const;
    QImage noiseImage() const;
    void setTintColor(const QColor &color);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QColor m_tintColor;
    QColor m_luminosityColor;
    qreal m_noiseOpacity = 0.03;
    QImage m_noiseImage;
};

class FQW_API AcrylicLabel : public QLabel
{
    Q_OBJECT

  public:
    // An invalid maxBlurSize blurs the image at its own size.
    AcrylicLabel(int blurRadius, const QColor &tintColor, const QColor &luminosityColor = QColor(255, 255, 255, 0),
                 const QSize &maxBlurSize = QSize(), QWidget *parent = nullptr);
    AcrylicLabel(int blurRadius, const QColor &tintColor, QWidget *parent);

    QString imagePath() const;
    QPixmap blurPixmap() const;
    // Used by the next setImage(), like assigning Python's blurRadius attribute.
    int blurRadius() const;
    void setBlurRadius(int radius);
    QSize maxBlurSize() const;
    void setMaxBlurSize(const QSize &size);
    AcrylicTextureLabel *acrylicTextureLabel() const;
    BlurCoverThread *blurThread() const;

    // Blurs the image on blurThread(); the label shows it, and takes its size, once that is done.
    void setImage(const QString &imagePath);
    void setTintColor(const QColor &color);

  protected:
    void resizeEvent(QResizeEvent *event) override;

  private:
    void onBlurFinished(const QImage &image);

    QString m_imagePath;
    QPixmap m_blurPixmap;
    int m_blurRadius = 0;
    QSize m_maxBlurSize;
    AcrylicTextureLabel *m_acrylicTextureLabel = nullptr;
    BlurCoverThread *m_blurThread = nullptr;
};

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

    // Re-blurs the current image.
    void setBlurRadius(int radius);
    void setTintColor(const QColor &color);
    void setLuminosityColor(const QColor &color);
    void setNoiseOpacity(qreal opacity);
    void setClipPath(const QPainterPath &path);
    // Applies from the next image on, like Python's.
    void setBlurPicSize(const QSize &size);
    // Captures the screen behind globalRect, when screen capture is available; otherwise the
    // brush paints an opaque tint instead.
    void grabImage(const QRect &globalRect);
    void setImage(const QImage &image);
    void setPixmap(const QPixmap &pixmap);
    bool setImage(const QString &path);
    QImage textureImage() const;
    void paint(QPainter *painter = nullptr);

    // Application-wide switch for the screen capture behind acrylic surfaces (on by default).
    static void setScreenCaptureEnabled(bool enabled);
    static bool isScreenCaptureEnabled();
    // Whether grabImage() can capture without side effects: enabled, not on Wayland (which does
    // not let clients read the screen), and on macOS only once the Screen Recording permission
    // has been granted, so that opening a menu never triggers the system prompt.
    static bool isScreenCaptureAvailable();

  private:
    QWidget *m_device = nullptr;
    QImage m_originalImage;
    QImage m_image;
    QSize m_blurPicSize;
    int m_blurRadius = 30;
    QColor m_tintColor = QColor(242, 242, 242, 150);
    QColor m_luminosityColor = QColor(255, 255, 255, 10);
    qreal m_noiseOpacity = 0.03;
    QPainterPath m_clipPath;
};

} // namespace FluentQt
