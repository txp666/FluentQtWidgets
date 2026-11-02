#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/Flyout.h>

#include <QtCore/QPointer>
#include <QtWidgets/QWidget>

class QGraphicsOpacityEffect;
class QHBoxLayout;
class QPropertyAnimation;
class QVBoxLayout;

namespace FluentQt {

class CaptionLabel;
class MediaPlayerBase;
class Slider;

// A 30x30 transparent tool button with a 16x16 icon and a tool tip shown after a second.
class FQW_API MediaPlayBarButton : public TransparentToolButton
{
    Q_OBJECT

  public:
    explicit MediaPlayBarButton(QWidget *parent = nullptr);
    explicit MediaPlayBarButton(const QIcon &icon, QWidget *parent = nullptr);
    explicit MediaPlayBarButton(FluentIcon icon, QWidget *parent = nullptr);
};

// Play (PlaySolid, "Play") or pause (PauseBold, "Pause"), with a 14x14 icon.
class FQW_API PlayButton : public MediaPlayBarButton
{
    Q_OBJECT

  public:
    explicit PlayButton(QWidget *parent = nullptr);

    void setPlay(bool isPlay);
};

// The volume flyout's content: mute button, volume slider and the volume.
class FQW_API VolumeView : public FlyoutViewBase
{
    Q_OBJECT

  public:
    explicit VolumeView(QWidget *parent = nullptr);

    MediaPlayBarButton *muteButton() const;
    Slider *volumeSlider() const;
    CaptionLabel *volumeLabel() const;

    void setMuted(bool isMute);
    void setVolume(int volume);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    MediaPlayBarButton *m_muteButton = nullptr;
    Slider *m_volumeSlider = nullptr;
    CaptionLabel *m_volumeLabel = nullptr;
};

// Shows the volume flyout above itself when clicked.
class FQW_API VolumeButton : public MediaPlayBarButton
{
    Q_OBJECT

  public:
    explicit VolumeButton(QWidget *parent = nullptr);
    // Deletes the flyout, which lives on the window.
    ~VolumeButton() override;

    VolumeView *volumeView() const;
    Flyout *volumeFlyout() const;
    bool isMuted() const;

  public slots:
    void setMuted(bool isMute);
    void setVolume(int volume);

  signals:
    void volumeChanged(int volume);
    void mutedChanged(bool isMuted);

  private:
    void showVolumeFlyout();

    VolumeView *m_volumeView = nullptr;
    QPointer<Flyout> m_volumeFlyout;
    bool m_isMuted = false;
};

// Play button, volume button and progress slider on a rounded card, connected to a MediaPlayerBase.
class FQW_API MediaPlayBarBase : public QWidget
{
    Q_OBJECT

  public:
    explicit MediaPlayBarBase(QWidget *parent = nullptr);

    // Connects the controls to the player and sets its volume to 30. The bar doesn't take ownership.
    void setMediaPlayer(MediaPlayerBase *player);
    MediaPlayerBase *player() const;

    PlayButton *playButton() const;
    VolumeButton *volumeButton() const;
    Slider *progressSlider() const;
    QGraphicsOpacityEffect *opacityEffect() const;

  public slots:
    // Fade the bar in or out in 250 ms.
    void fadeIn();
    void fadeOut();
    void play();
    void pause();
    void stop();
    void setVolume(int volume);
    void setPosition(qint64 position);
    void togglePlayState();

  protected:
    virtual void onPositionChanged(qint64 position);
    void paintEvent(QPaintEvent *event) override;

  private:
    MediaPlayerBase *m_player = nullptr;
    PlayButton *m_playButton = nullptr;
    VolumeButton *m_volumeButton = nullptr;
    Slider *m_progressSlider = nullptr;
    QGraphicsOpacityEffect *m_opacityEffect = nullptr;
    QPropertyAnimation *m_opacityAni = nullptr;
};

// One row: play button, progress slider, volume button, and the buttons added after it. 48 px high.
class FQW_API SimpleMediaPlayBar : public MediaPlayBarBase
{
    Q_OBJECT

  public:
    explicit SimpleMediaPlayBar(QWidget *parent = nullptr);

    // Adds a button to the right side of the bar.
    void addButton(MediaPlayBarButton *button);
    QHBoxLayout *hBoxLayout() const;

  private:
    QHBoxLayout *m_hBoxLayout = nullptr;
};

// Progress slider, current and remaining time, then the volume button on the left and skip back, play and
// skip forward in the centre. 102 px high.
class FQW_API StandardMediaPlayBar : public MediaPlayBarBase
{
    Q_OBJECT

  public:
    explicit StandardMediaPlayBar(QWidget *parent = nullptr);

    MediaPlayBarButton *skipBackButton() const;
    MediaPlayBarButton *skipForwardButton() const;
    CaptionLabel *currentTimeLabel() const;
    CaptionLabel *remainTimeLabel() const;

    // The three button groups of the bottom row; add your own buttons to them.
    QHBoxLayout *leftButtonLayout() const;
    QHBoxLayout *centerButtonLayout() const;
    QHBoxLayout *rightButtonLayout() const;

  public slots:
    // Back up or fast forward by the given milliseconds (the buttons use 10 s and 30 s).
    void skipBack(qint64 ms);
    void skipForward(qint64 ms);

  protected:
    void onPositionChanged(qint64 position) override;

  private:
    static QString formatTime(qint64 ms);

    QVBoxLayout *m_vBoxLayout = nullptr;
    QHBoxLayout *m_timeLayout = nullptr;
    QHBoxLayout *m_buttonLayout = nullptr;
    QHBoxLayout *m_leftButtonLayout = nullptr;
    QHBoxLayout *m_centerButtonLayout = nullptr;
    QHBoxLayout *m_rightButtonLayout = nullptr;
    MediaPlayBarButton *m_skipBackButton = nullptr;
    MediaPlayBarButton *m_skipForwardButton = nullptr;
    CaptionLabel *m_currentTimeLabel = nullptr;
    CaptionLabel *m_remainTimeLabel = nullptr;
};

} // namespace FluentQt
