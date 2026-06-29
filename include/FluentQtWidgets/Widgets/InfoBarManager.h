#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QSize>

class QPropertyAnimation;
class QParallelAnimationGroup;

namespace FluentQt {

class InfoBar;

class FQW_API InfoBarManager : public QObject
{
    Q_OBJECT

public:
    explicit InfoBarManager(QObject *parent = nullptr);

    void add(InfoBar *infoBar);
    void remove(InfoBar *infoBar);

    static InfoBarManager *make(InfoBarPosition position);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    virtual QPoint pos(InfoBar *infoBar, const QSize &parentSize = QSize()) const = 0;
    virtual QPoint slideStartPos(InfoBar *infoBar) const = 0;

    QPropertyAnimation *createSlideAni(InfoBar *infoBar);
    void updateDropAni(QWidget *parent);

protected:
    int m_spacing = 16;
    int m_margin = 24;
    QHash<QWidget *, QList<InfoBar *>> m_infoBars;
    QHash<QWidget *, QParallelAnimationGroup *> m_aniGroups;

private:
    QList<QPropertyAnimation *> m_slideAnis;
    QList<QPropertyAnimation *> m_dropAnis;

    static InfoBarManager *s_managers[7]; // indexed by InfoBarPosition
};

class FQW_API TopInfoBarManager : public InfoBarManager
{
    Q_OBJECT
private:
    QPoint pos(InfoBar *infoBar, const QSize &parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar *infoBar) const override;
};

class FQW_API TopRightInfoBarManager : public InfoBarManager
{
    Q_OBJECT
private:
    QPoint pos(InfoBar *infoBar, const QSize &parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar *infoBar) const override;
};

class FQW_API TopLeftInfoBarManager : public InfoBarManager
{
    Q_OBJECT
private:
    QPoint pos(InfoBar *infoBar, const QSize &parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar *infoBar) const override;
};

class FQW_API BottomInfoBarManager : public InfoBarManager
{
    Q_OBJECT
private:
    QPoint pos(InfoBar *infoBar, const QSize &parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar *infoBar) const override;
};

class FQW_API BottomRightInfoBarManager : public InfoBarManager
{
    Q_OBJECT
private:
    QPoint pos(InfoBar *infoBar, const QSize &parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar *infoBar) const override;
};

class FQW_API BottomLeftInfoBarManager : public InfoBarManager
{
    Q_OBJECT
private:
    QPoint pos(InfoBar *infoBar, const QSize &parentSize = QSize()) const override;
    QPoint slideStartPos(InfoBar *infoBar) const override;
};

} // namespace FluentQt
