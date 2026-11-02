#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtMultimedia/QMediaPlayer>

class QAudioOutput;

namespace FluentQt {

// The player a media play bar drives (qfluentwidgets.multimedia.MediaPlayerBase). Subclass it to put the
// play bars in front of another playback engine; MediaPlayer is the QMediaPlayer one.
class FQW_API MediaPlayerBase : public QObject
{
    Q_OBJECT

  public:
    explicit MediaPlayerBase(QObject *parent = nullptr);

    // Whether the media is playing.
    virtual bool isPlaying() const = 0;
    virtual QMediaPlayer::MediaStatus mediaStatus() const = 0;
    virtual QMediaPlayer::PlaybackState playbackState() const = 0;
    // The duration and the position of the current media, in ms.
    virtual qint64 duration() const = 0;
    virtual qint64 position() const = 0;
    // 0 to 100.
    virtual int volume() const = 0;
    virtual bool isMuted() const = 0;
    virtual QUrl source() const = 0;
    virtual qreal playbackRate() const = 0;
    virtual QObject *videoOutput() const = 0;

  public slots:
    virtual void pause() = 0;
    virtual void play() = 0;
    // Stops, and goes back to the beginning.
    virtual void stop() = 0;
    virtual void setPosition(qint64 position) = 0;
    virtual void setSource(const QUrl &source) = 0;
    virtual void setPlaybackRate(qreal rate) = 0;
    virtual void setVolume(int volume) = 0;
    virtual void setMuted(bool isMuted) = 0;
    virtual void setVideoOutput(QObject *output) = 0;

  signals:
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void playbackStateChanged(QMediaPlayer::PlaybackState state);
    void playbackRateChanged(qreal rate);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void sourceChanged(const QUrl &source);
    void volumeChanged(int volume);
    void mutedChanged(bool isMuted);
};

// A MediaPlayerBase on a QMediaPlayer and its QAudioOutput. Python's MediaPlayer is a QMediaPlayer subclass;
// in C++ a class can't be both, so this one owns them and hands them out for everything else QMediaPlayer does.
class FQW_API MediaPlayer : public MediaPlayerBase
{
    Q_OBJECT

  public:
    explicit MediaPlayer(QObject *parent = nullptr);

    QMediaPlayer *mediaPlayer() const;
    QAudioOutput *audioOutput() const;

    bool isPlaying() const override;
    QMediaPlayer::MediaStatus mediaStatus() const override;
    QMediaPlayer::PlaybackState playbackState() const override;
    qint64 duration() const override;
    qint64 position() const override;
    int volume() const override;
    bool isMuted() const override;
    QUrl source() const override;
    qreal playbackRate() const override;
    QObject *videoOutput() const override;

  public slots:
    void pause() override;
    void play() override;
    void stop() override;
    void setPosition(qint64 position) override;
    void setSource(const QUrl &source) override;
    void setPlaybackRate(qreal rate) override;
    // Emit volumeChanged / mutedChanged only when the value changes, as Python's do.
    void setVolume(int volume) override;
    void setMuted(bool isMuted) override;
    void setVideoOutput(QObject *output) override;

  private:
    QMediaPlayer *m_player = nullptr;
    QAudioOutput *m_audioOutput = nullptr;
};

} // namespace FluentQt
