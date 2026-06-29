#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

class QLabel;
class QEvent;
class QGraphicsOpacityEffect;
class QPropertyAnimation;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QEnterEvent;
#endif

namespace FluentQt {

class FQW_API StateCloseButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(bool isPressed READ isPressed)
    Q_PROPERTY(bool isEnter READ isEnter)

public:
    explicit StateCloseButton(QWidget *parent = nullptr);
    bool isPressed() const;
    bool isEnter() const;

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    bool m_isPressed = false;
    bool m_isEnter = false;
};

class FQW_API StateToolTip : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString content READ content WRITE setContent)
    Q_PROPERTY(bool done READ isDone WRITE setDone NOTIFY doneChanged)
    Q_PROPERTY(int rotateAngle READ rotateAngle WRITE setRotateAngle)
    Q_PROPERTY(int deltaAngle READ deltaAngle WRITE setDeltaAngle)

public:
    StateToolTip(const QString &title, const QString &content, QWidget *parent = nullptr);

    static StateToolTip *showIn(QWidget *parent, const QString &title, const QString &content);

    QString title() const;
    QString content() const;
    bool isDone() const;
    int rotateAngle() const;
    int deltaAngle() const;
    QLabel *titleLabel() const;
    QLabel *contentLabel() const;
    StateCloseButton *closeButton() const;

    QPoint suitablePosition(QWidget *parent) const;

public slots:
    void setTitle(const QString &title);
    void setContent(const QString &content);
    void setState(bool done = false);
    void setDone(bool done = true);
    void setRotateAngle(int angle);
    void setDeltaAngle(int angle);

signals:
    void closedSignal();
    void doneChanged(bool done);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initWidget();
    void initLayout();
    void applyQss();
    void fadeOut();
    void rotateTick();

    QLabel *m_titleLabel = nullptr;
    QLabel *m_contentLabel = nullptr;
    StateCloseButton *m_closeButton = nullptr;
    QGraphicsOpacityEffect *m_opacityEffect = nullptr;
    QPropertyAnimation *m_animation = nullptr;
    QTimer m_rotateTimer;
    bool m_done = false;
    int m_rotateAngle = 0;
    int m_deltaAngle = 20;
};

} // namespace FluentQt
