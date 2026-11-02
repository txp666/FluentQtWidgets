#include <FluentQtWidgets/Widgets/AcrylicLabel.h>

#include <QtCore/QMutexLocker>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtCore/QVector>
#include <QtGui/QGuiApplication>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QScreen>

#include <cmath>

namespace FluentQt {

#if defined(Q_OS_MACOS)
namespace Private {
bool isScreenCapturePermitted();
}
#endif

namespace {

bool &screenCaptureEnabled()
{
    static bool enabled = true;
    return enabled;
}

// scipy.ndimage's "reflect" mode (d c b a | a b c d | d c b a), for any distance from the edge.
int reflectIndex(int index, int size)
{
    const int period = 2 * size;
    index %= period;
    if (index < 0) {
        index += period;
    }
    return index < size ? index : period - 1 - index;
}

// scipy.ndimage._gaussian_kernel1d(sigma, 0, int(4 * sigma + 0.5)): the centre weight and the
// weights at distance 1..radius.
QVector<double> gaussianKernel(double sigma)
{
    const int radius = int(4.0 * sigma + 0.5);
    QVector<double> weights(radius + 1);
    double sum = 0;
    for (int x = -radius; x <= radius; ++x) {
        const double weight = std::exp(-0.5 / (sigma * sigma) * double(x) * double(x));
        sum += weight;
        if (x >= 0) {
            weights[x] = weight;
        }
    }
    for (double &weight : weights) {
        weight /= sum;
    }
    return weights;
}

// One pass of scipy's correlate1d along `count` lines of `length` samples, the channels of a
// pixel interleaved; results are truncated to 8 bits like scipy's uint8 output.
void correlateLines(uchar *data, int count, int length, qsizetype lineStep, qsizetype sampleStep, int channels,
                    const QVector<double> &weights)
{
    const int radius = int(weights.size()) - 1;
    QVector<double> padded((length + 2 * radius) * channels);
    for (int line = 0; line < count; ++line) {
        uchar *base = data + line * lineStep;
        for (int i = -radius; i < length + radius; ++i) {
            const uchar *sample = base + reflectIndex(i, length) * sampleStep;
            for (int c = 0; c < channels; ++c) {
                padded[(i + radius) * channels + c] = sample[c];
            }
        }
        for (int i = 0; i < length; ++i) {
            const double *center = padded.constData() + (i + radius) * channels;
            uchar *out = base + i * sampleStep;
            for (int c = 0; c < channels; ++c) {
                // The order of NI_Correlate1D for symmetric weights.
                double sum = center[c] * weights[0];
                for (int j = radius; j > 0; --j) {
                    sum += (center[c - j * channels] + center[c + j * channels]) * weights[j];
                }
                out[c] = uchar(sum);
            }
        }
    }
}

QImage acrylicTexture(const QColor &tintColor, const QColor &luminosityColor, qreal noiseOpacity,
                      const QImage &noiseImage)
{
    QImage texture(64, 64, QImage::Format_ARGB32_Premultiplied);
    texture.fill(luminosityColor);

    QPainter painter(&texture);
    painter.fillRect(texture.rect(), tintColor);
    painter.setOpacity(noiseOpacity);
    painter.drawImage(texture.rect(), noiseImage);
    return texture;
}

QImage loadNoiseImage() { return QImage(QStringLiteral(":/qfluentwidgets/images/acrylic/noise.png")); }

} // namespace

AcrylicBrush::AcrylicBrush(QWidget *device, int blurRadius, const QColor &tintColor,
                           const QColor &luminosityColor, qreal noiseOpacity)
    : m_device(device)
    , m_blurRadius(qMax(0, blurRadius))
    , m_tintColor(tintColor)
    , m_luminosityColor(luminosityColor)
    , m_noiseOpacity(qBound<qreal>(0.0, noiseOpacity, 1.0))
{
}

QWidget *AcrylicBrush::device() const { return m_device; }

void AcrylicBrush::setDevice(QWidget *device)
{
    if (m_device == device) {
        return;
    }
    m_device = device;
    if (m_device) {
        m_device->update();
    }
}

int AcrylicBrush::blurRadius() const { return m_blurRadius; }

QColor AcrylicBrush::tintColor() const { return m_tintColor; }

QColor AcrylicBrush::luminosityColor() const { return m_luminosityColor; }

qreal AcrylicBrush::noiseOpacity() const { return m_noiseOpacity; }

QPainterPath AcrylicBrush::clipPath() const { return m_clipPath; }

QSize AcrylicBrush::blurPicSize() const { return m_blurPicSize; }

QImage AcrylicBrush::image() const { return m_image; }

QImage AcrylicBrush::originalImage() const { return m_originalImage; }

bool AcrylicBrush::isNull() const { return m_image.isNull(); }

// Without a capture there is nothing to show through, so acrylic surfaces fall back to their
// solid look (NavigationPanel checks this).
bool AcrylicBrush::isAvailable() const { return isScreenCaptureAvailable(); }

void AcrylicBrush::setScreenCaptureEnabled(bool enabled) { screenCaptureEnabled() = enabled; }

bool AcrylicBrush::isScreenCaptureEnabled() { return screenCaptureEnabled(); }

bool AcrylicBrush::isScreenCaptureAvailable()
{
    if (!screenCaptureEnabled()) {
        return false;
    }
    const QString platform = QGuiApplication::platformName();
    if (platform.startsWith(QLatin1String("wayland"))) {
        return false;
    }
#if defined(Q_OS_MACOS)
    if (platform == QLatin1String("cocoa") && !Private::isScreenCapturePermitted()) {
        return false;
    }
#endif
    return true;
}

void AcrylicBrush::setBlurRadius(int radius)
{
    if (m_blurRadius == radius) {
        return;
    }
    m_blurRadius = radius;
    setImage(m_originalImage);
}

void AcrylicBrush::setTintColor(const QColor &color)
{
    if (m_tintColor == color) {
        return;
    }
    m_tintColor = color;
    if (m_device) {
        m_device->update();
    }
}

void AcrylicBrush::setLuminosityColor(const QColor &color)
{
    if (m_luminosityColor == color) {
        return;
    }
    m_luminosityColor = color;
    if (m_device) {
        m_device->update();
    }
}

void AcrylicBrush::setNoiseOpacity(qreal opacity)
{
    if (qFuzzyCompare(m_noiseOpacity, opacity)) {
        return;
    }
    m_noiseOpacity = opacity;
    if (m_device) {
        m_device->update();
    }
}

void AcrylicBrush::setClipPath(const QPainterPath &path)
{
    m_clipPath = path;
    if (m_device) {
        m_device->update();
    }
}

void AcrylicBrush::setBlurPicSize(const QSize &size) { m_blurPicSize = size; }

void AcrylicBrush::grabImage(const QRect &globalRect)
{
    QScreen *screen = QGuiApplication::screenAt(globalRect.center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen || globalRect.isEmpty()) {
        return;
    }
    if (!isScreenCaptureAvailable()) {
        setImage(QImage());
        return;
    }

    const QRect screenGeometry = screen->geometry();
    const QRect localRect = globalRect.translated(-screenGeometry.topLeft());
    setPixmap(screen->grabWindow(0, localRect.x(), localRect.y(), localRect.width(), localRect.height()));
}

void AcrylicBrush::setImage(const QImage &image)
{
    m_originalImage = image;
    m_image = gaussianBlur(image, m_blurRadius, 1, m_blurPicSize);
    if (m_device) {
        m_device->update();
    }
}

void AcrylicBrush::setPixmap(const QPixmap &pixmap) { setImage(pixmap.toImage()); }

bool AcrylicBrush::setImage(const QString &path)
{
    const QImage image(path);
    setImage(image);
    return !image.isNull();
}

QImage AcrylicBrush::textureImage() const
{
    static const QImage noiseImage = loadNoiseImage();
    return acrylicTexture(m_tintColor, m_luminosityColor, m_noiseOpacity, noiseImage);
}

void AcrylicBrush::paint(QPainter *painter)
{
    if (!m_device) {
        return;
    }

    QPainter ownedPainter;
    if (!painter) {
        ownedPainter.begin(m_device);
        painter = &ownedPainter;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing);
    if (!m_clipPath.isEmpty()) {
        painter->setClipPath(m_clipPath);
    }

    if (!m_image.isNull()) {
        painter->drawImage(QPoint(0, 0),
                           m_image.scaled(m_device->size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        // Nothing captured (see isAvailable()): an opaque base, so that the translucent tint does
        // not turn into a see-through surface.
        QColor base = m_tintColor;
        base.setAlpha(255);
        painter->fillRect(m_device->rect(), base);
    }

    painter->fillRect(m_device->rect(), QBrush(textureImage()));
    painter->restore();
}

QImage gaussianBlur(const QImage &image, qreal blurRadius, qreal brightFactor, const QSize &blurPicSize)
{
    if (image.isNull()) {
        return QImage();
    }

    // Python converts the pixmap to a PIL image: RGBA when it has alpha, RGB otherwise.
    QImage result = image.convertToFormat(image.hasAlphaChannel() ? QImage::Format_RGBA8888 : QImage::Format_RGB888);
    if (blurPicSize.isValid()) {
        const qreal ratio =
            qMin(qreal(blurPicSize.width()) / result.width(), qreal(blurPicSize.height()) / result.height());
        const qreal width = result.width() * ratio;
        if (width < result.width()) {
            result = result.scaled(int(width), int(result.height() * ratio), Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);
        }
    }
    if (result.isNull()) {
        return result;
    }
    result.detach();

    const int channels = 3;
    const int pixelBytes = result.format() == QImage::Format_RGB888 ? 3 : 4;
    uchar *bits = result.bits();
    const qsizetype stride = result.bytesPerLine();
    if (blurRadius > 1e-15) {
        const QVector<double> weights = gaussianKernel(blurRadius);
        // Axis 0 (columns) first, then axis 1 (rows), as gaussian_filter does.
        correlateLines(bits, result.width(), result.height(), pixelBytes, stride, channels, weights);
        correlateLines(bits, result.height(), result.width(), stride, pixelBytes, channels, weights);
    }
    if (brightFactor != 1) {
        for (int y = 0; y < result.height(); ++y) {
            uchar *line = bits + y * stride;
            for (int x = 0; x < result.width(); ++x) {
                for (int c = 0; c < channels; ++c) {
                    uchar &value = line[x * pixelBytes + c];
                    value = uchar(qBound(0.0, value * double(brightFactor), 255.0));
                }
            }
        }
    }
    return result;
}

QImage gaussianBlur(const QString &imagePath, qreal blurRadius, qreal brightFactor, const QSize &blurPicSize)
{
    return gaussianBlur(QImage(imagePath), blurRadius, brightFactor, blurPicSize);
}

// ==========================================
// BlurCoverThread
// ==========================================

BlurCoverThread::BlurCoverThread(QObject *parent) : QThread(parent) {}

BlurCoverThread::~BlurCoverThread() { wait(); }

QString BlurCoverThread::imagePath() const
{
    QMutexLocker locker(&m_mutex);
    return m_imagePath;
}

int BlurCoverThread::blurRadius() const
{
    QMutexLocker locker(&m_mutex);
    return m_blurRadius;
}

QSize BlurCoverThread::maxSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_maxSize;
}

void BlurCoverThread::blur(const QString &imagePath, int blurRadius, const QSize &maxSize)
{
    {
        QMutexLocker locker(&m_mutex);
        m_imagePath = imagePath;
        m_blurRadius = blurRadius;
        if (maxSize.isValid()) {
            m_maxSize = maxSize;
        }
        m_pending = true;
    }
    start();
}

void BlurCoverThread::run()
{
    forever
    {
        QString imagePath;
        int blurRadius = 0;
        QSize maxSize;
        {
            QMutexLocker locker(&m_mutex);
            if (!m_pending) {
                return;
            }
            m_pending = false;
            imagePath = m_imagePath;
            blurRadius = m_blurRadius;
            maxSize = m_maxSize;
        }
        if (imagePath.isEmpty()) {
            continue;
        }
        emit blurFinished(gaussianBlur(imagePath, blurRadius, 0.85, maxSize));
    }
}

// ==========================================
// AcrylicTextureLabel
// ==========================================

AcrylicTextureLabel::AcrylicTextureLabel(const QColor &tintColor, const QColor &luminosityColor, qreal noiseOpacity,
                                         QWidget *parent)
    : QLabel(parent), m_tintColor(tintColor), m_luminosityColor(luminosityColor), m_noiseOpacity(noiseOpacity),
      m_noiseImage(loadNoiseImage())
{
    setAttribute(Qt::WA_TranslucentBackground);
}

QColor AcrylicTextureLabel::tintColor() const { return m_tintColor; }

QColor AcrylicTextureLabel::luminosityColor() const { return m_luminosityColor; }

qreal AcrylicTextureLabel::noiseOpacity() const { return m_noiseOpacity; }

QImage AcrylicTextureLabel::noiseImage() const { return m_noiseImage; }

void AcrylicTextureLabel::setTintColor(const QColor &color)
{
    m_tintColor = color;
    update();
}

void AcrylicTextureLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), QBrush(acrylicTexture(m_tintColor, m_luminosityColor, m_noiseOpacity, m_noiseImage)));
}

// ==========================================
// AcrylicLabel
// ==========================================

AcrylicLabel::AcrylicLabel(int blurRadius, const QColor &tintColor, const QColor &luminosityColor,
                           const QSize &maxBlurSize, QWidget *parent)
    : QLabel(parent), m_blurRadius(blurRadius), m_maxBlurSize(maxBlurSize)
{
    m_acrylicTextureLabel = new AcrylicTextureLabel(tintColor, luminosityColor, 0.03, this);
    m_blurThread = new BlurCoverThread(this);
    connect(m_blurThread, &BlurCoverThread::blurFinished, this, &AcrylicLabel::onBlurFinished);
}

AcrylicLabel::AcrylicLabel(int blurRadius, const QColor &tintColor, QWidget *parent)
    : AcrylicLabel(blurRadius, tintColor, QColor(255, 255, 255, 0), QSize(), parent)
{
}

QString AcrylicLabel::imagePath() const { return m_imagePath; }

QPixmap AcrylicLabel::blurPixmap() const { return m_blurPixmap; }

int AcrylicLabel::blurRadius() const { return m_blurRadius; }

void AcrylicLabel::setBlurRadius(int radius) { m_blurRadius = radius; }

QSize AcrylicLabel::maxBlurSize() const { return m_maxBlurSize; }

void AcrylicLabel::setMaxBlurSize(const QSize &size) { m_maxBlurSize = size; }

AcrylicTextureLabel *AcrylicLabel::acrylicTextureLabel() const { return m_acrylicTextureLabel; }

BlurCoverThread *AcrylicLabel::blurThread() const { return m_blurThread; }

void AcrylicLabel::onBlurFinished(const QImage &image)
{
    m_blurPixmap = QPixmap::fromImage(image);
    setPixmap(m_blurPixmap);
    adjustSize();
}

void AcrylicLabel::setImage(const QString &imagePath)
{
    m_imagePath = imagePath;
    m_blurThread->blur(imagePath, m_blurRadius, m_maxBlurSize);
}

void AcrylicLabel::setTintColor(const QColor &color) { m_acrylicTextureLabel->setTintColor(color); }

void AcrylicLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    m_acrylicTextureLabel->resize(size());

    if (!m_blurPixmap.isNull() && m_blurPixmap.size() != size()) {
        setPixmap(m_blurPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    }
}

} // namespace FluentQt
