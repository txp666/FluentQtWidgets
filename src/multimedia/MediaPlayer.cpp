#include <FluentQtWidgets/Multimedia/MediaPlayer.h>

#include <QtMultimedia/QAudioOutput>

#include <cmath>

namespace FluentQt {

MediaPlayerBase::MediaPlayerBase(QObject *parent) : QObject(parent) {}

MediaPlayer::MediaPlayer(QObject *parent) : MediaPlayerBase(parent)
{
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);

    connect(m_player, &QMediaPlayer::mediaStatusChanged, this, &MediaPlayerBase::mediaStatusChanged);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &MediaPlayerBase::playbackStateChanged);
    connect(m_player, &QMediaPlayer::playbackRateChanged, this, &MediaPlayerBase::playbackRateChanged);
    connect(m_player, &QMediaPlayer::positionChanged, this, &MediaPlayerBase::positionChanged);
    connect(m_player, &QMediaPlayer::durationChanged, this, &MediaPlayerBase::durationChanged);
    connect(m_player, &QMediaPlayer::sourceChanged, this, &MediaPlayerBase::sourceChanged);
}

QMediaPlayer *MediaPlayer::mediaPlayer() const { return m_player; }

QAudioOutput *MediaPlayer::audioOutput() const { return m_audioOutput; }

bool MediaPlayer::isPlaying() const { return m_player->playbackState() == QMediaPlayer::PlayingState; }

QMediaPlayer::MediaStatus MediaPlayer::mediaStatus() const { return m_player->mediaStatus(); }

QMediaPlayer::PlaybackState MediaPlayer::playbackState() const { return m_player->playbackState(); }

qint64 MediaPlayer::duration() const { return m_player->duration(); }

qint64 MediaPlayer::position() const { return m_player->position(); }

int MediaPlayer::volume() const { return static_cast<int>(std::lround(m_audioOutput->volume() * 100)); }

bool MediaPlayer::isMuted() const { return m_audioOutput->isMuted(); }

QUrl MediaPlayer::source() const { return m_player->source(); }

qreal MediaPlayer::playbackRate() const { return m_player->playbackRate(); }

QObject *MediaPlayer::videoOutput() const { return m_player->videoOutput(); }

void MediaPlayer::pause() { m_player->pause(); }

void MediaPlayer::play() { m_player->play(); }

void MediaPlayer::stop() { m_player->stop(); }

void MediaPlayer::setPosition(qint64 position) { m_player->setPosition(position); }

void MediaPlayer::setSource(const QUrl &source) { m_player->setSource(source); }

void MediaPlayer::setPlaybackRate(qreal rate) { m_player->setPlaybackRate(rate); }

void MediaPlayer::setVolume(int volume)
{
    if (volume == this->volume()) {
        return;
    }

    m_audioOutput->setVolume(volume / 100.0);
    emit volumeChanged(volume);
}

void MediaPlayer::setMuted(bool isMuted)
{
    if (isMuted == m_audioOutput->isMuted()) {
        return;
    }

    m_audioOutput->setMuted(isMuted);
    emit mutedChanged(isMuted);
}

void MediaPlayer::setVideoOutput(QObject *output) { m_player->setVideoOutput(output); }

} // namespace FluentQt
