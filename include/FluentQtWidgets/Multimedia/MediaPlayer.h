#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QUrl>
#include <QtWidgets/QWidget>

class QAudioOutput;
class QHBoxLayout;
class QLabel;
class QMediaPlayer;
class QVideoWidget;

namespace FluentQt {

class BodyLabel;
class Slider;
class TransparentToolButton;

class FQW_API SimpleMediaPlayBar : public QWidget
{
    Q_OBJECT

  public:
    explicit SimpleMediaPlayBar(QWidget *parent = nullptr);

    QMediaPlayer *player() const;
    QAudioOutput *audioOutput() const;
    TransparentToolButton *playButton() const;
    Slider *progressSlider() const;
    BodyLabel *timeLabel() const;

  public slots:
    void setSource(const QUrl &source);
    void play();
    void pause();
    void stop();
    void togglePlay();
    void setPosition(qint64 position);

  protected:
    QHBoxLayout *controlsLayout() const;
    QString formatTime(qint64 milliseconds) const;
    virtual void syncPlaybackState();

    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    TransparentToolButton *m_playButton = nullptr;
    Slider *m_progressSlider = nullptr;
    BodyLabel *m_timeLabel = nullptr;
    QHBoxLayout *m_controlsLayout = nullptr;
    bool m_sliderPressed = false;
};

class FQW_API StandardMediaPlayBar : public SimpleMediaPlayBar
{
    Q_OBJECT

  public:
    explicit StandardMediaPlayBar(QWidget *parent = nullptr);

    TransparentToolButton *volumeButton() const;
    Slider *volumeSlider() const;

  private slots:
    void setVolume(int value);
    void syncMutedIcon();

  private:
    TransparentToolButton *m_volumeButton = nullptr;
    Slider *m_volumeSlider = nullptr;
};

class FQW_API VideoWidget : public QWidget
{
    Q_OBJECT

  public:
    explicit VideoWidget(QWidget *parent = nullptr);

    QMediaPlayer *player() const;
    QAudioOutput *audioOutput() const;
    QVideoWidget *videoOutput() const;
    StandardMediaPlayBar *playBar() const;

  public slots:
    void setVideo(const QUrl &source);
    void play();
    void pause();
    void stop();

  private:
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
    QVideoWidget *m_videoOutput = nullptr;
    StandardMediaPlayBar *m_playBar = nullptr;
};

} // namespace FluentQt
