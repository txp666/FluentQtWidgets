#include <FluentQtWidgets/Multimedia/MediaPlayBar.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Multimedia/MediaPlayer.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/Slider.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>

#include <QtCore/QPropertyAnimation>
#include <QtGui/QPainter>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

namespace {

// The rounded card behind the play bars and the volume view.
void paintCard(QWidget *widget)
{
    QPainter painter(widget);
    painter.setRenderHints(QPainter::Antialiasing);

    if (ThemeManager::instance()->effectiveTheme() == Theme::Dark) {
        painter.setBrush(QColor(46, 46, 46));
        painter.setPen(QColor(0, 0, 0, 20));
    } else {
        painter.setBrush(QColor(248, 248, 248));
        painter.setPen(QColor(0, 0, 0, 10));
    }

    painter.drawRoundedRect(QRectF(widget->rect().adjusted(1, 1, -1, -1)), 8, 8);
}

void initPlayBarButton(MediaPlayBarButton *button)
{
    button->installEventFilter(new ToolTipFilter(button, 1000));
    button->setFixedSize(30, 30);
    button->setIconSize(QSize(16, 16));
}

} // namespace

// ==========================================
// MediaPlayBarButton
// ==========================================

MediaPlayBarButton::MediaPlayBarButton(QWidget *parent) : TransparentToolButton(parent) { initPlayBarButton(this); }

MediaPlayBarButton::MediaPlayBarButton(const QIcon &icon, QWidget *parent) : TransparentToolButton(icon, parent)
{
    initPlayBarButton(this);
}

MediaPlayBarButton::MediaPlayBarButton(FluentIcon icon, QWidget *parent)
    : MediaPlayBarButton(FluentQt::icon(icon), parent)
{
}

// ==========================================
// PlayButton
// ==========================================

PlayButton::PlayButton(QWidget *parent) : MediaPlayBarButton(parent)
{
    setIconSize(QSize(14, 14));
    setPlay(false);
}

void PlayButton::setPlay(bool isPlay)
{
    if (isPlay) {
        setIcon(FluentQt::icon(FluentIcon::PauseBold));
        setToolTip(tr("Pause"));
    } else {
        setIcon(FluentQt::icon(FluentIcon::PlaySolid));
        setToolTip(tr("Play"));
    }
}

// ==========================================
// VolumeView
// ==========================================

VolumeView::VolumeView(QWidget *parent) : FlyoutViewBase(parent)
{
    m_muteButton = new MediaPlayBarButton(FluentIcon::Volume, this);
    m_volumeSlider = new Slider(Qt::Horizontal, this);
    m_volumeLabel = new CaptionLabel(QStringLiteral("30"), this);

    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setFixedWidth(208);
    setFixedSize(295, 64);

    const int h = height();
    m_muteButton->move(10, h / 2 - m_muteButton->height() / 2);
    m_volumeSlider->move(45, 21);
}

MediaPlayBarButton *VolumeView::muteButton() const { return m_muteButton; }

Slider *VolumeView::volumeSlider() const { return m_volumeSlider; }

CaptionLabel *VolumeView::volumeLabel() const { return m_volumeLabel; }

void VolumeView::setMuted(bool isMute)
{
    if (isMute) {
        m_muteButton->setIcon(FluentQt::icon(FluentIcon::Mute));
        m_muteButton->setToolTip(tr("Unmute"));
    } else {
        m_muteButton->setIcon(FluentQt::icon(FluentIcon::Volume));
        m_muteButton->setToolTip(tr("Mute"));
    }
}

void VolumeView::setVolume(int volume)
{
    m_volumeSlider->setValue(volume);

    m_volumeLabel->setNum(volume);
    m_volumeLabel->adjustSize();

    const QRect tr = m_volumeLabel->fontMetrics().boundingRect(QString::number(volume));
    m_volumeLabel->move(width() - 20 - tr.width(), height() / 2 - tr.height() / 2);
}

void VolumeView::paintEvent(QPaintEvent *) { paintCard(this); }

// ==========================================
// VolumeButton
// ==========================================

VolumeButton::VolumeButton(QWidget *parent) : MediaPlayBarButton(parent)
{
    m_volumeView = new VolumeView(this);
    m_volumeFlyout = new Flyout(m_volumeView, window(), false);
    m_volumeFlyout->setAnimationType(FlyoutAnimationType::PullUp);
    setMuted(false);

    m_volumeFlyout->hide();
    connect(m_volumeView->muteButton(), &QAbstractButton::clicked, this, [this]() { emit mutedChanged(!m_isMuted); });
    connect(m_volumeView->volumeSlider(), &QSlider::valueChanged, this, &VolumeButton::volumeChanged);
    connect(this, &QAbstractButton::clicked, this, &VolumeButton::showVolumeFlyout);
}

VolumeButton::~VolumeButton() { delete m_volumeFlyout; }

VolumeView *VolumeButton::volumeView() const { return m_volumeView; }

Flyout *VolumeButton::volumeFlyout() const { return m_volumeFlyout; }

bool VolumeButton::isMuted() const { return m_isMuted; }

void VolumeButton::setMuted(bool isMute)
{
    m_isMuted = isMute;
    m_volumeView->setMuted(isMute);
    setIcon(FluentQt::icon(isMute ? FluentIcon::Mute : FluentIcon::Volume));
}

void VolumeButton::setVolume(int volume) { m_volumeView->setVolume(volume); }

void VolumeButton::showVolumeFlyout()
{
    if (m_volumeFlyout->isVisible()) {
        return;
    }

    m_volumeFlyout->exec(this);
}

// ==========================================
// MediaPlayBarBase
// ==========================================

MediaPlayBarBase::MediaPlayBarBase(QWidget *parent) : QWidget(parent)
{
    m_playButton = new PlayButton(this);
    m_volumeButton = new VolumeButton(this);
    m_progressSlider = new Slider(Qt::Horizontal, this);

    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityAni = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_opacityEffect->setOpacity(1);
    m_opacityAni->setDuration(250);

    setGraphicsEffect(m_opacityEffect);
    FluentStyleSheet::setRole(this, QStringLiteral("MediaPlayer"));

    connect(m_playButton, &QAbstractButton::clicked, this, &MediaPlayBarBase::togglePlayState);
}

void MediaPlayBarBase::setMediaPlayer(MediaPlayerBase *player)
{
    if (m_player) {
        disconnect(m_player, nullptr, this, nullptr);
        disconnect(m_player, nullptr, m_progressSlider, nullptr);
        disconnect(m_player, nullptr, m_volumeButton, nullptr);
        disconnect(m_progressSlider, nullptr, m_player, nullptr);
        disconnect(m_volumeButton, nullptr, m_player, nullptr);
    }

    m_player = player;
    if (!m_player) {
        return;
    }

    connect(m_player, &MediaPlayerBase::durationChanged, m_progressSlider,
            [this](qint64 duration) { m_progressSlider->setMaximum(static_cast<int>(duration)); });
    connect(m_player, &MediaPlayerBase::positionChanged, this, &MediaPlayBarBase::onPositionChanged);
    connect(m_player, &MediaPlayerBase::mediaStatusChanged, this,
            [this]() { m_playButton->setPlay(m_player->isPlaying()); });
    connect(m_player, &MediaPlayerBase::volumeChanged, m_volumeButton, &VolumeButton::setVolume);
    connect(m_player, &MediaPlayerBase::mutedChanged, m_volumeButton, &VolumeButton::setMuted);

    // Not valueChanged: it follows the playback too, and seeking on it would interrupt the sound.
    connect(m_progressSlider, &QSlider::sliderMoved, m_player, [this](int value) { m_player->setPosition(value); });
    connect(m_progressSlider, &Slider::clicked, m_player, [this](int value) { m_player->setPosition(value); });
    connect(m_volumeButton, &VolumeButton::volumeChanged, m_player, &MediaPlayerBase::setVolume);
    connect(m_volumeButton, &VolumeButton::mutedChanged, m_player, &MediaPlayerBase::setMuted);

    m_player->setVolume(30);
}

MediaPlayerBase *MediaPlayBarBase::player() const { return m_player; }

PlayButton *MediaPlayBarBase::playButton() const { return m_playButton; }

VolumeButton *MediaPlayBarBase::volumeButton() const { return m_volumeButton; }

Slider *MediaPlayBarBase::progressSlider() const { return m_progressSlider; }

QGraphicsOpacityEffect *MediaPlayBarBase::opacityEffect() const { return m_opacityEffect; }

void MediaPlayBarBase::fadeIn()
{
    m_opacityAni->stop();
    m_opacityAni->setStartValue(m_opacityEffect->opacity());
    m_opacityAni->setEndValue(1.0);
    m_opacityAni->start();
}

void MediaPlayBarBase::fadeOut()
{
    m_opacityAni->stop();
    m_opacityAni->setStartValue(m_opacityEffect->opacity());
    m_opacityAni->setEndValue(0.0);
    m_opacityAni->start();
}

void MediaPlayBarBase::play()
{
    if (m_player) {
        m_player->play();
    }
}

void MediaPlayBarBase::pause()
{
    if (m_player) {
        m_player->pause();
    }
}

void MediaPlayBarBase::stop()
{
    if (m_player) {
        m_player->stop();
    }
}

void MediaPlayBarBase::setVolume(int volume)
{
    if (m_player) {
        m_player->setVolume(volume);
    }
}

void MediaPlayBarBase::setPosition(qint64 position)
{
    if (m_player) {
        m_player->setPosition(position);
    }
}

void MediaPlayBarBase::togglePlayState()
{
    if (!m_player) {
        return;
    }

    if (m_player->isPlaying()) {
        m_player->pause();
    } else {
        m_player->play();
    }

    m_playButton->setPlay(m_player->isPlaying());
}

void MediaPlayBarBase::onPositionChanged(qint64 position) { m_progressSlider->setValue(static_cast<int>(position)); }

void MediaPlayBarBase::paintEvent(QPaintEvent *) { paintCard(this); }

// ==========================================
// SimpleMediaPlayBar
// ==========================================

SimpleMediaPlayBar::SimpleMediaPlayBar(QWidget *parent) : MediaPlayBarBase(parent)
{
    m_hBoxLayout = new QHBoxLayout(this);

    m_hBoxLayout->setContentsMargins(10, 4, 10, 4);
    m_hBoxLayout->setSpacing(6);
    m_hBoxLayout->addWidget(playButton(), 0, Qt::AlignLeft);
    m_hBoxLayout->addWidget(progressSlider(), 1);
    m_hBoxLayout->addWidget(volumeButton(), 0);

    setFixedHeight(48);
    setMediaPlayer(new MediaPlayer(this));
}

void SimpleMediaPlayBar::addButton(MediaPlayBarButton *button) { m_hBoxLayout->addWidget(button, 0); }

QHBoxLayout *SimpleMediaPlayBar::hBoxLayout() const { return m_hBoxLayout; }

// ==========================================
// StandardMediaPlayBar
// ==========================================

StandardMediaPlayBar::StandardMediaPlayBar(QWidget *parent) : MediaPlayBarBase(parent)
{
    m_vBoxLayout = new QVBoxLayout(this);
    m_timeLayout = new QHBoxLayout();
    m_buttonLayout = new QHBoxLayout();
    auto *leftButtonContainer = new QWidget();
    auto *centerButtonContainer = new QWidget();
    auto *rightButtonContainer = new QWidget();
    m_leftButtonLayout = new QHBoxLayout(leftButtonContainer);
    m_centerButtonLayout = new QHBoxLayout(centerButtonContainer);
    m_rightButtonLayout = new QHBoxLayout(rightButtonContainer);

    m_skipBackButton = new MediaPlayBarButton(FluentIcon::SkipBack, this);
    m_skipForwardButton = new MediaPlayBarButton(FluentIcon::SkipForward, this);

    m_currentTimeLabel = new CaptionLabel(QStringLiteral("0:00:00"), this);
    m_remainTimeLabel = new CaptionLabel(QStringLiteral("0:00:00"), this);

    setFixedHeight(102);
    m_vBoxLayout->setSpacing(6);
    m_vBoxLayout->setContentsMargins(5, 9, 5, 9);
    m_vBoxLayout->addWidget(progressSlider(), 1, Qt::AlignTop);

    m_vBoxLayout->addLayout(m_timeLayout);
    m_timeLayout->setContentsMargins(10, 0, 10, 0);
    m_timeLayout->addWidget(m_currentTimeLabel, 0, Qt::AlignLeft);
    m_timeLayout->addWidget(m_remainTimeLabel, 0, Qt::AlignRight);

    m_vBoxLayout->addStretch(1);
    m_vBoxLayout->addLayout(m_buttonLayout, 1);
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_leftButtonLayout->setContentsMargins(4, 0, 0, 0);
    m_centerButtonLayout->setContentsMargins(0, 0, 0, 0);
    m_rightButtonLayout->setContentsMargins(0, 0, 4, 0);

    m_leftButtonLayout->addWidget(volumeButton(), 0, Qt::AlignLeft);
    m_centerButtonLayout->addWidget(m_skipBackButton);
    m_centerButtonLayout->addWidget(playButton());
    m_centerButtonLayout->addWidget(m_skipForwardButton);

    m_buttonLayout->addWidget(leftButtonContainer, 0, Qt::AlignLeft);
    m_buttonLayout->addWidget(centerButtonContainer, 0, Qt::AlignHCenter);
    m_buttonLayout->addWidget(rightButtonContainer, 0, Qt::AlignRight);

    setMediaPlayer(new MediaPlayer(this));

    connect(m_skipBackButton, &QAbstractButton::clicked, this, [this]() { skipBack(10000); });
    connect(m_skipForwardButton, &QAbstractButton::clicked, this, [this]() { skipForward(30000); });
}

MediaPlayBarButton *StandardMediaPlayBar::skipBackButton() const { return m_skipBackButton; }

MediaPlayBarButton *StandardMediaPlayBar::skipForwardButton() const { return m_skipForwardButton; }

CaptionLabel *StandardMediaPlayBar::currentTimeLabel() const { return m_currentTimeLabel; }

CaptionLabel *StandardMediaPlayBar::remainTimeLabel() const { return m_remainTimeLabel; }

QHBoxLayout *StandardMediaPlayBar::leftButtonLayout() const { return m_leftButtonLayout; }

QHBoxLayout *StandardMediaPlayBar::centerButtonLayout() const { return m_centerButtonLayout; }

QHBoxLayout *StandardMediaPlayBar::rightButtonLayout() const { return m_rightButtonLayout; }

void StandardMediaPlayBar::skipBack(qint64 ms)
{
    if (player()) {
        player()->setPosition(player()->position() - ms);
    }
}

void StandardMediaPlayBar::skipForward(qint64 ms)
{
    if (player()) {
        player()->setPosition(player()->position() + ms);
    }
}

void StandardMediaPlayBar::onPositionChanged(qint64 position)
{
    MediaPlayBarBase::onPositionChanged(position);
    m_currentTimeLabel->setText(formatTime(position));
    m_remainTimeLabel->setText(formatTime(player()->duration() - position));
}

QString StandardMediaPlayBar::formatTime(qint64 ms)
{
    // Python's int(time / 1000) truncates towards zero.
    const qint64 time = ms / 1000;
    const qint64 s = time % 60;
    const qint64 m = (time / 60) % 60;
    const qint64 h = time / 3600;
    return QStringLiteral("%1:%2:%3").arg(h).arg(m, 2, 10, QLatin1Char('0')).arg(s, 2, 10, QLatin1Char('0'));
}

} // namespace FluentQt
