// Media play bars, the volume flyout, the media player and the video widget (built with Qt Multimedia).

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <QtCore/QDataStream>
#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QtCore/QTemporaryDir>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimediaWidgets/QGraphicsVideoItem>
#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QVBoxLayout>

#include "TestSupport.h"

#include <cmath>
#include <optional>

using namespace FluentQt;

namespace {

// A player without a playback engine: records what the bar asks for, and lets the test emit what an engine
// would.
class FakePlayer : public MediaPlayerBase
{
  public:
    using MediaPlayerBase::MediaPlayerBase;

    bool isPlaying() const override { return m_state == QMediaPlayer::PlayingState; }
    QMediaPlayer::MediaStatus mediaStatus() const override { return QMediaPlayer::LoadedMedia; }
    QMediaPlayer::PlaybackState playbackState() const override { return m_state; }
    qint64 duration() const override { return m_duration; }
    qint64 position() const override { return m_position; }
    int volume() const override { return m_volume; }
    bool isMuted() const override { return m_muted; }
    QUrl source() const override { return m_source; }
    qreal playbackRate() const override { return 1; }
    QObject *videoOutput() const override { return m_videoOutput; }

    void pause() override { m_state = QMediaPlayer::PausedState; }
    void play() override { m_state = QMediaPlayer::PlayingState; }
    void stop() override
    {
        m_state = QMediaPlayer::StoppedState;
        m_position = 0;
    }
    void setPosition(qint64 position) override
    {
        m_position = position;
        ++positionRequests;
        emit positionChanged(position);
    }
    void setSource(const QUrl &source) override { m_source = source; }
    void setPlaybackRate(qreal) override {}
    void setVolume(int volume) override
    {
        if (volume != m_volume) {
            m_volume = volume;
            emit volumeChanged(volume);
        }
    }
    void setMuted(bool muted) override
    {
        if (muted != m_muted) {
            m_muted = muted;
            emit mutedChanged(muted);
        }
    }
    void setVideoOutput(QObject *output) override { m_videoOutput = output; }

    void setDuration(qint64 duration)
    {
        m_duration = duration;
        emit durationChanged(duration);
    }

    int positionRequests = 0;

  private:
    QMediaPlayer::PlaybackState m_state = QMediaPlayer::StoppedState;
    qint64 m_duration = 0;
    qint64 m_position = 0;
    int m_volume = 100;
    bool m_muted = false;
    QUrl m_source;
    QObject *m_videoOutput = nullptr;
};

QRect geometryIn(const QWidget *widget, const QWidget *ancestor)
{
    return QRect(widget->mapTo(ancestor, QPoint(0, 0)), widget->size());
}

// Within `tolerance` px on every side: the CaptionLabels' text is measured by Qt 5 in Python and Qt 6 here.
bool near(const QRect &actual, const QRect &expected, int tolerance)
{
    return std::abs(actual.x() - expected.x()) <= tolerance && std::abs(actual.y() - expected.y()) <= tolerance &&
           std::abs(actual.width() - expected.width()) <= tolerance &&
           std::abs(actual.height() - expected.height()) <= tolerance;
}

QByteArray describe(const QRect &actual, const QRect &expected)
{
    return QStringLiteral("C++ %1,%2 %3x%4, Python %5,%6 %7x%8")
        .arg(actual.x())
        .arg(actual.y())
        .arg(actual.width())
        .arg(actual.height())
        .arg(expected.x())
        .arg(expected.y())
        .arg(expected.width())
        .arg(expected.height())
        .toUtf8();
}

#define COMPARE_GEOMETRY(actual, expected, tolerance)                                                                  \
    QVERIFY2(near((actual), (expected), (tolerance)), describe((actual), (expected)).constData())

// A mono 16-bit PCM WAV of `ms` milliseconds of a quiet 440 Hz tone.
bool writeWav(const QString &path, int ms)
{
    const int sampleRate = 8000;
    const quint32 samples = static_cast<quint32>(sampleRate) * ms / 1000;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QDataStream out(&file);
    out.setByteOrder(QDataStream::LittleEndian);
    out.writeRawData("RIFF", 4);
    out << quint32(36 + samples * 2);
    out.writeRawData("WAVEfmt ", 8);
    out << quint32(16) << quint16(1) << quint16(1) << quint32(sampleRate) << quint32(sampleRate * 2) << quint16(2)
        << quint16(16);
    out.writeRawData("data", 4);
    out << quint32(samples * 2);
    for (quint32 i = 0; i < samples; ++i) {
        out << qint16(1000 * std::sin(2 * M_PI * 440 * i / sampleRate));
    }
    return out.status() == QDataStream::Ok;
}

} // namespace

class MediaTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    // Python's MediaPlayBarButton: 30x30, a 16x16 icon; the play button a 14x14 one.
    void buttonsAreThirtyPixelsSquare()
    {
        MediaPlayBarButton button(FluentIcon::SkipBack);
        QCOMPARE(button.size(), QSize(30, 30));
        QCOMPARE(button.minimumSize(), QSize(30, 30));
        QCOMPARE(button.maximumSize(), QSize(30, 30));
        QCOMPARE(button.iconSize(), QSize(16, 16));

        PlayButton play;
        QCOMPARE(play.size(), QSize(30, 30));
        QCOMPARE(play.iconSize(), QSize(14, 14));
        QCOMPARE(play.toolTip(), QStringLiteral("Play"));
        play.setPlay(true);
        QCOMPARE(play.toolTip(), QStringLiteral("Pause"));
        play.setPlay(false);
        QCOMPARE(play.toolTip(), QStringLiteral("Play"));

        // Their tool tips come after a second.
        const auto filters = button.findChildren<ToolTipFilter *>();
        QCOMPARE(filters.size(), 1);
        QCOMPARE(filters.first()->toolTipDelay(), 1000);
    }

    // Measured with tools/parity/measure_python_media.py at 600x48: margins 10,4, spacing 6, the play
    // button, the progress slider and the volume button (no time label).
    void simplePlayBarLayoutMatchesPython()
    {
        SimpleMediaPlayBar bar;
        bar.resize(600, 48);
        bar.show();
        QVERIFY(QTest::qWaitForWindowExposed(&bar));

        QCOMPARE(bar.height(), 48);
        QCOMPARE(bar.minimumHeight(), 48);
        QCOMPARE(bar.maximumHeight(), 48);
        COMPARE_GEOMETRY(geometryIn(bar.playButton(), &bar), QRect(10, 9, 30, 30), 0);
        COMPARE_GEOMETRY(geometryIn(bar.progressSlider(), &bar), QRect(46, 13, 508, 22), 0);
        COMPARE_GEOMETRY(geometryIn(bar.volumeButton(), &bar), QRect(560, 9, 30, 30), 0);
        QVERIFY(bar.findChildren<QLabel *>(QString(), Qt::FindDirectChildrenOnly).isEmpty());

        // addButton puts buttons after the volume button.
        auto *extra = new MediaPlayBarButton(FluentIcon::FullScreen);
        bar.addButton(extra);
        QCOMPARE(bar.hBoxLayout()->indexOf(extra), 3);
        QCOMPARE(bar.hBoxLayout()->indexOf(bar.volumeButton()), 2);
    }

    // Measured with tools/parity/measure_python_media.py at 600x102.
    void standardPlayBarLayoutMatchesPython()
    {
        StandardMediaPlayBar bar;
        bar.resize(600, 102);
        bar.show();
        QVERIFY(QTest::qWaitForWindowExposed(&bar));

        QCOMPARE(bar.height(), 102);
        QCOMPARE(bar.maximumHeight(), 102);
        COMPARE_GEOMETRY(geometryIn(bar.progressSlider(), &bar), QRect(5, 9, 590, 22), 0);
        if (FqwTest::hasPythonMeasurementFont(bar.currentTimeLabel())) {
            COMPARE_GEOMETRY(geometryIn(bar.currentTimeLabel(), &bar), QRect(15, 37, 43, 17), 2);
            COMPARE_GEOMETRY(geometryIn(bar.remainTimeLabel(), &bar), QRect(542, 37, 43, 17), 2);
        }
        COMPARE_GEOMETRY(geometryIn(bar.volumeButton(), &bar), QRect(9, 63, 30, 30), 0);
        COMPARE_GEOMETRY(geometryIn(bar.skipBackButton(), &bar), QRect(249, 63, 30, 30), 0);
        COMPARE_GEOMETRY(geometryIn(bar.playButton(), &bar), QRect(285, 63, 30, 30), 0);
        COMPARE_GEOMETRY(geometryIn(bar.skipForwardButton(), &bar), QRect(321, 63, 30, 30), 0);

        QCOMPARE(bar.currentTimeLabel()->property("fqw").toString(), QStringLiteral("CaptionLabel"));
        QCOMPARE(bar.currentTimeLabel()->text(), QStringLiteral("0:00:00"));
        QCOMPARE(bar.remainTimeLabel()->text(), QStringLiteral("0:00:00"));
        QCOMPARE(bar.leftButtonLayout()->indexOf(bar.volumeButton()), 0);
        QCOMPARE(bar.centerButtonLayout()->indexOf(bar.playButton()), 1);
        QCOMPARE(bar.rightButtonLayout()->count(), 0);
    }

    // Python's VolumeView: 295x64, the mute button at 10, the 208 px slider at 45,21, the volume right-aligned
    // 20 px from the edge.
    void volumeViewMatchesPython()
    {
        VolumeView view;
        view.setVolume(30);
        view.show();
        QVERIFY(QTest::qWaitForWindowExposed(&view));

        QCOMPARE(view.size(), QSize(295, 64));
        COMPARE_GEOMETRY(view.muteButton()->geometry(), QRect(10, 17, 30, 30), 0);
        COMPARE_GEOMETRY(view.volumeSlider()->geometry(), QRect(45, 21, 208, 22), 0);
        if (FqwTest::hasPythonMeasurementFont(view.volumeLabel())) {
            COMPARE_GEOMETRY(view.volumeLabel()->geometry(), QRect(261, 24, 15, 17), 2);
        }
        QCOMPARE(view.volumeLabel()->text(), QStringLiteral("30"));

        view.setVolume(100);
        QCOMPARE(view.volumeSlider()->value(), 100);
        QCOMPARE(view.volumeLabel()->text(), QStringLiteral("100"));
        QVERIFY(view.volumeLabel()->x() < 261);

        view.setMuted(true);
        QCOMPARE(view.muteButton()->toolTip(), QStringLiteral("Unmute"));
        view.setMuted(false);
        QCOMPARE(view.muteButton()->toolTip(), QStringLiteral("Mute"));
    }

    // Measured with tools/parity/measure_python_media.py at 800x450: the bar floats 11 px in from the edges.
    void videoWidgetFloatsItsPlayBarLikePython()
    {
        VideoWidget video;
        video.resize(800, 450);
        video.show();
        QVERIFY(QTest::qWaitForWindowExposed(&video));

        COMPARE_GEOMETRY(video.playBar()->geometry(), QRect(11, 337, 778, 102), 0);
        QCOMPARE(video.horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(video.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(video.videoItem()->size(), QSizeF(800, 450));
        QCOMPARE(video.player()->videoOutput(), static_cast<QObject *>(video.videoItem()));
        QVERIFY(video.hasMouseTracking());

        video.resize(640, 360);
        QCOMPARE(video.playBar()->geometry(), QRect(11, 247, 618, 102));
        QCOMPARE(video.videoItem()->size(), QSizeF(640, 360));
    }

    // The bar fades in when the pointer enters, and out three seconds after it leaves; hiding pauses.
    void videoWidgetFadesItsPlayBarOnHover()
    {
        VideoWidget video;
        auto *player = new FakePlayer(&video);
        video.playBar()->setMediaPlayer(player);
        video.resize(400, 300);
        // Not shown yet: on a desktop the real pointer would send its own Enter and Leave events.
        auto *effect = video.playBar()->opacityEffect();
        QCOMPARE(effect->opacity(), 1.0);

        QEvent leave(QEvent::Leave);
        QApplication::sendEvent(&video, &leave);
        QTest::qWait(1000);
        QCOMPARE(effect->opacity(), 1.0);
        QTRY_COMPARE_WITH_TIMEOUT(effect->opacity(), 0.0, 4000);

        QEnterEvent enter(QPointF(10, 10), QPointF(10, 10), QPointF(10, 10));
        QApplication::sendEvent(&video, &enter);
        QTRY_COMPARE_WITH_TIMEOUT(effect->opacity(), 1.0, 1000);

        // Back in before the three seconds are up: the bar stays.
        QApplication::sendEvent(&video, &leave);
        QTest::qWait(500);
        QApplication::sendEvent(&video, &enter);
        QTest::qWait(3000);
        QCOMPARE(effect->opacity(), 1.0);

        video.show();
        QVERIFY(QTest::qWaitForWindowExposed(&video));
        video.play();
        QVERIFY(player->isPlaying());
        video.togglePlayState();
        QVERIFY(!player->isPlaying());
        video.togglePlayState();
        QVERIFY(player->isPlaying());
        video.hide();
        QVERIFY(!player->isPlaying());
    }

    // The bar's controls drive the player, and the player's signals update the bar.
    void playBarDrivesItsPlayer()
    {
        StandardMediaPlayBar bar;
        FakePlayer player;
        bar.setMediaPlayer(&player);
        QCOMPARE(bar.player(), &player);

        // setMediaPlayer sets the volume to 30, as Python's does.
        QCOMPARE(player.volume(), 30);
        QCOMPARE(bar.volumeButton()->volumeView()->volumeSlider()->value(), 30);

        player.setDuration(125000);
        QCOMPARE(bar.progressSlider()->maximum(), 125000);

        // Play / pause.
        QTest::mouseClick(bar.playButton(), Qt::LeftButton);
        QVERIFY(player.isPlaying());
        QCOMPARE(bar.playButton()->toolTip(), QStringLiteral("Pause"));
        QTest::mouseClick(bar.playButton(), Qt::LeftButton);
        QVERIFY(!player.isPlaying());
        QCOMPARE(bar.playButton()->toolTip(), QStringLiteral("Play"));

        // The position: slider and time labels (h:mm:ss of the position and of what's left).
        player.setPosition(62500);
        QCOMPARE(bar.progressSlider()->value(), 62500);
        QCOMPARE(bar.currentTimeLabel()->text(), QStringLiteral("0:01:02"));
        QCOMPARE(bar.remainTimeLabel()->text(), QStringLiteral("0:01:02"));
        player.setDuration(3725000);
        player.setPosition(0);
        QCOMPARE(bar.remainTimeLabel()->text(), QStringLiteral("1:02:05"));

        // Seeking: dragging and clicking the slider set the position; the playback moving the slider doesn't
        // seek back.
        player.positionRequests = 0;
        bar.progressSlider()->setValue(5000);
        QCOMPARE(player.positionRequests, 0);
        emit bar.progressSlider()->sliderMoved(20000);
        QCOMPARE(player.position(), 20000);
        emit bar.progressSlider()->clicked(40000);
        QCOMPARE(player.position(), 40000);

        // Skip back 10 s and forward 30 s.
        QTest::mouseClick(bar.skipBackButton(), Qt::LeftButton);
        QCOMPARE(player.position(), 30000);
        QTest::mouseClick(bar.skipForwardButton(), Qt::LeftButton);
        QCOMPARE(player.position(), 60000);

        // Volume: both ways between the player and the volume flyout.
        auto *view = bar.volumeButton()->volumeView();
        player.setVolume(70);
        QCOMPARE(view->volumeSlider()->value(), 70);
        QCOMPARE(view->volumeLabel()->text(), QStringLiteral("70"));
        view->volumeSlider()->setValue(40);
        QCOMPARE(player.volume(), 40);
        bar.setVolume(55);
        QCOMPARE(view->volumeSlider()->value(), 55);

        // Mute: the flyout's mute button toggles it, and the volume button's icon state follows.
        QTest::mouseClick(view->muteButton(), Qt::LeftButton);
        QVERIFY(player.isMuted());
        QVERIFY(bar.volumeButton()->isMuted());
        QCOMPARE(view->muteButton()->toolTip(), QStringLiteral("Unmute"));
        QTest::mouseClick(view->muteButton(), Qt::LeftButton);
        QVERIFY(!player.isMuted());
        QVERIFY(!bar.volumeButton()->isMuted());

        // Another player replaces the first.
        FakePlayer other;
        bar.setMediaPlayer(&other);
        player.setPosition(1000);
        QCOMPARE(bar.progressSlider()->value(), 60000);
        emit bar.progressSlider()->clicked(3000);
        QCOMPARE(player.position(), 1000);
        QCOMPARE(other.position(), 3000);
    }

    // The volume button opens its flyout above itself, and only once.
    void volumeButtonPullsUpItsFlyout()
    {
        QWidget window;
        auto *layout = new QVBoxLayout(&window);
        layout->addStretch(1);
        auto *bar = new SimpleMediaPlayBar(&window);
        layout->addWidget(bar);
        window.resize(600, 300);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));

        auto *flyout = bar->volumeButton()->volumeFlyout();
        QVERIFY(!flyout->isVisible());
        QCOMPARE(flyout->view(), bar->volumeButton()->volumeView());
        QCOMPARE(flyout->animationType(), FlyoutAnimationType::PullUp);
        QVERIFY(!flyout->isDeleteOnClose());

        QTest::mouseClick(bar->volumeButton(), Qt::LeftButton);
        QTRY_VERIFY(flyout->isVisible());
        QTest::qWait(400);
        const QPoint buttonTop = bar->volumeButton()->mapToGlobal(QPoint(0, 0));
        QVERIFY2(flyout->geometry().top() < buttonTop.y(), "the flyout opens above the button");
        const QPoint viewBottom =
            bar->volumeButton()->volumeView()->mapToGlobal(QPoint(0, bar->volumeButton()->volumeView()->height()));
        QVERIFY(viewBottom.y() <= buttonTop.y());

        flyout->close();
        QTRY_VERIFY(!flyout->isVisible());
        QVERIFY(bar->volumeButton()->volumeView());
    }

    // The flyout lives on the window, like Python's, but goes with its button.
    void volumeFlyoutGoesWithItsButton()
    {
        QWidget window;
        auto *bar = new SimpleMediaPlayBar(&window);
        QPointer<Flyout> flyout = bar->volumeButton()->volumeFlyout();
        QCOMPARE(flyout->parentWidget(), &window);
        delete bar;
        QVERIFY(flyout.isNull());
    }

    // The fade animations restart from wherever the opacity is.
    void playBarFadesInAndOut()
    {
        SimpleMediaPlayBar bar;
        auto *effect = bar.opacityEffect();
        QCOMPARE(bar.graphicsEffect(), effect);
        bar.fadeOut();
        QTRY_COMPARE_WITH_TIMEOUT(effect->opacity(), 0.0, 1000);
        bar.fadeIn();
        QTest::qWait(100);
        bar.fadeOut();
        QTRY_COMPARE_WITH_TIMEOUT(effect->opacity(), 0.0, 1000);
        bar.fadeIn();
        QTRY_COMPARE_WITH_TIMEOUT(effect->opacity(), 1.0, 1000);
    }

    // MediaPlayer: volume in 0-100 on its QAudioOutput, signals only on changes, as Python's.
    void mediaPlayerVolumeAndMute()
    {
        MediaPlayer player;
        QVERIFY(player.mediaPlayer());
        QCOMPARE(player.mediaPlayer()->audioOutput(), player.audioOutput());
        QCOMPARE(player.volume(), 100);

        QSignalSpy volumeSpy(&player, &MediaPlayerBase::volumeChanged);
        QSignalSpy mutedSpy(&player, &MediaPlayerBase::mutedChanged);
        player.setVolume(29);
        QCOMPARE(player.volume(), 29);
        QCOMPARE(player.audioOutput()->volume(), 0.29f);
        player.setVolume(29);
        QCOMPARE(volumeSpy.count(), 1);
        QCOMPARE(volumeSpy.first().first().toInt(), 29);

        player.setMuted(true);
        player.setMuted(true);
        QVERIFY(player.isMuted());
        QVERIFY(player.audioOutput()->isMuted());
        QCOMPARE(mutedSpy.count(), 1);

        SimpleMediaPlayBar bar;
        auto *barPlayer = qobject_cast<MediaPlayer *>(bar.player());
        QVERIFY(barPlayer);
        QCOMPARE(barPlayer->volume(), 30);
    }

    // A real file through MediaPlayer and the standard bar: load, play, pause, seek.
    void standardPlayBarPlaysAWavFile()
    {
#ifdef Q_OS_MACOS
        // AVFoundation reports through the Cocoa event loop, which the offscreen platform doesn't run: the
        // media would stay LoadingMedia. tst_media_native runs this on the desktop.
        if (QGuiApplication::platformName() == QLatin1String("offscreen")) {
            QSKIP("AVFoundation needs the Cocoa platform to load media (run tst_media_native)");
        }
#endif
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        const QString path = dir.filePath(QStringLiteral("tone.wav"));
        QVERIFY(writeWav(path, 3000));

        StandardMediaPlayBar bar;
        auto *player = qobject_cast<MediaPlayer *>(bar.player());
        QVERIFY(player);
        if (!player->mediaPlayer()->isAvailable()) {
            QSKIP("No Qt Multimedia backend: set QT_PLUGIN_PATH to the multimedia plugins");
        }
        player->setVolume(0);

        QSignalSpy sourceSpy(player, &MediaPlayerBase::sourceChanged);
        player->setSource(QUrl::fromLocalFile(path));
        QCOMPARE(sourceSpy.count(), 1);
        QCOMPARE(player->source(), QUrl::fromLocalFile(path));
        QTRY_COMPARE_WITH_TIMEOUT(player->mediaStatus(), QMediaPlayer::LoadedMedia, 5000);
        QTRY_VERIFY_WITH_TIMEOUT(std::abs(player->duration() - 3000) <= 50, 5000);
        QCOMPARE(bar.progressSlider()->maximum(), static_cast<int>(player->duration()));

        QTest::mouseClick(bar.playButton(), Qt::LeftButton);
        QVERIFY(player->isPlaying());
        QCOMPARE(bar.playButton()->toolTip(), QStringLiteral("Pause"));
        QTRY_VERIFY_WITH_TIMEOUT(player->position() > 0, 3000);
        QTRY_VERIFY_WITH_TIMEOUT(bar.progressSlider()->value() > 0, 3000);

        QTest::mouseClick(bar.playButton(), Qt::LeftButton);
        QVERIFY(!player->isPlaying());
        QCOMPARE(bar.playButton()->toolTip(), QStringLiteral("Play"));

        emit bar.progressSlider()->clicked(1500);
        QTRY_VERIFY_WITH_TIMEOUT(std::abs(player->position() - 1500) <= 50, 3000);
        QTRY_COMPARE(bar.currentTimeLabel()->text(), QStringLiteral("0:00:01"));
        QCOMPARE(bar.remainTimeLabel()->text(), QStringLiteral("0:00:01"));
        QCOMPARE(bar.progressSlider()->value(), static_cast<int>(player->position()));

        // Played to the end, the button goes back to Play.
        bar.play();
        QTRY_COMPARE_WITH_TIMEOUT(player->mediaStatus(), QMediaPlayer::EndOfMedia, 5000);
        QTRY_COMPARE(bar.playButton()->toolTip(), QStringLiteral("Play"));
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

// Measured like tools/parity/measure_python_media.py: Fusion and a 14 px application font in the Fluent
// families.
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QFont font = QApplication::font();
    font.setFamilies({QStringLiteral("Segoe UI"), QStringLiteral("Microsoft YaHei"), QStringLiteral("PingFang SC")});
    font.setPixelSize(14);
    QApplication::setFont(font);
    ThemeManager::instance()->setTheme(Theme::Light);
    MediaTest test;
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&test, argc, argv);
}

#include "tst_media.moc"
