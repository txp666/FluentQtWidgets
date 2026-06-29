#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>

#include <QtCore/QSize>
#include <QtGui/QIcon>
#include <QtWidgets/QWidget>

class QPaintEvent;

namespace FluentQt {

class FQW_API IconWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)

  public:
    explicit IconWidget(QWidget *parent = nullptr);
    explicit IconWidget(const QIcon &icon, QWidget *parent = nullptr);
    explicit IconWidget(FluentQt::FluentIcon icon, QWidget *parent = nullptr);

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
    QSize m_iconSize = QSize(20, 20);
};

} // namespace FluentQt
