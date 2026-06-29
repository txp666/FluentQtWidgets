#include <FluentQtWidgets/Widgets/Label.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Menu.h>

#include <QtCore/QRectF>
#include <QtCore/QUrl>
#include <QtGui/QClipboard>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QDesktopServices>
#include <QtGui/QFont>
#include <QtGui/QImageReader>
#include <QtGui/QMouseEvent>
#include <QtGui/QMovie>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtWidgets/QApplication>

namespace FluentQt {

namespace {

bool isRemoteImageUrl(const QUrl &url)
{
    const QString scheme = url.scheme().toLower();
    return url.isValid() && !url.host().isEmpty() &&
           (scheme == QStringLiteral("http") || scheme == QStringLiteral("https"));
}

} // namespace

// FluentLabelBase
FluentLabelBase::FluentLabelBase(QWidget *parent) : QLabel(parent)
{
    setFluentTextStyle(QStringLiteral("BodyLabel"), 14);
    connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged,
            this, &FluentLabelBase::updateTextColor);
}

FluentLabelBase::FluentLabelBase(const QString &text, QWidget *parent) : QLabel(text, parent)
{
    setFluentTextStyle(QStringLiteral("BodyLabel"), 14);
    connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged,
            this, &FluentLabelBase::updateTextColor);
}

void FluentLabelBase::setFluentTextStyle(const QString &role, int pixelSize, QFont::Weight weight)
{
    QFont labelFont = font();
    labelFont.setPixelSize(pixelSize);
    labelFont.setWeight(weight);
    setFont(labelFont);
    FluentStyleSheet::setRole(this, role);
    m_lightColor = QColor(32, 32, 32);
    m_darkColor = QColor(243, 243, 243);
    updateTextColor();
}

QColor FluentLabelBase::lightColor() const { return m_lightColor; }

QColor FluentLabelBase::darkColor() const { return m_darkColor; }

int FluentLabelBase::pixelFontSize() const { return font().pixelSize(); }

bool FluentLabelBase::strikeOut() const { return font().strikeOut(); }

bool FluentLabelBase::underline() const { return font().underline(); }

void FluentLabelBase::setLightColor(const QColor &color) { setTextColor(color, m_darkColor); }

void FluentLabelBase::setDarkColor(const QColor &color) { setTextColor(m_lightColor, color); }

void FluentLabelBase::setTextColor(const QColor &light, const QColor &dark)
{
    m_lightColor = light;
    m_darkColor = dark;
    updateTextColor();
}

void FluentLabelBase::updateTextColor()
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    QPalette pal = palette();
    pal.setColor(QPalette::WindowText, dark ? m_darkColor : m_lightColor);
    pal.setColor(QPalette::Text, dark ? m_darkColor : m_lightColor);
    setPalette(pal);
}

void FluentLabelBase::setPixelFontSize(int size)
{
    QFont f = font();
    f.setPixelSize(size);
    setFont(f);
}

void FluentLabelBase::setStrikeOut(bool strikeOut)
{
    QFont f = font();
    f.setStrikeOut(strikeOut);
    setFont(f);
}

void FluentLabelBase::setUnderline(bool underline)
{
    QFont f = font();
    f.setUnderline(underline);
    setFont(f);
}

void FluentLabelBase::contextMenuEvent(QContextMenuEvent *event)
{
    auto *menu = new LabelContextMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    menu->exec(event->globalPos(), true, MenuAnimationType::FadeInDropDown);
}

// CaptionLabel
CaptionLabel::CaptionLabel(QWidget *parent) : FluentLabelBase(parent)
{
    setFluentTextStyle(QStringLiteral("CaptionLabel"), 12);
}

CaptionLabel::CaptionLabel(const QString &text, QWidget *parent) : FluentLabelBase(text, parent)
{
    setFluentTextStyle(QStringLiteral("CaptionLabel"), 12);
}

// BodyLabel
BodyLabel::BodyLabel(QWidget *parent) : FluentLabelBase(parent) { setFluentTextStyle(QStringLiteral("BodyLabel"), 14); }
BodyLabel::BodyLabel(const QString &text, QWidget *parent) : FluentLabelBase(text, parent)
{
    setFluentTextStyle(QStringLiteral("BodyLabel"), 14);
}

// StrongBodyLabel
StrongBodyLabel::StrongBodyLabel(QWidget *parent) : FluentLabelBase(parent)
{
    setFluentTextStyle(QStringLiteral("StrongBodyLabel"), 14, QFont::DemiBold);
}
StrongBodyLabel::StrongBodyLabel(const QString &text, QWidget *parent) : FluentLabelBase(text, parent)
{
    setFluentTextStyle(QStringLiteral("StrongBodyLabel"), 14, QFont::DemiBold);
}

// SubtitleLabel
SubtitleLabel::SubtitleLabel(QWidget *parent) : FluentLabelBase(parent)
{
    setFluentTextStyle(QStringLiteral("SubtitleLabel"), 20, QFont::DemiBold);
}
SubtitleLabel::SubtitleLabel(const QString &text, QWidget *parent) : FluentLabelBase(text, parent)
{
    setFluentTextStyle(QStringLiteral("SubtitleLabel"), 20, QFont::DemiBold);
}

// TitleLabel
TitleLabel::TitleLabel(QWidget *parent) : FluentLabelBase(parent)
{
    setFluentTextStyle(QStringLiteral("TitleLabel"), 28, QFont::DemiBold);
}
TitleLabel::TitleLabel(const QString &text, QWidget *parent) : FluentLabelBase(text, parent)
{
    setFluentTextStyle(QStringLiteral("TitleLabel"), 28, QFont::DemiBold);
}

// LargeTitleLabel
LargeTitleLabel::LargeTitleLabel(QWidget *parent) : FluentLabelBase(parent)
{
    setFluentTextStyle(QStringLiteral("LargeTitleLabel"), 40, QFont::DemiBold);
}
LargeTitleLabel::LargeTitleLabel(const QString &text, QWidget *parent) : FluentLabelBase(text, parent)
{
    setFluentTextStyle(QStringLiteral("LargeTitleLabel"), 40, QFont::DemiBold);
}

// DisplayLabel
DisplayLabel::DisplayLabel(QWidget *parent) : FluentLabelBase(parent)
{
    setFluentTextStyle(QStringLiteral("DisplayLabel"), 68, QFont::DemiBold);
}
DisplayLabel::DisplayLabel(const QString &text, QWidget *parent) : FluentLabelBase(text, parent)
{
    setFluentTextStyle(QStringLiteral("DisplayLabel"), 68, QFont::DemiBold);
}

// ImageLabel
ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(this, QStringLiteral("ImageLabel"));
}

ImageLabel::ImageLabel(const QString &path, QWidget *parent) : ImageLabel(parent) { setImagePath(path); }

ImageLabel::ImageLabel(const QPixmap &pixmap, QWidget *parent) : ImageLabel(parent) { setPixmap(pixmap); }

ImageLabel::ImageLabel(const QImage &image, QWidget *parent) : ImageLabel(parent) { setImage(image); }

QImage ImageLabel::image() const { return m_image; }

QPixmap ImageLabel::imagePixmap() const { return QPixmap::fromImage(m_image); }

bool ImageLabel::isNull() const { return m_image.isNull(); }

int ImageLabel::borderRadius() const { return m_topLeftRadius; }

int ImageLabel::topLeftRadius() const { return m_topLeftRadius; }
int ImageLabel::topRightRadius() const { return m_topRightRadius; }
int ImageLabel::bottomLeftRadius() const { return m_bottomLeftRadius; }
int ImageLabel::bottomRightRadius() const { return m_bottomRightRadius; }

void ImageLabel::setImage(const QImage &image)
{
    m_image = image;
    if (!m_image.isNull()) {
        setFixedSize(m_image.size());
    }
    update();
}

void ImageLabel::setPixmap(const QPixmap &pixmap) { setImage(pixmap.toImage()); }

bool ImageLabel::setImagePath(const QString &path)
{
    QImageReader reader(path);
    if (reader.supportsAnimation()) {
        auto *movie = new QMovie(path, QByteArray(), this);
        if (!movie->isValid()) {
            delete movie;
            return false;
        }
        setMovie(movie);
        return true;
    }

    const QImage loaded = reader.read();
    if (loaded.isNull()) {
        return false;
    }
    setImage(loaded);
    return true;
}

void ImageLabel::setBorderRadius(int topLeft, int topRight, int bottomLeft, int bottomRight)
{
    m_topLeftRadius = qMax(0, topLeft);
    m_topRightRadius = qMax(0, topRight);
    m_bottomLeftRadius = qMax(0, bottomLeft);
    m_bottomRightRadius = qMax(0, bottomRight);
    update();
}

void ImageLabel::setBorderRadius(int radius)
{
    setBorderRadius(radius, radius, radius, radius);
}

void ImageLabel::setTopLeftRadius(int radius)
{
    setBorderRadius(radius, m_topRightRadius, m_bottomLeftRadius, m_bottomRightRadius);
}
void ImageLabel::setTopRightRadius(int radius)
{
    setBorderRadius(m_topLeftRadius, radius, m_bottomLeftRadius, m_bottomRightRadius);
}
void ImageLabel::setBottomLeftRadius(int radius)
{
    setBorderRadius(m_topLeftRadius, m_topRightRadius, radius, m_bottomRightRadius);
}
void ImageLabel::setBottomRightRadius(int radius)
{
    setBorderRadius(m_topLeftRadius, m_topRightRadius, m_bottomLeftRadius, radius);
}

void ImageLabel::scaledToWidth(int width)
{
    if (isNull()) return;
    int h = int(double(width) / m_image.width() * m_image.height());
    setFixedSize(width, h);
    if (m_movie) {
        m_movie->setScaledSize(QSize(width, h));
    }
}

void ImageLabel::scaledToHeight(int height)
{
    if (isNull()) return;
    int w = int(double(height) / m_image.height() * m_image.width());
    setFixedSize(w, height);
    if (m_movie) {
        m_movie->setScaledSize(QSize(w, height));
    }
}

void ImageLabel::setScaledSize(const QSize &size)
{
    if (isNull()) return;
    setFixedSize(size);
    if (m_movie) {
        m_movie->setScaledSize(size);
    }
}

void ImageLabel::setMovie(QMovie *movie)
{
    if (m_movie) {
        m_movie->stop();
        disconnect(m_movie, nullptr, this, nullptr);
        m_movie->deleteLater();
    }
    m_movie = movie;
    if (m_movie) {
        m_movie->setParent(this);
        connect(m_movie, &QMovie::frameChanged, this, &ImageLabel::onMovieFrameChanged);
        m_movie->start();
        m_image = m_movie->currentImage();
        if (!m_image.isNull()) {
            setFixedSize(m_image.size());
        }
        update();
    }
}

void ImageLabel::onMovieFrameChanged(int)
{
    if (!m_movie) return;
    m_image = m_movie->currentImage();
    update();
}

void ImageLabel::paintEvent(QPaintEvent *)
{
    if (m_image.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const qreal dpr = devicePixelRatioF();
    QImage scaled = m_image.scaled(size() * dpr, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    // Build path with 4 independent corner radii
    QPainterPath path;
    const int w = width();
    const int h = height();
    const int tl = m_topLeftRadius;
    const int tr = m_topRightRadius;
    const int bl = m_bottomLeftRadius;
    const int br = m_bottomRightRadius;

    if (tl > 0 || tr > 0 || bl > 0 || br > 0) {
        path.moveTo(tl, 0);
        path.lineTo(w - tr, 0);
        if (tr > 0) path.arcTo(w - tr * 2, 0, tr * 2, tr * 2, 90, -90);
        else path.lineTo(w, 0);
        path.lineTo(w, h - br);
        if (br > 0) path.arcTo(w - br * 2, h - br * 2, br * 2, br * 2, 0, -90);
        else path.lineTo(w, h);
        path.lineTo(bl, h);
        if (bl > 0) path.arcTo(0, h - bl * 2, bl * 2, bl * 2, -90, -90);
        else path.lineTo(0, h);
        path.lineTo(0, tl);
        if (tl > 0) path.arcTo(0, 0, tl * 2, tl * 2, -180, -90);
        else path.lineTo(0, 0);
        path.closeSubpath();
        painter.setClipPath(path);
    }

    painter.setPen(Qt::NoPen);
    painter.drawImage(rect(), scaled);
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *event)
{
    QLabel::mouseReleaseEvent(event);
    if (event->button() == Qt::LeftButton) {
        emit clicked();
    }
}

// AvatarWidget
AvatarWidget::AvatarWidget(QWidget *parent) : ImageLabel(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setRadius(48);
    FluentStyleSheet::setRole(this, QStringLiteral("AvatarWidget"));
}

AvatarWidget::AvatarWidget(const QPixmap &pixmap, QWidget *parent) : AvatarWidget(parent)
{
    setPixmap(pixmap);
    setRadius(m_radius);
}

AvatarWidget::AvatarWidget(const QImage &image, QWidget *parent) : AvatarWidget(parent)
{
    setImage(image);
    setRadius(m_radius);
}

AvatarWidget::AvatarWidget(const QString &imagePath, QWidget *parent) : AvatarWidget(parent)
{
    setImage(imagePath);
}

QString AvatarWidget::text() const { return QLabel::text(); }
int AvatarWidget::radius() const { return m_radius; }
QPixmap AvatarWidget::imagePixmap() const { return ImageLabel::imagePixmap(); }
QColor AvatarWidget::lightBackgroundColor() const { return m_lightBackgroundColor; }
QColor AvatarWidget::darkBackgroundColor() const { return m_darkBackgroundColor; }
QSize AvatarWidget::sizeHint() const { return QSize(m_radius * 2, m_radius * 2); }

void AvatarWidget::setText(const QString &text)
{
    if (QLabel::text() == text) return;
    QLabel::setText(text);
    update();
}

void AvatarWidget::setRadius(int radius)
{
    const int boundedRadius = qMax(1, radius);
    const bool changed = m_radius != boundedRadius;
    m_radius = boundedRadius;
    setFixedSize(sizeHint());

    QFont avatarFont = font();
    avatarFont.setPixelSize(qMax(10, m_radius));
    avatarFont.setWeight(QFont::DemiBold);
    setFont(avatarFont);

    updateGeometry();
    update();
    if (changed) {
        emit radiusChanged(m_radius);
    }
}

void AvatarWidget::setImage(const QImage &image)
{
    clearMovie();
    clearNetworkReply();
    ImageLabel::setImage(image);
    setFixedSize(sizeHint());
}

bool AvatarWidget::setImage(const QString &pathOrUrl) { return setImagePath(pathOrUrl); }

bool AvatarWidget::setImageUrl(const QUrl &url)
{
    if (!isRemoteImageUrl(url)) {
        return false;
    }

    clearMovie();
    clearNetworkReply();
    if (!m_networkAccessManager) {
        m_networkAccessManager = new QNetworkAccessManager(this);
    }

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    auto *reply = m_networkAccessManager->get(request);
    m_pendingImageReply = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply, url]() {
        if (reply != m_pendingImageReply) {
            reply->deleteLater();
            return;
        }

        m_pendingImageReply = nullptr;
        const bool hasNetworkError = reply->error() != QNetworkReply::NoError;
        const QByteArray data = hasNetworkError ? QByteArray() : reply->readAll();
        reply->deleteLater();

        QImage loaded;
        if (hasNetworkError || !loaded.loadFromData(data)) {
            emit imageLoadFailed(url);
            return;
        }

        clearMovie();
        ImageLabel::setImage(loaded);
        setFixedSize(sizeHint());
        emit imageLoaded(loaded);
    });

    return true;
}

void AvatarWidget::setPixmap(const QPixmap &pixmap)
{
    clearMovie();
    clearNetworkReply();
    setImage(pixmap.toImage());
}

bool AvatarWidget::setImagePath(const QString &path)
{
    const QUrl url(path);
    if (isRemoteImageUrl(url)) {
        return setImageUrl(url);
    }
    return setMoviePath(path);
}

bool AvatarWidget::setMoviePath(const QString &path)
{
    clearNetworkReply();
    QImageReader reader(path);
    if (!reader.supportsAnimation()) {
        clearMovie();
        const bool loaded = ImageLabel::setImagePath(path);
        setFixedSize(sizeHint());
        return loaded;
    }
    clearMovie();
    auto *movie = new QMovie(path, QByteArray(), this);
    if (!movie->isValid()) {
        movie->deleteLater();
        return false;
    }
    m_movie = movie;
    connect(m_movie, &QMovie::frameChanged, this, &AvatarWidget::onMovieFrameChanged);
    m_movie->setScaledSize(size());
    m_movie->start();
    onMovieFrameChanged(0);
    setFixedSize(sizeHint());
    return true;
}

void AvatarWidget::setGif(const QString &path) { setMoviePath(path); }
QMovie *AvatarWidget::movie() const { return m_movie; }

void AvatarWidget::setBackgroundColor(const QColor &light, const QColor &dark)
{
    m_lightBackgroundColor = light;
    m_darkBackgroundColor = dark;
    update();
}

void AvatarWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    if (!isNull()) {
        drawImageAvatar(&painter);
        return;
    }
    drawTextAvatar(&painter);
}

void AvatarWidget::drawImageAvatar(QPainter *painter)
{
    const qreal dpr = devicePixelRatioF();
    QImage scaled = image().scaled(size() * dpr, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    const int d = int(m_radius * 2 * dpr);
    const int iw = scaled.width();
    const int ih = scaled.height();
    const int x = (iw - d) / 2;
    const int y = (ih - d) / 2;
    scaled = scaled.copy(x, y, d, d);

    QPainterPath clipPath;
    clipPath.addEllipse(QRectF(rect()));
    painter->setClipPath(clipPath);
    painter->setPen(Qt::NoPen);
    painter->drawImage(rect(), scaled);
}

void AvatarWidget::drawTextAvatar(QPainter *painter)
{
    if (text().isEmpty()) return;

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    painter->setPen(Qt::NoPen);
    painter->setBrush(dark ? m_darkBackgroundColor : m_lightBackgroundColor);
    painter->drawEllipse(QRectF(rect()));

    painter->setFont(font());
    painter->setPen(dark ? QColor(Qt::white) : QColor(Qt::black));
    painter->drawText(rect(), Qt::AlignCenter, text().left(1).toUpper());
}

void AvatarWidget::clearMovie()
{
    if (!m_movie) return;
    m_movie->stop();
    m_movie->deleteLater();
    m_movie = nullptr;
}

void AvatarWidget::clearNetworkReply()
{
    if (!m_pendingImageReply) return;
    disconnect(m_pendingImageReply, nullptr, this, nullptr);
    m_pendingImageReply->abort();
    m_pendingImageReply->deleteLater();
    m_pendingImageReply = nullptr;
}

void AvatarWidget::onMovieFrameChanged(int)
{
    if (!m_movie) return;
    ImageLabel::setImage(m_movie->currentImage());
    update();
}

// PixmapLabel
PixmapLabel::PixmapLabel(QWidget *parent) : QLabel(parent) {}

QPixmap PixmapLabel::pixmap() const { return m_pixmap; }

void PixmapLabel::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    setFixedSize(pixmap.size());
    update();
}

void PixmapLabel::paintEvent(QPaintEvent *event)
{
    if (m_pixmap.isNull()) {
        QLabel::paintEvent(event);
        return;
    }
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);
    painter.drawPixmap(rect(), m_pixmap);
}

// HyperlinkLabel
HyperlinkLabel::HyperlinkLabel(QWidget *parent) : QPushButton(parent)
{
    setCursor(Qt::PointingHandCursor);
    setFlat(true);
    QFont f = font();
    f.setPixelSize(14);
    setFont(f);
    FluentStyleSheet::setRole(this, QStringLiteral("HyperlinkLabel"));
    setProperty("underline", m_underlineVisible);
    connect(this, &QPushButton::clicked, this, &HyperlinkLabel::onLinkClicked);
}

HyperlinkLabel::HyperlinkLabel(const QString &text, QWidget *parent) : HyperlinkLabel(parent) { setText(text); }

HyperlinkLabel::HyperlinkLabel(const QUrl &url, const QString &text, QWidget *parent) : HyperlinkLabel(parent)
{
    setText(text);
    m_url = url;
}

QUrl HyperlinkLabel::url() const { return m_url; }
bool HyperlinkLabel::underlineVisible() const { return m_underlineVisible; }

void HyperlinkLabel::setUrl(const QUrl &url) { m_url = url; }

void HyperlinkLabel::setUrl(const QString &url) { setUrl(QUrl(url)); }

void HyperlinkLabel::setUnderlineVisible(bool visible)
{
    m_underlineVisible = visible;
    setProperty("underline", visible);
    QFont f = font();
    f.setUnderline(visible);
    setFont(f);
    FluentStyleSheet::polish(this);
}

void HyperlinkLabel::onLinkClicked()
{
    if (m_url.isValid()) {
        QDesktopServices::openUrl(m_url);
    }
}

} // namespace FluentQt
