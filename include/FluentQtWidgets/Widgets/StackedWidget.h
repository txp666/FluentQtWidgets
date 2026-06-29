#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QVector>
#include <QtWidgets/QStackedWidget>

class QGraphicsOpacityEffect;
class QLabel;
class QParallelAnimationGroup;
class QPropertyAnimation;
class QWidget;

namespace FluentQt {

class FQW_API OpacityAniStackedWidget : public QStackedWidget
{
    Q_OBJECT

  public:
    explicit OpacityAniStackedWidget(QWidget *parent = nullptr);

    int addWidget(QWidget *widget);
    void insertWidget(int index, QWidget *widget);
    void removeWidget(QWidget *widget);

  public slots:
    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget *widget);

  private slots:
    void onAnimationFinished();

  private:
    void ensureAnimationForWidget(QWidget *widget, int index);

    int m_nextIndex = 0;
    QVector<QGraphicsOpacityEffect *> m_effects;
    QVector<QPropertyAnimation *> m_animations;
};

struct FQW_API PopUpAniInfo
{
    QWidget *widget = nullptr;
    int deltaX = 0;
    int deltaY = 76;
    QPropertyAnimation *animation = nullptr;
};

class FQW_API PopUpAniStackedWidget : public QStackedWidget
{
    Q_OBJECT
    Q_PROPERTY(bool animationEnabled READ isAnimationEnabled WRITE setAnimationEnabled)

  public:
    explicit PopUpAniStackedWidget(QWidget *parent = nullptr);

    int addWidget(QWidget *widget, int deltaX = 0, int deltaY = 76);
    void removeWidget(QWidget *widget);
    bool isAnimationEnabled() const;
    QVector<PopUpAniInfo> animationInfos() const;

  signals:
    void aniFinished();
    void aniStart();

  public slots:
    void setAnimationEnabled(bool enabled);
    void setCurrentIndex(int index, bool needPopOut = false, bool showNextWidgetDirectly = true,
                         int duration = 250, QEasingCurve::Type easingCurve = QEasingCurve::OutQuad);
    void setCurrentWidget(QWidget *widget, bool needPopOut = false, bool showNextWidgetDirectly = true,
                          int duration = 250, QEasingCurve::Type easingCurve = QEasingCurve::OutQuad);

  private slots:
    void onAnimationFinished();

  private:
    void setupAnimation(QPropertyAnimation *animation, const QPoint &startValue, const QPoint &endValue,
                        int duration, QEasingCurve::Type easingCurve);

    QVector<PopUpAniInfo> m_animationInfos;
    bool m_animationEnabled = true;
    int m_nextIndex = -1;
    QPropertyAnimation *m_currentAnimation = nullptr;
};

class FQW_API TransitionStackedWidget : public QStackedWidget
{
    Q_OBJECT
    Q_PROPERTY(bool animationEnabled READ isAnimationEnabled WRITE setAnimationEnabled)

  public:
    explicit TransitionStackedWidget(QWidget *parent = nullptr);

    bool isAnimationEnabled() const;
    int addWidget(QWidget *widget);
    int insertWidget(int index, QWidget *widget);
    QLabel *currentSnapshot() const;
    QLabel *nextSnapshot() const;

  signals:
    void aniFinished();
    void aniStart();

  public slots:
    void setAnimationEnabled(bool enabled);
    void setCurrentIndex(int index, int duration = -1, bool isBack = false);
    void setCurrentWidget(QWidget *widget, int duration = -1, bool isBack = false);

  protected:
    virtual void setupTransitionAnimation(int nextIndex, int duration, bool isBack);
    void stopAnimation();
    void hideSnapshots();
    QLabel *createSnapshotLabel();
    void renderSnapshot(QWidget *widget, QLabel *label);
    QParallelAnimationGroup *animationGroup() const;
    int nextIndex() const;

  private slots:
    void onAnimationFinished();

  private:
    QParallelAnimationGroup *m_animationGroup = nullptr;
    QLabel *m_currentSnapshot = nullptr;
    QLabel *m_nextSnapshot = nullptr;
    int m_nextIndex = -1;
    bool m_animationEnabled = true;
};

class FQW_API EntranceTransitionStackedWidget : public TransitionStackedWidget
{
    Q_OBJECT

  public:
    explicit EntranceTransitionStackedWidget(QWidget *parent = nullptr);

  protected:
    void setupTransitionAnimation(int nextIndex, int duration, bool isBack) override;
};

class FQW_API DrillInTransitionStackedWidget : public TransitionStackedWidget
{
    Q_OBJECT

  public:
    explicit DrillInTransitionStackedWidget(QWidget *parent = nullptr);

  protected:
    void setupTransitionAnimation(int nextIndex, int duration, bool isBack) override;
};

} // namespace FluentQt
