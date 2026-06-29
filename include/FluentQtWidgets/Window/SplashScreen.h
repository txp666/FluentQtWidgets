#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QSize>
#include <QtGui/QIcon>
#include <QtWidgets/QWidget>

namespace FluentQt {

class FluentTitleBar;
class IconWidget;

class FQW_API SplashScreen : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

  public:
    explicit SplashScreen(const QIcon &icon, QWidget *parent = nullptr, bool enableShadow = true);

    QIcon icon() const;
    QSize iconSize() const;
    FluentTitleBar *titleBar() const;
    IconWidget *iconWidget() const;

  public slots:
    void setIcon(const QIcon &icon);
    void setIconSize(const QSize &size);
    void setTitleBar(FluentTitleBar *titleBar);
    void finish();

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

  private:
    void repositionIcon();

    QIcon m_icon;
    QSize m_iconSize = QSize(96, 96);
    FluentTitleBar *m_titleBar = nullptr;
    IconWidget *m_iconWidget = nullptr;
    bool m_enableShadow = true;
};

} // namespace FluentQt
