#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QPoint>
#include <QtCore/QPointer>
#include <QtWidgets/QFrame>

class QLabel;

namespace FluentQt {

class CaptionLabel;
class TransparentToolButton;

class FQW_API FluentTitleBar : public QFrame
{
    Q_OBJECT

  public:
    explicit FluentTitleBar(QWidget *parent = nullptr);

    CaptionLabel *titleLabel() const;
    QLabel *iconLabel() const;
    TransparentToolButton *minimizeButton() const;
    TransparentToolButton *maximizeButton() const;
    TransparentToolButton *closeButton() const;

  public slots:
    void setTitle(const QString &title);
    void setWindowIcon(const QIcon &icon);
    void updateMaximizeButtonState();
    void toggleMaximized();

  protected:
    bool event(QEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    void initLayout();
    void retranslateUi();
    void syncWithWindow();
    bool isCaptionArea(const QPoint &pos) const;

    // The window whose title, icon and state are mirrored; follows reparenting.
    QPointer<QWidget> m_syncedWindow;

    CaptionLabel *m_titleLabel = nullptr;
    QLabel *m_iconLabel = nullptr;
    TransparentToolButton *m_minButton = nullptr;
    TransparentToolButton *m_maxButton = nullptr;
    TransparentToolButton *m_closeButton = nullptr;
    QPoint m_dragPosition;
    bool m_dragging = false;
};

} // namespace FluentQt
