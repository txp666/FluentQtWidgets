#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSequentialAnimationGroup>
#include <QtGui/QColor>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QWidget>

class QPaintEvent;
class QPainter;

namespace FluentQt {

class FQW_API ProgressBar : public QProgressBar
{
    Q_OBJECT
    Q_PROPERTY(qreal val READ val WRITE setVal)
    Q_PROPERTY(bool useAni READ useAni WRITE setUseAni)
    Q_PROPERTY(QColor lightBarColor READ lightBarColor WRITE setLightBarColor)
    Q_PROPERTY(QColor darkBarColor READ darkBarColor WRITE setDarkBarColor)
    Q_PROPERTY(QColor lightBackgroundColor READ lightBackgroundColor WRITE setLightBackgroundColor)
    Q_PROPERTY(QColor darkBackgroundColor READ darkBackgroundColor WRITE setDarkBackgroundColor)
    Q_PROPERTY(QPropertyAnimation *ani READ ani)

  public:
    explicit ProgressBar(QWidget *parent = nullptr, bool useAni = true);

    QPropertyAnimation *ani() const;
    qreal val() const;
    bool useAni() const;
    bool isPaused() const;
    bool isError() const;
    QColor lightBarColor() const;
    QColor darkBarColor() const;
    QColor lightBackgroundColor() const;
    QColor darkBackgroundColor() const;

  public slots:
    void setVal(qreal v);
    void setUseAni(bool use);
    void setLightBarColor(const QColor &color);
    void setDarkBarColor(const QColor &color);
    void setLightBackgroundColor(const QColor &color);
    void setDarkBackgroundColor(const QColor &color);
    void setCustomBarColor(const QColor &light, const QColor &dark);
    void setCustomBackgroundColor(const QColor &light, const QColor &dark);
    void pause();
    void resume();
    void error();
    void setPaused(bool paused);
    void setError(bool isError);

  protected:
    void paintEvent(QPaintEvent *event) override;

  protected:
    QColor barColor() const;
    QColor backgroundColor() const;
    QString valText() const;

  private slots:
    void onValueChanged(int value);

  private:
    qreal m_val = 0;
    bool m_useAni = true;
    bool m_isPaused = false;
    bool m_isError = false;
    QColor m_lightBackgroundColor = QColor(0, 0, 0, 155);
    QColor m_darkBackgroundColor = QColor(255, 255, 255, 155);
    QColor m_lightBarColor;
    QColor m_darkBarColor;
    QPropertyAnimation *m_animation = nullptr;
};

class FQW_API IndeterminateProgressBar : public QProgressBar
{
    Q_OBJECT
    Q_PROPERTY(qreal shortPos READ shortPos WRITE setShortPos)
    Q_PROPERTY(qreal longPos READ longPos WRITE setLongPos)
    Q_PROPERTY(QColor lightBarColor READ lightBarColor WRITE setLightBarColor)
    Q_PROPERTY(QColor darkBarColor READ darkBarColor WRITE setDarkBarColor)
    Q_PROPERTY(QPropertyAnimation *shortBarAni READ shortBarAni)
    Q_PROPERTY(QPropertyAnimation *longBarAni READ longBarAni)
    Q_PROPERTY(QParallelAnimationGroup *aniGroup READ aniGroup)
    Q_PROPERTY(QSequentialAnimationGroup *longBarAniGroup READ longBarAniGroup)

  public:
    explicit IndeterminateProgressBar(QWidget *parent = nullptr, bool start = true);

    QPropertyAnimation *shortBarAni() const;
    QPropertyAnimation *longBarAni() const;
    QParallelAnimationGroup *aniGroup() const;
    QSequentialAnimationGroup *longBarAniGroup() const;
    qreal shortPos() const;
    qreal longPos() const;
    bool isStarted() const;
    bool isPaused() const;
    bool isError() const;
    QColor lightBarColor() const;
    QColor darkBarColor() const;

  public slots:
    void setShortPos(qreal p);
    void setLongPos(qreal p);
    void setLightBarColor(const QColor &color);
    void setDarkBarColor(const QColor &color);
    void start();
    void stop();
    void pause();
    void resume();
    void error();
    void setPaused(bool paused);
    void setError(bool isError);
    void setCustomBarColor(const QColor &light, const QColor &dark);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QColor barColor() const;

    qreal m_shortPos = 0;
    qreal m_longPos = 0;
    QColor m_lightBarColor;
    QColor m_darkBarColor;
    bool m_isError = false;
    QPropertyAnimation *m_shortBarAni = nullptr;
    QPropertyAnimation *m_longBarAni = nullptr;
    QParallelAnimationGroup *m_aniGroup = nullptr;
    QSequentialAnimationGroup *m_longBarAniGroup = nullptr;
};

class FQW_API ProgressRing : public ProgressBar
{
    Q_OBJECT
    Q_PROPERTY(int strokeWidth READ strokeWidth WRITE setStrokeWidth)

  public:
    explicit ProgressRing(QWidget *parent = nullptr, bool useAni = true);

    int strokeWidth() const;
    QSize sizeHint() const override;

  public slots:
    void setStrokeWidth(int width);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void drawText(QPainter *painter, const QString &text) const;

  private:
    int m_strokeWidth = 6;
};

class FQW_API IndeterminateProgressRing : public QProgressBar
{
    Q_OBJECT
    Q_PROPERTY(int startAngle READ startAngle WRITE setStartAngle)
    Q_PROPERTY(int spanAngle READ spanAngle WRITE setSpanAngle)
    Q_PROPERTY(int strokeWidth READ strokeWidth WRITE setStrokeWidth)
    Q_PROPERTY(QColor lightBarColor READ lightBarColor WRITE setLightBarColor)
    Q_PROPERTY(QColor darkBarColor READ darkBarColor WRITE setDarkBarColor)
    Q_PROPERTY(QColor lightBackgroundColor READ lightBackgroundColor WRITE setLightBackgroundColor)
    Q_PROPERTY(QColor darkBackgroundColor READ darkBackgroundColor WRITE setDarkBackgroundColor)
    Q_PROPERTY(QPropertyAnimation *startAngleAni1 READ startAngleAni1)
    Q_PROPERTY(QPropertyAnimation *startAngleAni2 READ startAngleAni2)
    Q_PROPERTY(QPropertyAnimation *spanAngleAni1 READ spanAngleAni1)
    Q_PROPERTY(QPropertyAnimation *spanAngleAni2 READ spanAngleAni2)
    Q_PROPERTY(QSequentialAnimationGroup *startAngleAniGroup READ startAngleAniGroup)
    Q_PROPERTY(QSequentialAnimationGroup *spanAngleAniGroup READ spanAngleAniGroup)
    Q_PROPERTY(QParallelAnimationGroup *aniGroup READ aniGroup)

  public:
    explicit IndeterminateProgressRing(QWidget *parent = nullptr, bool start = true);

    QPropertyAnimation *startAngleAni1() const;
    QPropertyAnimation *startAngleAni2() const;
    QPropertyAnimation *spanAngleAni1() const;
    QPropertyAnimation *spanAngleAni2() const;
    QSequentialAnimationGroup *startAngleAniGroup() const;
    QSequentialAnimationGroup *spanAngleAniGroup() const;
    QParallelAnimationGroup *aniGroup() const;
    int startAngle() const;
    int spanAngle() const;
    int strokeWidth() const;
    bool isStarted() const;
    QColor lightBarColor() const;
    QColor darkBarColor() const;
    QColor lightBackgroundColor() const;
    QColor darkBackgroundColor() const;

  public slots:
    void setStartAngle(int angle);
    void setSpanAngle(int angle);
    void setStrokeWidth(int width);
    void setLightBarColor(const QColor &color);
    void setDarkBarColor(const QColor &color);
    void setLightBackgroundColor(const QColor &color);
    void setDarkBackgroundColor(const QColor &color);
    void start();
    void stop();
    void setCustomBarColor(const QColor &light, const QColor &dark);
    void setCustomBackgroundColor(const QColor &light, const QColor &dark);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    QColor barColor() const;
    QColor backgroundColor() const;

    QColor m_lightBackgroundColor = QColor(0, 0, 0, 0);
    QColor m_darkBackgroundColor = QColor(255, 255, 255, 0);
    QColor m_lightBarColor;
    QColor m_darkBarColor;
    int m_strokeWidth = 6;
    int m_startAngle = -180;
    int m_spanAngle = 0;
    QPropertyAnimation *m_startAngleAni1 = nullptr;
    QPropertyAnimation *m_startAngleAni2 = nullptr;
    QPropertyAnimation *m_spanAngleAni1 = nullptr;
    QPropertyAnimation *m_spanAngleAni2 = nullptr;
    QSequentialAnimationGroup *m_startAngleAniGroup = nullptr;
    QSequentialAnimationGroup *m_spanAngleAniGroup = nullptr;
    QParallelAnimationGroup *m_aniGroup = nullptr;
};

} // namespace FluentQt
