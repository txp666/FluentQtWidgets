#include <FluentQtWidgets/Widgets/AcrylicLabel.h>

#include <FluentQtWidgets/StyleSheet.h>

#include <QtCore/QtMath>
#include <QtCore/QPoint>
#include <QtCore/QRect>
#include <QtGui/QGuiApplication>
#include <QtGui/QImageReader>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QScreen>
#include <QtWidgets/QSizePolicy>

namespace FluentQt {
namespace {

QImage boxBlur(const QImage &source, int radius)
{
    if (source.isNull() || radius <= 0) {
        return source;
    }

    const QImage input = source.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    const int width = input.width();
    const int height = input.height();
    if (width <= 1 || height <= 1) {
        return input;
    }

    const int window = radius * 2 + 1;
    QImage horizontal(width, height, QImage::Format_ARGB32_Premultiplied);
    QImage output(width, height, QImage::Format_ARGB32_Premultiplied);

    for (int y = 0; y < height; ++y) {
        const auto *src = reinterpret_cast<const QRgb *>(input.constScanLine(y));
        auto *dst = reinterpret_cast<QRgb *>(horizontal.scanLine(y));

        int a = 0;
        int r = 0;
        int g = 0;
        int b = 0;
        for (int i = -radius; i <= radius; ++i) {
            const QRgb pixel = src[qBound(0, i, width - 1)];
            a += qAlpha(pixel);
            r += qRed(pixel);
            g += qGreen(pixel);
            b += qBlue(pixel);
        }

        for (int x = 0; x < width; ++x) {
            dst[x] = qRgba(r / window, g / window, b / window, a / window);
            const int removeIndex = qBound(0, x - radius, width - 1);
            const int addIndex = qBound(0, x + radius + 1, width - 1);
            const QRgb removePixel = src[removeIndex];
            const QRgb addPixel = src[addIndex];
            a += qAlpha(addPixel) - qAlpha(removePixel);
            r += qRed(addPixel) - qRed(removePixel);
            g += qGreen(addPixel) - qGreen(removePixel);
            b += qBlue(addPixel) - qBlue(removePixel);
        }
    }

    for (int x = 0; x < width; ++x) {
        int a = 0;
        int r = 0;
        int g = 0;
        int b = 0;
        for (int i = -radius; i <= radius; ++i) {
            const auto *src = reinterpret_cast<const QRgb *>(horizontal.constScanLine(qBound(0, i, height - 1)));
            const QRgb pixel = src[x];
            a += qAlpha(pixel);
            r += qRed(pixel);
            g += qGreen(pixel);
            b += qBlue(pixel);
        }

        for (int y = 0; y < height; ++y) {
            auto *dst = reinterpret_cast<QRgb *>(output.scanLine(y));
            dst[x] = qRgba(r / window, g / window, b / window, a / window);
            const int removeIndex = qBound(0, y - radius, height - 1);
            const int addIndex = qBound(0, y + radius + 1, height - 1);
            const auto *removeLine = reinterpret_cast<const QRgb *>(horizontal.constScanLine(removeIndex));
            const auto *addLine = reinterpret_cast<const QRgb *>(horizontal.constScanLine(addIndex));
            const QRgb removePixel = removeLine[x];
            const QRgb addPixel = addLine[x];
            a += qAlpha(addPixel) - qAlpha(removePixel);
            r += qRed(addPixel) - qRed(removePixel);
            g += qGreen(addPixel) - qGreen(removePixel);
            b += qBlue(addPixel) - qBlue(removePixel);
        }
    }

    return output;
}

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

bool AcrylicBrush::isAvailable() const { return true; }

void AcrylicBrush::setBlurRadius(int radius)
{
    const int boundedRadius = qMax(0, radius);
    if (m_blurRadius == boundedRadius) {
        return;
    }
    m_blurRadius = boundedRadius;
    invalidateCache();
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
    const qreal boundedOpacity = qBound<qreal>(0.0, opacity, 1.0);
    if (qFuzzyCompare(m_noiseOpacity, boundedOpacity)) {
        return;
    }
    m_noiseOpacity = boundedOpacity;
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

void AcrylicBrush::setBlurPicSize(const QSize &size)
{
    if (m_blurPicSize == size) {
        return;
    }
    m_blurPicSize = size;
    invalidateCache();
    setImage(m_originalImage);
}

void AcrylicBrush::grabImage(const QRect &globalRect)
{
    QScreen *screen = QGuiApplication::screenAt(globalRect.center());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen || globalRect.isEmpty()) {
        return;
    }

    const QRect screenGeometry = screen->geometry();
    const QRect localRect = globalRect.translated(-screenGeometry.topLeft());
    setPixmap(screen->grabWindow(0, localRect.x(), localRect.y(), localRect.width(), localRect.height()));
}

void AcrylicBrush::setImage(const QImage &image)
{
    m_originalImage = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    if (m_originalImage.isNull()) {
        m_image = QImage();
    } else {
        m_image = blurredImage(m_originalImage);
    }
    if (m_device) {
        m_device->update();
    }
}

void AcrylicBrush::setPixmap(const QPixmap &pixmap) { setImage(pixmap.toImage()); }

bool AcrylicBrush::setImage(const QString &path) { return setImagePath(path); }

bool AcrylicBrush::setImagePath(const QString &path)
{
    QImageReader reader(path);
    const QImage loaded = reader.read();
    if (loaded.isNull()) {
        return false;
    }
    setImage(loaded);
    return true;
}

QImage AcrylicBrush::textureImage() const
{
    QImage texture(64, 64, QImage::Format_ARGB32_Premultiplied);
    texture.fill(m_luminosityColor);

    QPainter texturePainter(&texture);
    texturePainter.fillRect(texture.rect(), m_tintColor);

    QImage noise(QStringLiteral(":/qfluentwidgets/images/acrylic/noise.png"));
    if (!noise.isNull() && m_noiseOpacity > 0) {
        texturePainter.setOpacity(m_noiseOpacity);
        texturePainter.drawImage(texture.rect(), noise);
    }
    return texture;
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
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    if (!m_clipPath.isEmpty()) {
        painter->setClipPath(m_clipPath);
    }

    if (!m_image.isNull()) {
        QImage image = m_image.scaled(m_device->size(), Qt::KeepAspectRatioByExpanding,
                                      Qt::SmoothTransformation);
        painter->drawImage(QPoint(0, 0), image);
    }

    painter->fillRect(m_device->rect(), QBrush(textureImage()));
    painter->restore();
}

void AcrylicBrush::invalidateCache() { m_cachedImage = QImage(); }

QImage AcrylicBrush::blurredImage(const QImage &image) const
{
    if (image.isNull()) {
        return image;
    }

    QImage source = image;
    if (m_blurPicSize.isValid() && !m_blurPicSize.isEmpty()) {
        source = source.scaled(m_blurPicSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    }

    if (m_blurRadius <= 0) {
        return source;
    }
    QImage blurred = boxBlur(source, m_blurRadius);
    blurred = boxBlur(blurred, m_blurRadius);
    return boxBlur(blurred, m_blurRadius);
}

AcrylicLabel::AcrylicLabel(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicLabel"));
}

AcrylicLabel::AcrylicLabel(int blurRadius, const QColor &tintColor, QWidget *parent) : AcrylicLabel(parent)
{
    m_blurRadius = qMax(0, blurRadius);
    m_tintColor = tintColor;
}

AcrylicLabel::AcrylicLabel(const QImage &image, QWidget *parent) : AcrylicLabel(parent) { setImage(image); }

AcrylicLabel::AcrylicLabel(const QPixmap &pixmap, QWidget *parent) : AcrylicLabel(parent) { setPixmap(pixmap); }

AcrylicLabel::AcrylicLabel(const QString &imagePath, QWidget *parent) : AcrylicLabel(parent) { setImagePath(imagePath); }

int AcrylicLabel::blurRadius() const { return m_blurRadius; }

QColor AcrylicLabel::tintColor() const { return m_tintColor; }

Qt::AspectRatioMode AcrylicLabel::aspectRatioMode() const { return m_aspectRatioMode; }

QImage AcrylicLabel::image() const { return m_image; }

QPixmap AcrylicLabel::pixmap() const { return QPixmap::fromImage(m_image); }

bool AcrylicLabel::isNull() const { return m_image.isNull(); }

QSize AcrylicLabel::sizeHint() const
{
    if (!m_image.isNull()) {
        return m_image.size();
    }
    return QSize(320, 180);
}

void AcrylicLabel::setBlurRadius(int radius)
{
    const int boundedRadius = qMax(0, radius);
    if (m_blurRadius == boundedRadius) {
        return;
    }

    m_blurRadius = boundedRadius;
    invalidateCache();
    update();
    emit blurRadiusChanged(m_blurRadius);
}

void AcrylicLabel::setTintColor(const QColor &color)
{
    if (m_tintColor == color) {
        return;
    }

    m_tintColor = color;
    update();
    emit tintColorChanged(m_tintColor);
}

void AcrylicLabel::setAspectRatioMode(Qt::AspectRatioMode mode)
{
    if (m_aspectRatioMode == mode) {
        return;
    }

    m_aspectRatioMode = mode;
    invalidateCache();
    update();
}

void AcrylicLabel::setImage(const QImage &image)
{
    m_image = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    invalidateCache();
    updateGeometry();
    update();
}

bool AcrylicLabel::setImage(const QString &path) { return setImagePath(path); }

void AcrylicLabel::setPixmap(const QPixmap &pixmap) { setImage(pixmap.toImage()); }

bool AcrylicLabel::setImagePath(const QString &path)
{
    QImageReader reader(path);
    const QImage loaded = reader.read();
    if (loaded.isNull()) {
        return false;
    }

    setImage(loaded);
    return true;
}

void AcrylicLabel::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    if (m_image.isNull()) {
        painter.fillRect(rect(), m_tintColor);
        return;
    }

    const qreal dpr = devicePixelRatioF();
    const QImage image = scaledImageForPaint(dpr);
    painter.drawImage(rect(), image);
    painter.fillRect(rect(), m_tintColor);
}

void AcrylicLabel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    invalidateCache();
}

void AcrylicLabel::invalidateCache()
{
    m_cachedImage = QImage();
    m_cachedDeviceSize = QSize();
    m_cachedDevicePixelRatio = 1.0;
    m_cachedBlurRadius = -1;
}

QImage AcrylicLabel::scaledImageForPaint(qreal devicePixelRatio) const
{
    const QSize deviceSize(qCeil(width() * devicePixelRatio), qCeil(height() * devicePixelRatio));
    if (!m_cachedImage.isNull() && m_cachedDeviceSize == deviceSize &&
        qFuzzyCompare(m_cachedDevicePixelRatio, devicePixelRatio) && m_cachedBlurRadius == m_blurRadius) {
        return m_cachedImage;
    }

    if (deviceSize.isEmpty()) {
        return QImage();
    }

    QImage scaled = m_image.scaled(deviceSize, m_aspectRatioMode, Qt::SmoothTransformation);
    if (m_aspectRatioMode == Qt::KeepAspectRatioByExpanding && scaled.size() != deviceSize) {
        const int x = qMax(0, (scaled.width() - deviceSize.width()) / 2);
        const int y = qMax(0, (scaled.height() - deviceSize.height()) / 2);
        scaled = scaled.copy(QRect(QPoint(x, y), deviceSize));
    }

    if (scaled.size() != deviceSize) {
        QImage canvas(deviceSize, QImage::Format_ARGB32_Premultiplied);
        canvas.fill(Qt::transparent);
        QPainter canvasPainter(&canvas);
        canvasPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        const QPoint topLeft((deviceSize.width() - scaled.width()) / 2, (deviceSize.height() - scaled.height()) / 2);
        canvasPainter.drawImage(topLeft, scaled);
        scaled = canvas;
    }

    m_cachedImage = blurredImage(scaled);
    m_cachedImage.setDevicePixelRatio(devicePixelRatio);
    m_cachedDeviceSize = deviceSize;
    m_cachedDevicePixelRatio = devicePixelRatio;
    m_cachedBlurRadius = m_blurRadius;
    return m_cachedImage;
}

QImage AcrylicLabel::blurredImage(const QImage &image) const
{
    if (m_blurRadius <= 0) {
        return image;
    }

    // Three box passes approximate a Gaussian blur closely enough for acrylic material.
    QImage blurred = boxBlur(image, m_blurRadius);
    blurred = boxBlur(blurred, m_blurRadius);
    return boxBlur(blurred, m_blurRadius);
}

} // namespace FluentQt
