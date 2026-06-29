#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QSize>
#include <QtCore/QUrl>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

class QMovie;
class QMouseEvent;
class QNetworkAccessManager;
class QNetworkReply;
class QPainter;
class QPaintEvent;

namespace FluentQt {

class FQW_API FluentLabelBase : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QColor lightColor READ lightColor WRITE setLightColor)
    Q_PROPERTY(QColor darkColor READ darkColor WRITE setDarkColor)
    Q_PROPERTY(int pixelFontSize READ pixelFontSize WRITE setPixelFontSize)
    Q_PROPERTY(bool strikeOut READ strikeOut WRITE setStrikeOut)
    Q_PROPERTY(bool underline READ underline WRITE setUnderline)

  public:
    explicit FluentLabelBase(QWidget *parent = nullptr);
    explicit FluentLabelBase(const QString &text, QWidget *parent = nullptr);

    QColor lightColor() const;
    QColor darkColor() const;
    int pixelFontSize() const;
    bool strikeOut() const;
    bool underline() const;

  public slots:
    void setLightColor(const QColor &color);
    void setDarkColor(const QColor &color);
    void setTextColor(const QColor &light, const QColor &dark);
    void setPixelFontSize(int size);
    void setStrikeOut(bool strikeOut);
    void setUnderline(bool underline);

  protected:
    void setFluentTextStyle(const QString &role, int pixelSize, QFont::Weight weight = QFont::Normal);
    void contextMenuEvent(QContextMenuEvent *event) override;

  private:
    void updateTextColor();
    QColor m_lightColor;
    QColor m_darkColor;
};

class FQW_API CaptionLabel : public FluentLabelBase
{
  public:
    explicit CaptionLabel(QWidget *parent = nullptr);
    explicit CaptionLabel(const QString &text, QWidget *parent = nullptr);
};

class FQW_API BodyLabel : public FluentLabelBase
{
  public:
    explicit BodyLabel(QWidget *parent = nullptr);
    explicit BodyLabel(const QString &text, QWidget *parent = nullptr);
};

class FQW_API StrongBodyLabel : public FluentLabelBase
{
  public:
    explicit StrongBodyLabel(QWidget *parent = nullptr);
    explicit StrongBodyLabel(const QString &text, QWidget *parent = nullptr);
};

class FQW_API SubtitleLabel : public FluentLabelBase
{
  public:
    explicit SubtitleLabel(QWidget *parent = nullptr);
    explicit SubtitleLabel(const QString &text, QWidget *parent = nullptr);
};

class FQW_API TitleLabel : public FluentLabelBase
{
  public:
    explicit TitleLabel(QWidget *parent = nullptr);
    explicit TitleLabel(const QString &text, QWidget *parent = nullptr);
};

class FQW_API LargeTitleLabel : public FluentLabelBase
{
  public:
    explicit LargeTitleLabel(QWidget *parent = nullptr);
    explicit LargeTitleLabel(const QString &text, QWidget *parent = nullptr);
};

class FQW_API DisplayLabel : public FluentLabelBase
{
  public:
    explicit DisplayLabel(QWidget *parent = nullptr);
    explicit DisplayLabel(const QString &text, QWidget *parent = nullptr);
};

class FQW_API ImageLabel : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
    Q_PROPERTY(int topLeftRadius READ topLeftRadius WRITE setTopLeftRadius)
    Q_PROPERTY(int topRightRadius READ topRightRadius WRITE setTopRightRadius)
    Q_PROPERTY(int bottomLeftRadius READ bottomLeftRadius WRITE setBottomLeftRadius)
    Q_PROPERTY(int bottomRightRadius READ bottomRightRadius WRITE setBottomRightRadius)

  public:
    explicit ImageLabel(QWidget *parent = nullptr);
    explicit ImageLabel(const QString &path, QWidget *parent = nullptr);
    explicit ImageLabel(const QPixmap &pixmap, QWidget *parent = nullptr);
    explicit ImageLabel(const QImage &image, QWidget *parent = nullptr);

    QImage image() const;
    QPixmap imagePixmap() const;
    bool isNull() const;
    int borderRadius() const;
    int topLeftRadius() const;
    int topRightRadius() const;
    int bottomLeftRadius() const;
    int bottomRightRadius() const;

  public slots:
    void setImage(const QImage &image);
    void setPixmap(const QPixmap &pixmap);
    bool setImagePath(const QString &path);
    void setBorderRadius(int topLeft, int topRight, int bottomLeft, int bottomRight);
    void setBorderRadius(int radius);

    void setTopLeftRadius(int radius);
    void setTopRightRadius(int radius);
    void setBottomLeftRadius(int radius);
    void setBottomRightRadius(int radius);

    void scaledToWidth(int width);
    void scaledToHeight(int height);
    void setScaledSize(const QSize &size);
    void setMovie(QMovie *movie);

  signals:
    void clicked();

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    void onMovieFrameChanged(int frame);

    QImage m_image;
    int m_topLeftRadius = 0;
    int m_topRightRadius = 0;
    int m_bottomLeftRadius = 0;
    int m_bottomRightRadius = 0;
    QMovie *m_movie = nullptr;
};

class FQW_API AvatarWidget : public ImageLabel
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(int radius READ radius WRITE setRadius NOTIFY radiusChanged)

  public:
    explicit AvatarWidget(QWidget *parent = nullptr);
    explicit AvatarWidget(const QPixmap &pixmap, QWidget *parent = nullptr);
    explicit AvatarWidget(const QImage &image, QWidget *parent = nullptr);
    explicit AvatarWidget(const QString &imagePath, QWidget *parent = nullptr);

    QString text() const;
    int radius() const;
    QPixmap imagePixmap() const;
    QColor lightBackgroundColor() const;
    QColor darkBackgroundColor() const;
    QSize sizeHint() const override;

  public slots:
    void setText(const QString &text);
    void setRadius(int radius);
    void setImage(const QImage &image);
    bool setImage(const QString &pathOrUrl);
    bool setImageUrl(const QUrl &url);
    void setPixmap(const QPixmap &pixmap);
    bool setImagePath(const QString &path);
    void setBackgroundColor(const QColor &light, const QColor &dark);
    bool setMoviePath(const QString &path);
    void setGif(const QString &path);
    QMovie *movie() const;

  signals:
    void radiusChanged(int radius);
    void imageLoaded(const QImage &image);
    void imageLoadFailed(const QUrl &url);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    void drawImageAvatar(QPainter *painter);
    void drawTextAvatar(QPainter *painter);
    void clearMovie();
    void clearNetworkReply();
    void onMovieFrameChanged(int frame);

    int m_radius = 0;
    QColor m_lightBackgroundColor = QColor(0, 0, 0, 50);
    QColor m_darkBackgroundColor = QColor(255, 255, 255, 50);
    QMovie *m_movie = nullptr;
    QNetworkAccessManager *m_networkAccessManager = nullptr;
    QNetworkReply *m_pendingImageReply = nullptr;
};

class FQW_API PixmapLabel : public QLabel
{
    Q_OBJECT

  public:
    explicit PixmapLabel(QWidget *parent = nullptr);

    QPixmap pixmap() const;

  public slots:
    void setPixmap(const QPixmap &pixmap);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QPixmap m_pixmap;
};

class FQW_API HyperlinkLabel : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl)
    Q_PROPERTY(bool underlineVisible READ underlineVisible WRITE setUnderlineVisible)

  public:
    explicit HyperlinkLabel(QWidget *parent = nullptr);
    explicit HyperlinkLabel(const QString &text, QWidget *parent = nullptr);
    HyperlinkLabel(const QUrl &url, const QString &text, QWidget *parent = nullptr);

    QUrl url() const;
    bool underlineVisible() const;

  public slots:
    void setUrl(const QUrl &url);
    void setUrl(const QString &url);
    void setUnderlineVisible(bool visible);

  private:
    void onLinkClicked();

    QUrl m_url;
    bool m_underlineVisible = false;
};

} // namespace FluentQt
