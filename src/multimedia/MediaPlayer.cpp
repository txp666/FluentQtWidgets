#include <FluentQtWidgets/Multimedia/MediaPlayer.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/Slider.h>

#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QVideoWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

SimpleMediaPlayBar::SimpleMediaPlayBar(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("SimpleMediaPlayBar"));
    FluentStyleSheet::setRole(this, QStringLiteral("MediaPlayer"));
    setMinimumHeight(48);

    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);

    m_playButton = new TransparentToolButton(icon(FluentIcon::Play), this);
    m_playButton->setFixedSize(36, 36);
    m_playButton->setIconSize(QSize(16, 16));

    m_progressSlider = new Slider(Qt::Horizontal, this);
    m_progressSlider->setRange(0, 0);

    m_timeLabel = new BodyLabel(QStringLiteral("00:00 / 00:00"), this);
    m_timeLabel->setMinimumWidth(96);
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_controlsLayout = new QHBoxLayout(this);
    m_controlsLayout->setContentsMargins(8, 6, 8, 6);
    m_controlsLayout->setSpacing(8);
    m_controlsLayout->addWidget(m_playButton);
    m_controlsLayout->addWidget(m_progressSlider, 1);
    m_controlsLayout->addWidget(m_timeLabel);

    connect(m_playButton, &QToolButton::clicked, this, &SimpleMediaPlayBar::togglePlay);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &SimpleMediaPlayBar::syncPlaybackState);
    connect(m_player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        m_progressSlider->setRange(0, static_cast<int>(qMax<qint64>(0, duration)));
        m_timeLabel->setText(QStringLiteral("%1 / %2").arg(formatTime(m_player->position()), formatTime(duration)));
    });
    connect(m_player, &QMediaPlayer::positionChanged, this, [this](qint64 position) {
        if (!m_sliderPressed) {
            m_progressSlider->setValue(static_cast<int>(position));
        }
        m_timeLabel->setText(
            QStringLiteral("%1 / %2").arg(formatTime(position), formatTime(m_player->duration())));
    });
    connect(m_progressSlider, &QSlider::sliderPressed, this, [this]() { m_sliderPressed = true; });
    connect(m_progressSlider, &QSlider::sliderReleased, this, [this]() {
        m_sliderPressed = false;
        setPosition(m_progressSlider->value());
    });
    connect(m_progressSlider, &Slider::clicked, this, [this](int value) { setPosition(value); });
}

QMediaPlayer *SimpleMediaPlayBar::player() const { return m_player; }

QAudioOutput *SimpleMediaPlayBar::audioOutput() const { return m_audioOutput; }

TransparentToolButton *SimpleMediaPlayBar::playButton() const { return m_playButton; }

Slider *SimpleMediaPlayBar::progressSlider() const { return m_progressSlider; }

BodyLabel *SimpleMediaPlayBar::timeLabel() const { return m_timeLabel; }

QHBoxLayout *SimpleMediaPlayBar::controlsLayout() const { return m_controlsLayout; }

void SimpleMediaPlayBar::setSource(const QUrl &source) { m_player->setSource(source); }

void SimpleMediaPlayBar::play() { m_player->play(); }

void SimpleMediaPlayBar::pause() { m_player->pause(); }

void SimpleMediaPlayBar::stop() { m_player->stop(); }

void SimpleMediaPlayBar::togglePlay()
{
    if (m_player->playbackState() == QMediaPlayer::PlayingState) {
        pause();
    } else {
        play();
    }
}

void SimpleMediaPlayBar::setPosition(qint64 position) { m_player->setPosition(position); }

QString SimpleMediaPlayBar::formatTime(qint64 milliseconds) const
{
    const qint64 seconds = qMax<qint64>(0, milliseconds / 1000);
    const qint64 minutes = seconds / 60;
    const qint64 hours = minutes / 60;
    if (hours > 0) {
        return QStringLiteral("%1:%2:%3")
            .arg(hours, 2, 10, QLatin1Char('0'))
            .arg(minutes % 60, 2, 10, QLatin1Char('0'))
            .arg(seconds % 60, 2, 10, QLatin1Char('0'));
    }
    return QStringLiteral("%1:%2").arg(minutes, 2, 10, QLatin1Char('0')).arg(seconds % 60, 2, 10, QLatin1Char('0'));
}

void SimpleMediaPlayBar::syncPlaybackState()
{
    m_playButton->setIcon(icon(m_player->playbackState() == QMediaPlayer::PlayingState ? FluentIcon::Pause
                                                                                         : FluentIcon::Play));
}

StandardMediaPlayBar::StandardMediaPlayBar(QWidget *parent) : SimpleMediaPlayBar(parent)
{
    setObjectName(QStringLiteral("StandardMediaPlayBar"));
    FluentStyleSheet::setRole(this, QStringLiteral("MediaPlayer"));

    m_volumeButton = new TransparentToolButton(icon(FluentIcon::Volume), this);
    m_volumeButton->setFixedSize(36, 36);
    m_volumeButton->setIconSize(QSize(16, 16));
    m_volumeSlider = new Slider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setFixedWidth(96);
    m_volumeSlider->setValue(80);
    m_audioOutput->setVolume(0.8);

    m_controlsLayout->addWidget(m_volumeButton);
    m_controlsLayout->addWidget(m_volumeSlider);

    connect(m_volumeSlider, &QSlider::valueChanged, this, &StandardMediaPlayBar::setVolume);
    connect(m_volumeButton, &QToolButton::clicked, this, [this]() {
        m_audioOutput->setMuted(!m_audioOutput->isMuted());
        syncMutedIcon();
    });
}

TransparentToolButton *StandardMediaPlayBar::volumeButton() const { return m_volumeButton; }

Slider *StandardMediaPlayBar::volumeSlider() const { return m_volumeSlider; }

void StandardMediaPlayBar::setVolume(int value)
{
    m_audioOutput->setVolume(qBound(0.0, value / 100.0, 1.0));
    if (value > 0 && m_audioOutput->isMuted()) {
        m_audioOutput->setMuted(false);
    }
    syncMutedIcon();
}

void StandardMediaPlayBar::syncMutedIcon()
{
    m_volumeButton->setIcon(icon(m_audioOutput->isMuted() || m_volumeSlider->value() == 0 ? FluentIcon::Cancel
                                                                                           : FluentIcon::Volume));
}

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    setObjectName(QStringLiteral("VideoWidget"));
    FluentStyleSheet::setRole(this, QStringLiteral("MediaPlayer"));

    m_videoOutput = new QVideoWidget(this);
    m_playBar = new StandardMediaPlayBar(this);
    m_player = m_playBar->player();
    m_audioOutput = m_playBar->audioOutput();
    m_player->setVideoOutput(m_videoOutput);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(m_videoOutput, 1);
    layout->addWidget(m_playBar);
}

QMediaPlayer *VideoWidget::player() const { return m_player; }

QAudioOutput *VideoWidget::audioOutput() const { return m_audioOutput; }

QVideoWidget *VideoWidget::videoOutput() const { return m_videoOutput; }

StandardMediaPlayBar *VideoWidget::playBar() const { return m_playBar; }

void VideoWidget::setVideo(const QUrl &source) { m_playBar->setSource(source); }

void VideoWidget::play() { m_playBar->play(); }

void VideoWidget::pause() { m_playBar->pause(); }

void VideoWidget::stop() { m_playBar->stop(); }

} // namespace FluentQt
