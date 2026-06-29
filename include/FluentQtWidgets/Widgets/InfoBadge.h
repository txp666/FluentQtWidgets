#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/InfoBadgeManager.h>

#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtWidgets/QLabel>

class QPaintEvent;

namespace FluentQt {

enum class InfoLevel
{
    Info,
    Success,
    Attention,
    Warning,
    Error
};

class FQW_API InfoBadge : public QLabel
{
    Q_OBJECT
    Q_PROPERTY(QColor lightBackgroundColor READ lightBackgroundColor WRITE setLightBackgroundColor)
    Q_PROPERTY(QColor darkBackgroundColor READ darkBackgroundColor WRITE setDarkBackgroundColor)

  public:
    explicit InfoBadge(QWidget *parent = nullptr);
    explicit InfoBadge(const QString &text, InfoLevel level = InfoLevel::Attention, QWidget *parent = nullptr);

    static InfoBadge *make(const QString &text, QWidget *parent = nullptr, InfoLevel level = InfoLevel::Info,
                           QWidget *target = nullptr,
                           InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *create(const QString &text, InfoLevel level, QWidget *parent = nullptr,
                             QWidget *target = nullptr,
                             InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *info(const QString &text, QWidget *parent = nullptr, QWidget *target = nullptr,
                           InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *info(int count, QWidget *parent = nullptr, QWidget *target = nullptr,
                           InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *success(const QString &text, QWidget *parent = nullptr, QWidget *target = nullptr,
                              InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *success(int count, QWidget *parent = nullptr, QWidget *target = nullptr,
                              InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *attention(const QString &text, QWidget *parent = nullptr, QWidget *target = nullptr,
                                InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *attention(int count, QWidget *parent = nullptr, QWidget *target = nullptr,
                                InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *attension(const QString &text, QWidget *parent = nullptr, QWidget *target = nullptr,
                                InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *attension(int count, QWidget *parent = nullptr, QWidget *target = nullptr,
                                InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *warning(const QString &text, QWidget *parent = nullptr, QWidget *target = nullptr,
                              InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *warning(int count, QWidget *parent = nullptr, QWidget *target = nullptr,
                              InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *error(const QString &text, QWidget *parent = nullptr, QWidget *target = nullptr,
                            InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *error(int count, QWidget *parent = nullptr, QWidget *target = nullptr,
                            InfoBadgePosition position = InfoBadgePosition::TopRight);
    static InfoBadge *custom(const QString &text, const QColor &lightBackground, const QColor &darkBackground = QColor(),
                             QWidget *parent = nullptr, QWidget *target = nullptr,
                             InfoBadgePosition position = InfoBadgePosition::TopRight);

    InfoLevel level() const;
    QColor lightBackgroundColor() const;
    QColor darkBackgroundColor() const;
    InfoBadgeManager *manager() const;
    QSize sizeHint() const override;
    void setCount(int count);
    void setCustomBackgroundColor(const QColor &lightBackground, const QColor &darkBackground = QColor());

  public slots:
    void setLevel(FluentQt::InfoLevel level);
    void setLightBackgroundColor(const QColor &color);
    void setDarkBackgroundColor(const QColor &color);
    void setManager(FluentQt::InfoBadgeManager *manager);

  signals:
    void levelChanged(FluentQt::InfoLevel level);

  protected:
    void paintEvent(QPaintEvent *event) override;
    QColor backgroundColor() const;

  private:
    QString levelToken() const;
    void updateRole();

    InfoLevel m_level = InfoLevel::Attention;
    QColor m_lightBackgroundColor;
    QColor m_darkBackgroundColor;
    InfoBadgeManager *m_manager = nullptr;
};

class FQW_API DotInfoBadge : public InfoBadge
{
    Q_OBJECT

  public:
    explicit DotInfoBadge(InfoLevel level = InfoLevel::Attention, QWidget *parent = nullptr);

    static DotInfoBadge *create(InfoLevel level, QWidget *parent = nullptr, QWidget *target = nullptr,
                                InfoBadgePosition position = InfoBadgePosition::TopRight);
    static DotInfoBadge *info(QWidget *parent = nullptr, QWidget *target = nullptr,
                              InfoBadgePosition position = InfoBadgePosition::TopRight);
    static DotInfoBadge *success(QWidget *parent = nullptr, QWidget *target = nullptr,
                                 InfoBadgePosition position = InfoBadgePosition::TopRight);
    static DotInfoBadge *attention(QWidget *parent = nullptr, QWidget *target = nullptr,
                                   InfoBadgePosition position = InfoBadgePosition::TopRight);
    static DotInfoBadge *attension(QWidget *parent = nullptr, QWidget *target = nullptr,
                                   InfoBadgePosition position = InfoBadgePosition::TopRight);
    static DotInfoBadge *warning(QWidget *parent = nullptr, QWidget *target = nullptr,
                                 InfoBadgePosition position = InfoBadgePosition::TopRight);
    static DotInfoBadge *error(QWidget *parent = nullptr, QWidget *target = nullptr,
                               InfoBadgePosition position = InfoBadgePosition::TopRight);
    static DotInfoBadge *custom(const QColor &lightBackground, const QColor &darkBackground = QColor(),
                                QWidget *parent = nullptr, QWidget *target = nullptr,
                                InfoBadgePosition position = InfoBadgePosition::TopRight);

  protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API IconInfoBadge : public InfoBadge
{
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

  public:
    explicit IconInfoBadge(QWidget *parent = nullptr);
    explicit IconInfoBadge(const QIcon &icon, InfoLevel level = InfoLevel::Attention, QWidget *parent = nullptr);
    explicit IconInfoBadge(FluentQt::FluentIcon icon, InfoLevel level = InfoLevel::Attention,
                           QWidget *parent = nullptr);

    static IconInfoBadge *create(const QIcon &icon, InfoLevel level, QWidget *parent = nullptr,
                                 QWidget *target = nullptr,
                                 InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *create(FluentQt::FluentIcon icon, InfoLevel level, QWidget *parent = nullptr,
                                 QWidget *target = nullptr,
                                 InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *info(const QIcon &icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                               InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *info(FluentQt::FluentIcon icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                               InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *success(const QIcon &icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                  InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *success(FluentQt::FluentIcon icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                  InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *attention(const QIcon &icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                    InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *attention(FluentQt::FluentIcon icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                    InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *attension(const QIcon &icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                    InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *attension(FluentQt::FluentIcon icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                    InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *warning(const QIcon &icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                  InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *warning(FluentQt::FluentIcon icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                  InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *error(const QIcon &icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *error(FluentQt::FluentIcon icon, QWidget *parent = nullptr, QWidget *target = nullptr,
                                InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *custom(const QIcon &icon, const QColor &lightBackground, const QColor &darkBackground = QColor(),
                                 QWidget *parent = nullptr, QWidget *target = nullptr,
                                 InfoBadgePosition position = InfoBadgePosition::TopRight);
    static IconInfoBadge *custom(FluentQt::FluentIcon icon, const QColor &lightBackground,
                                 const QColor &darkBackground = QColor(), QWidget *parent = nullptr,
                                 QWidget *target = nullptr,
                                 InfoBadgePosition position = InfoBadgePosition::TopRight);

    QIcon icon() const;
    QSize iconSize() const;
    QSize sizeHint() const override;

  public slots:
    void setIcon(const QIcon &icon);
    void setIcon(FluentQt::FluentIcon icon);
    void setIconSize(const QSize &size);

  signals:
    void iconChanged(const QIcon &icon);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QIcon m_icon;
    QSize m_iconSize = QSize(8, 8);
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::InfoLevel)
