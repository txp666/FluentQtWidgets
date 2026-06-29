#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QStringList>

class QEvent;
class QWidget;

namespace FluentQt {

class InfoBadge;

enum class InfoBadgePosition
{
    TopRight,
    BottomRight,
    Right,
    TopLeft,
    Left,
    BottomLeft,
    NavigationItem
};

class FQW_API InfoBadgeManager : public QObject
{
    Q_OBJECT

  public:
    explicit InfoBadgeManager(QWidget *target, InfoBadge *badge, QObject *parent = nullptr);

    QWidget *target() const;
    InfoBadge *badge() const;

    virtual QPoint position() const;

    static InfoBadgeManager *make(InfoBadgePosition position, QWidget *target, InfoBadge *badge);
    static InfoBadgeManager *make(const QString &name, QWidget *target, InfoBadge *badge);
    static void registerManager(InfoBadgePosition position, const QString &name);
    static QStringList registeredNames();

    static InfoBadge *attach(InfoBadge *badge, QWidget *target, InfoBadgePosition position = InfoBadgePosition::TopRight);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

    QWidget *m_target = nullptr;
    InfoBadge *m_badge = nullptr;
};

class FQW_API TopRightInfoBadgeManager : public InfoBadgeManager
{
  public:
    using InfoBadgeManager::InfoBadgeManager;
    QPoint position() const override;
};

class FQW_API RightInfoBadgeManager : public InfoBadgeManager
{
  public:
    using InfoBadgeManager::InfoBadgeManager;
    QPoint position() const override;
};

class FQW_API BottomRightInfoBadgeManager : public InfoBadgeManager
{
  public:
    using InfoBadgeManager::InfoBadgeManager;
    QPoint position() const override;
};

class FQW_API TopLeftInfoBadgeManager : public InfoBadgeManager
{
  public:
    using InfoBadgeManager::InfoBadgeManager;
    QPoint position() const override;
};

class FQW_API LeftInfoBadgeManager : public InfoBadgeManager
{
  public:
    using InfoBadgeManager::InfoBadgeManager;
    QPoint position() const override;
};

class FQW_API BottomLeftInfoBadgeManager : public InfoBadgeManager
{
  public:
    using InfoBadgeManager::InfoBadgeManager;
    QPoint position() const override;
};

class FQW_API NavigationItemInfoBadgeManager : public InfoBadgeManager
{
  public:
    using InfoBadgeManager::InfoBadgeManager;
    QPoint position() const override;

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::InfoBadgePosition)
