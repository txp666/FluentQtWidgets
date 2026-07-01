#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtCore/QVector>
#if defined(FQW_HAS_MULTIMEDIA) && FQW_HAS_MULTIMEDIA
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QMediaPlayer>
#endif

#include <cmath>
#include <cstring>
#include <limits>

using namespace FluentQt;

namespace {

constexpr auto kWaveformAudioResourcePath = ":/gallery/audio/test.wav";
constexpr auto kWaveformAudioResourceUrl = "qrc:/gallery/audio/test.wav";

quint16 readLittleEndian16(const QByteArray &data, int offset)
{
    if (offset < 0 || offset + 1 >= data.size()) {
        return 0;
    }
    return static_cast<quint16>(static_cast<unsigned char>(data.at(offset))) |
           (static_cast<quint16>(static_cast<unsigned char>(data.at(offset + 1))) << 8);
}

quint32 readLittleEndian32(const QByteArray &data, int offset)
{
    if (offset < 0 || offset + 3 >= data.size()) {
        return 0;
    }
    return static_cast<quint32>(static_cast<unsigned char>(data.at(offset))) |
           (static_cast<quint32>(static_cast<unsigned char>(data.at(offset + 1))) << 8) |
           (static_cast<quint32>(static_cast<unsigned char>(data.at(offset + 2))) << 16) |
           (static_cast<quint32>(static_cast<unsigned char>(data.at(offset + 3))) << 24);
}

bool chunkIdEquals(const QByteArray &data, int offset, const char *id)
{
    return offset >= 0 && offset + 4 <= data.size() && std::memcmp(data.constData() + offset, id, 4) == 0;
}

qreal pcmLevel(const QByteArray &data, int offset, int bitsPerSample)
{
    switch (bitsPerSample) {
    case 8:
        return qAbs((static_cast<int>(static_cast<unsigned char>(data.at(offset))) - 128) / 128.0);
    case 16: {
        const auto sample = static_cast<qint16>(readLittleEndian16(data, offset));
        return qAbs(sample / 32768.0);
    }
    case 24: {
        int sample = static_cast<int>(static_cast<unsigned char>(data.at(offset))) |
                     (static_cast<int>(static_cast<unsigned char>(data.at(offset + 1))) << 8) |
                     (static_cast<int>(static_cast<unsigned char>(data.at(offset + 2))) << 16);
        if ((sample & 0x800000) != 0) {
            sample |= ~0xFFFFFF;
        }
        return qAbs(sample / 8388608.0);
    }
    case 32: {
        const auto sample = static_cast<qint32>(readLittleEndian32(data, offset));
        return qAbs(sample / 2147483648.0);
    }
    default:
        return 0;
    }
}

QVector<qreal> generatedWaveformSamples()
{
    QVector<qreal> samples;
    samples.reserve(260);
    for (int i = 0; i < 260; ++i) {
        const qreal t = static_cast<qreal>(i) / 259.0;
        qreal envelope = 0.0;
        if (t < 0.07) {
            envelope = 0.05;
        } else if (t < 0.52) {
            envelope = 0.46 + 0.12 * std::sin(t * 72.0);
        } else if (t < 0.74) {
            envelope = 0.72 + 0.24 * std::sin(t * 58.0);
        } else {
            envelope = 0.52 * std::exp(-(t - 0.74) * 16.0);
        }

        const qreal carrier = 0.55 + 0.35 * std::sin(i * 1.37) + 0.25 * std::sin(i * 2.91);
        samples.append(qBound<qreal>(0.02, envelope * qAbs(carrier), 1.0));
    }
    return samples;
}

QVector<qreal> waveformSamplesFromWav(const QString &path, int maximumBars = 900)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return generatedWaveformSamples();
    }

    const QByteArray data = file.readAll();
    if (data.size() < 44 || !chunkIdEquals(data, 0, "RIFF") || !chunkIdEquals(data, 8, "WAVE")) {
        return generatedWaveformSamples();
    }

    int audioFormat = 0;
    int channelCount = 0;
    int bitsPerSample = 0;
    int dataOffset = -1;
    int dataSize = 0;
    for (int offset = 12; offset + 8 <= data.size();) {
        const quint32 chunkSize = readLittleEndian32(data, offset + 4);
        const int payloadOffset = offset + 8;
        const int nextOffset = payloadOffset + static_cast<int>(chunkSize) + static_cast<int>(chunkSize % 2);
        if (nextOffset < payloadOffset || payloadOffset + static_cast<int>(chunkSize) > data.size()) {
            break;
        }

        if (chunkIdEquals(data, offset, "fmt ") && chunkSize >= 16) {
            audioFormat = readLittleEndian16(data, payloadOffset);
            channelCount = readLittleEndian16(data, payloadOffset + 2);
            bitsPerSample = readLittleEndian16(data, payloadOffset + 14);
        } else if (chunkIdEquals(data, offset, "data")) {
            dataOffset = payloadOffset;
            dataSize = static_cast<int>(chunkSize);
        }

        offset = nextOffset;
    }

    const int bytesPerSample = bitsPerSample / 8;
    const int frameSize = bytesPerSample * channelCount;
    if (audioFormat != 1 || channelCount <= 0 || bytesPerSample <= 0 || frameSize <= 0 || dataOffset < 0 ||
        dataSize <= 0) {
        return generatedWaveformSamples();
    }

    const int frameCount = dataSize / frameSize;
    const int barCount = qBound(1, maximumBars, frameCount);
    QVector<qreal> peaks;
    peaks.reserve(barCount);
    qreal globalPeak = 0;
    for (int i = 0; i < barCount; ++i) {
        const int beginFrame = static_cast<int>(std::floor(static_cast<qreal>(i) * frameCount / barCount));
        const int endFrame =
            qMax(beginFrame + 1, static_cast<int>(std::floor(static_cast<qreal>(i + 1) * frameCount / barCount)));
        qreal peak = 0;
        for (int frame = beginFrame; frame < qMin(endFrame, frameCount); ++frame) {
            const int frameOffset = dataOffset + frame * frameSize;
            for (int channel = 0; channel < channelCount; ++channel) {
                const int sampleOffset = frameOffset + channel * bytesPerSample;
                peak = qMax(peak, pcmLevel(data, sampleOffset, bitsPerSample));
            }
        }
        globalPeak = qMax(globalPeak, peak);
        peaks.append(peak);
    }

    if (globalPeak > std::numeric_limits<qreal>::epsilon()) {
        for (qreal &peak : peaks) {
            peak = qBound<qreal>(0.0, peak / globalPeak, 1.0);
        }
    }
    return peaks;
}

QString formatMilliseconds(qint64 milliseconds)
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

void updateAudioTimeLabel(BodyLabel *label, qint64 position, qint64 duration)
{
    if (!label) {
        return;
    }
    label->setText(QStringLiteral("%1 / %2").arg(formatMilliseconds(position), formatMilliseconds(duration)));
}

QWidget *createAudioWaveform(QWidget *parent)
{
    auto *view = new QWidget(parent);
    auto *layout = new QVBoxLayout(view);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(10);

    auto *controls = new QWidget(view);
    auto *controlsLayout = new QHBoxLayout(controls);
    controlsLayout->setContentsMargins(4, 0, 4, 0);
    controlsLayout->setSpacing(8);

    auto *playButton = new TransparentToolButton(icon(FluentIcon::Play), controls);
    playButton->setFixedSize(36, 36);
    playButton->setIconSize(QSize(16, 16));

    auto *fileLabel = new BodyLabel(QStringLiteral("test.wav"), controls);
    auto *timeLabel = new BodyLabel(QStringLiteral("00:00 / 00:00"), controls);
    timeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    timeLabel->setMinimumWidth(96);

    controlsLayout->addWidget(playButton);
    controlsLayout->addWidget(fileLabel);
    controlsLayout->addStretch(1);
    controlsLayout->addWidget(timeLabel);

    auto *waveform = new AudioWaveformWidget(view);
    waveform->setSamples(waveformSamplesFromWav(QString::fromLatin1(kWaveformAudioResourcePath)));
    waveform->setProgress(0);
    waveform->setMinimumHeight(220);
    waveform->setBarWidth(2.0);
    waveform->setBarGap(3.0);

    layout->addWidget(controls);
    layout->addWidget(waveform);

#if defined(FQW_HAS_MULTIMEDIA) && FQW_HAS_MULTIMEDIA
    auto *player = new QMediaPlayer(view);
    auto *audioOutput = new QAudioOutput(view);
    audioOutput->setVolume(0.8);
    player->setAudioOutput(audioOutput);
    player->setSource(QUrl(QString::fromLatin1(kWaveformAudioResourceUrl)));

    QObject::connect(playButton, &QToolButton::clicked, view, [player]() {
        if (player->playbackState() == QMediaPlayer::PlayingState) {
            player->pause();
        } else {
            player->play();
        }
    });
    QObject::connect(player, &QMediaPlayer::playbackStateChanged, view, [playButton](QMediaPlayer::PlaybackState state) {
        playButton->setIcon(icon(state == QMediaPlayer::PlayingState ? FluentIcon::Pause : FluentIcon::Play));
    });
    QObject::connect(player, &QMediaPlayer::durationChanged, view,
                     [player, timeLabel](qint64 duration) { updateAudioTimeLabel(timeLabel, player->position(), duration); });
    QObject::connect(player, &QMediaPlayer::positionChanged, view, [player, waveform, timeLabel](qint64 position) {
        const qint64 duration = player->duration();
        if (duration > 0) {
            waveform->setProgress(static_cast<qreal>(position) / duration);
        }
        updateAudioTimeLabel(timeLabel, position, duration);
    });
    QObject::connect(waveform, &AudioWaveformWidget::waveformClicked, view, [player](qreal progress) {
        if (player->duration() > 0) {
            player->setPosition(qRound64(progress * player->duration()));
        }
    });
    QObject::connect(waveform, &AudioWaveformWidget::waveformMoved, view, [player](qreal progress) {
        if (player->duration() > 0) {
            player->setPosition(qRound64(progress * player->duration()));
        }
    });
#else
    playButton->setEnabled(false);
#endif

    return view;
}

qreal realtimeValue(qreal t, int seriesIndex)
{
    const qreal base = 48.0 + 20.0 * std::sin(t * 0.032 + seriesIndex * 0.8);
    const qreal detail = 9.0 * std::sin(t * (0.097 + seriesIndex * 0.013));
    const qreal drift = 6.0 * std::sin(t * 0.007 + seriesIndex * 1.7);
    qreal pulse = 0.0;
    if (seriesIndex == 2) {
        const qreal phase = std::fmod(t, 180.0);
        pulse = phase < 24.0 ? 28.0 * std::exp(-phase * 0.12) : 0.0;
    }
    return qBound<qreal>(0.0, base + detail + drift + pulse, 100.0);
}

RealtimePlotWidget *createRealtimePlot(QWidget *parent)
{
    auto *plot = new RealtimePlotWidget(parent);
    plot->setMinimumHeight(320);
    plot->setCapacity(60000);
    plot->setSeriesName(0, tx("ChartInterface", "CPU"));
    plot->setSeriesColor(0, QColor(0, 159, 170));
    const int memorySeries = plot->addSeries(tx("ChartInterface", "Memory"), QColor(22, 163, 74));
    const int ioSeries = plot->addSeries(tx("ChartInterface", "I/O"), QColor(245, 158, 11));

    constexpr int seedCount = 900;
    QVector<qreal> cpu;
    QVector<qreal> memory;
    QVector<qreal> io;
    cpu.reserve(seedCount);
    memory.reserve(seedCount);
    io.reserve(seedCount);
    for (int i = 0; i < seedCount; ++i) {
        cpu.append(realtimeValue(i, 0));
        memory.append(realtimeValue(i, 1));
        io.append(realtimeValue(i, 2));
    }
    plot->setSamples(0, cpu);
    plot->setSamples(memorySeries, memory);
    plot->setSamples(ioSeries, io);

    auto *timer = new QTimer(plot);
    QObject::connect(timer, &QTimer::timeout, plot,
                     [plot, memorySeries, ioSeries, t = static_cast<qreal>(seedCount)]() mutable {
                         plot->appendSample(0, realtimeValue(t, 0));
                         plot->appendSample(memorySeries, realtimeValue(t, 1));
                         plot->appendSample(ioSeries, realtimeValue(t, 2));
                         t += 1.0;
                     });
    timer->start(24);
    return plot;
}

QJsonObject axisChartOption(const QString &title, const QStringList &categories, const QJsonArray &values,
                            const QString &seriesName, const QString &seriesType)
{
    return {
        {QStringLiteral("title"), QJsonObject{{QStringLiteral("text"), title}, {QStringLiteral("left"), QStringLiteral("center")}}},
        {QStringLiteral("tooltip"), QJsonObject{{QStringLiteral("trigger"), QStringLiteral("axis")}}},
        {QStringLiteral("grid"),
         QJsonObject{{QStringLiteral("left"), QStringLiteral("3%")},
                     {QStringLiteral("right"), QStringLiteral("4%")},
                     {QStringLiteral("bottom"), QStringLiteral("3%")},
                     {QStringLiteral("containLabel"), true}}},
        {QStringLiteral("xAxis"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("category")},
                     {QStringLiteral("boundaryGap"), seriesType != QStringLiteral("line")},
                     {QStringLiteral("data"), QJsonArray::fromStringList(categories)}}},
        {QStringLiteral("yAxis"), QJsonObject{{QStringLiteral("type"), QStringLiteral("value")}}},
        {QStringLiteral("series"),
         QJsonArray{QJsonObject{{QStringLiteral("name"), seriesName},
                                {QStringLiteral("type"), seriesType},
                                {QStringLiteral("smooth"), true},
                                {QStringLiteral("data"), values}}}}};
}

QJsonObject barOption()
{
    return axisChartOption(tx("ChartInterface", "Weekly active users"),
                           {tx("ChartInterface", "Mon"), tx("ChartInterface", "Tue"), tx("ChartInterface", "Wed"),
                            tx("ChartInterface", "Thu"), tx("ChartInterface", "Fri"), tx("ChartInterface", "Sat"),
                            tx("ChartInterface", "Sun")},
                           QJsonArray{120, 200, 150, 80, 70, 110, 130},
                           tx("ChartInterface", "Users"), QStringLiteral("bar"));
}

QJsonObject lineOption()
{
    return axisChartOption(tx("ChartInterface", "Revenue trend"),
                           {tx("ChartInterface", "Jan"), tx("ChartInterface", "Feb"), tx("ChartInterface", "Mar"),
                            tx("ChartInterface", "Apr"), tx("ChartInterface", "May"), tx("ChartInterface", "Jun")},
                           QJsonArray{820, 932, 901, 934, 1290, 1330},
                           tx("ChartInterface", "Revenue"), QStringLiteral("line"));
}

QJsonObject areaOption()
{
    const QStringList days{tx("ChartInterface", "Mon"), tx("ChartInterface", "Tue"), tx("ChartInterface", "Wed"),
                           tx("ChartInterface", "Thu"), tx("ChartInterface", "Fri"), tx("ChartInterface", "Sat"),
                           tx("ChartInterface", "Sun")};
    return {
        {QStringLiteral("title"),
         QJsonObject{{QStringLiteral("text"), tx("ChartInterface", "Stacked engagement")},
                     {QStringLiteral("left"), QStringLiteral("center")}}},
        {QStringLiteral("tooltip"), QJsonObject{{QStringLiteral("trigger"), QStringLiteral("axis")}}},
        {QStringLiteral("legend"), QJsonObject{{QStringLiteral("top"), 28}}},
        {QStringLiteral("grid"),
         QJsonObject{{QStringLiteral("left"), QStringLiteral("3%")},
                     {QStringLiteral("right"), QStringLiteral("4%")},
                     {QStringLiteral("bottom"), QStringLiteral("3%")},
                     {QStringLiteral("containLabel"), true}}},
        {QStringLiteral("xAxis"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("category")},
                     {QStringLiteral("boundaryGap"), false},
                     {QStringLiteral("data"), QJsonArray::fromStringList(days)}}},
        {QStringLiteral("yAxis"), QJsonObject{{QStringLiteral("type"), QStringLiteral("value")}}},
        {QStringLiteral("series"),
         QJsonArray{
             QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Desktop")},
                         {QStringLiteral("type"), QStringLiteral("line")},
                         {QStringLiteral("stack"), QStringLiteral("Total")},
                         {QStringLiteral("smooth"), true},
                         {QStringLiteral("areaStyle"), QJsonObject{}},
                         {QStringLiteral("data"), QJsonArray{120, 132, 101, 134, 90, 230, 210}}},
             QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Mobile")},
                         {QStringLiteral("type"), QStringLiteral("line")},
                         {QStringLiteral("stack"), QStringLiteral("Total")},
                         {QStringLiteral("smooth"), true},
                         {QStringLiteral("areaStyle"), QJsonObject{}},
                         {QStringLiteral("data"), QJsonArray{220, 182, 191, 234, 290, 330, 310}}},
             QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Tablet")},
                         {QStringLiteral("type"), QStringLiteral("line")},
                         {QStringLiteral("stack"), QStringLiteral("Total")},
                         {QStringLiteral("smooth"), true},
                         {QStringLiteral("areaStyle"), QJsonObject{}},
                         {QStringLiteral("data"), QJsonArray{150, 232, 201, 154, 190, 330, 410}}}}}};
}

QJsonObject mixedOption()
{
    const QStringList months{tx("ChartInterface", "Jan"), tx("ChartInterface", "Feb"), tx("ChartInterface", "Mar"),
                             tx("ChartInterface", "Apr"), tx("ChartInterface", "May"), tx("ChartInterface", "Jun")};
    const QJsonArray yAxis{
        QJsonObject{{QStringLiteral("type"), QStringLiteral("value")},
                    {QStringLiteral("name"), tx("ChartInterface", "Orders")}},
        QJsonObject{{QStringLiteral("type"), QStringLiteral("value")},
                    {QStringLiteral("name"), tx("ChartInterface", "Rate")},
                    {QStringLiteral("axisLabel"),
                     QJsonObject{{QStringLiteral("formatter"), QStringLiteral("{value}%")}}}}};
    const QJsonArray series{
        QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Orders")},
                    {QStringLiteral("type"), QStringLiteral("bar")},
                    {QStringLiteral("data"), QJsonArray{320, 382, 301, 434, 490, 530}}},
        QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Conversion")},
                    {QStringLiteral("type"), QStringLiteral("line")},
                    {QStringLiteral("smooth"), true},
                    {QStringLiteral("yAxisIndex"), 1},
                    {QStringLiteral("data"), QJsonArray{3.2, 4.1, 3.6, 5.2, 5.8, 6.4}}}};
    return {
        {QStringLiteral("title"),
         QJsonObject{{QStringLiteral("text"), tx("ChartInterface", "Sales and conversion")},
                     {QStringLiteral("left"), QStringLiteral("center")}}},
        {QStringLiteral("tooltip"), QJsonObject{{QStringLiteral("trigger"), QStringLiteral("axis")}}},
        {QStringLiteral("legend"), QJsonObject{{QStringLiteral("top"), 28}}},
        {QStringLiteral("grid"),
         QJsonObject{{QStringLiteral("left"), QStringLiteral("3%")},
                     {QStringLiteral("right"), QStringLiteral("4%")},
                     {QStringLiteral("bottom"), QStringLiteral("3%")},
                     {QStringLiteral("containLabel"), true}}},
        {QStringLiteral("xAxis"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("category")},
                     {QStringLiteral("data"), QJsonArray::fromStringList(months)}}},
        {QStringLiteral("yAxis"), yAxis},
        {QStringLiteral("series"), series}};
}

QJsonObject pieOption()
{
    return {
        {QStringLiteral("title"),
         QJsonObject{{QStringLiteral("text"), tx("ChartInterface", "Traffic source")},
                     {QStringLiteral("left"), QStringLiteral("center")}}},
        {QStringLiteral("tooltip"), QJsonObject{{QStringLiteral("trigger"), QStringLiteral("item")}}},
        {QStringLiteral("legend"), QJsonObject{{QStringLiteral("bottom"), 0}, {QStringLiteral("left"), QStringLiteral("center")}}},
        {QStringLiteral("series"),
         QJsonArray{QJsonObject{
             {QStringLiteral("name"), tx("ChartInterface", "Visits")},
             {QStringLiteral("type"), QStringLiteral("pie")},
             {QStringLiteral("radius"), QJsonArray{QStringLiteral("40%"), QStringLiteral("70%")}},
             {QStringLiteral("avoidLabelOverlap"), false},
             {QStringLiteral("data"),
              QJsonArray{QJsonObject{{QStringLiteral("value"), 1048}, {QStringLiteral("name"), tx("ChartInterface", "Search")}},
                         QJsonObject{{QStringLiteral("value"), 735}, {QStringLiteral("name"), tx("ChartInterface", "Direct")}},
                         QJsonObject{{QStringLiteral("value"), 580}, {QStringLiteral("name"), tx("ChartInterface", "Email")}},
                         QJsonObject{{QStringLiteral("value"), 484}, {QStringLiteral("name"), tx("ChartInterface", "Ads")}},
                         QJsonObject{{QStringLiteral("value"), 300}, {QStringLiteral("name"), tx("ChartInterface", "Video")}}}}}}}};
}

QJsonObject scatterOption()
{
    return {
        {QStringLiteral("title"),
         QJsonObject{{QStringLiteral("text"), tx("ChartInterface", "Satisfaction by response time")},
                     {QStringLiteral("left"), QStringLiteral("center")}}},
        {QStringLiteral("tooltip"), QJsonObject{{QStringLiteral("trigger"), QStringLiteral("item")}}},
        {QStringLiteral("grid"),
         QJsonObject{{QStringLiteral("left"), QStringLiteral("3%")},
                     {QStringLiteral("right"), QStringLiteral("4%")},
                     {QStringLiteral("bottom"), QStringLiteral("3%")},
                     {QStringLiteral("containLabel"), true}}},
        {QStringLiteral("xAxis"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("value")},
                     {QStringLiteral("name"), tx("ChartInterface", "Response time")}}},
        {QStringLiteral("yAxis"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("value")},
                     {QStringLiteral("name"), tx("ChartInterface", "Score")}}},
        {QStringLiteral("series"),
         QJsonArray{QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Tickets")},
                                {QStringLiteral("type"), QStringLiteral("scatter")},
                                {QStringLiteral("symbolSize"), 14},
                                {QStringLiteral("data"),
                                 QJsonArray{QJsonArray{8, 96}, QJsonArray{15, 89}, QJsonArray{21, 82},
                                            QJsonArray{24, 76}, QJsonArray{31, 73}, QJsonArray{39, 65},
                                            QJsonArray{44, 61}, QJsonArray{53, 52}, QJsonArray{61, 47}}}}}}};
}

QJsonObject radarOption()
{
    return {
        {QStringLiteral("title"),
         QJsonObject{{QStringLiteral("text"), tx("ChartInterface", "Product capability")},
                     {QStringLiteral("left"), QStringLiteral("center")}}},
        {QStringLiteral("tooltip"), QJsonObject{}},
        {QStringLiteral("legend"), QJsonObject{{QStringLiteral("bottom"), 0}}},
        {QStringLiteral("radar"),
         QJsonObject{{QStringLiteral("indicator"),
                      QJsonArray{QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Design")},
                                             {QStringLiteral("max"), 100}},
                                 QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Performance")},
                                             {QStringLiteral("max"), 100}},
                                 QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Stability")},
                                             {QStringLiteral("max"), 100}},
                                 QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Docs")},
                                             {QStringLiteral("max"), 100}},
                                 QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Ecosystem")},
                                             {QStringLiteral("max"), 100}}}}}},
        {QStringLiteral("series"),
         QJsonArray{QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Score")},
                                {QStringLiteral("type"), QStringLiteral("radar")},
                                {QStringLiteral("data"),
                                 QJsonArray{QJsonObject{{QStringLiteral("value"), QJsonArray{92, 88, 84, 76, 80}},
                                                        {QStringLiteral("name"), tx("ChartInterface", "Qt")}},
                                            QJsonObject{{QStringLiteral("value"), QJsonArray{84, 82, 78, 88, 90}},
                                                        {QStringLiteral("name"), tx("ChartInterface", "Web")}}}}}}}};
}

QJsonObject gaugeOption()
{
    return {
        {QStringLiteral("title"),
         QJsonObject{{QStringLiteral("text"), tx("ChartInterface", "Deployment health")},
                     {QStringLiteral("left"), QStringLiteral("center")}}},
        {QStringLiteral("series"),
         QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("gauge")},
                                {QStringLiteral("progress"), QJsonObject{{QStringLiteral("show"), true}}},
                                {QStringLiteral("axisLine"),
                                 QJsonObject{{QStringLiteral("lineStyle"),
                                              QJsonObject{{QStringLiteral("width"), 16}}}}},
                                {QStringLiteral("detail"),
                                 QJsonObject{{QStringLiteral("valueAnimation"), true},
                                             {QStringLiteral("formatter"), QStringLiteral("{value}%")}}},
                                {QStringLiteral("data"),
                                 QJsonArray{QJsonObject{{QStringLiteral("value"), 86},
                                                        {QStringLiteral("name"), tx("ChartInterface", "Healthy")}}}}}}}};
}

QJsonObject heatmapOption()
{
    const QStringList hours{QStringLiteral("10:00"), QStringLiteral("12:00"), QStringLiteral("14:00"),
                            QStringLiteral("16:00"), QStringLiteral("18:00"), QStringLiteral("20:00")};
    const QStringList weekdays{tx("ChartInterface", "Mon"), tx("ChartInterface", "Tue"), tx("ChartInterface", "Wed"),
                               tx("ChartInterface", "Thu"), tx("ChartInterface", "Fri")};
    return {
        {QStringLiteral("title"),
         QJsonObject{{QStringLiteral("text"), tx("ChartInterface", "Activity heatmap")},
                     {QStringLiteral("left"), QStringLiteral("center")}}},
        {QStringLiteral("tooltip"), QJsonObject{{QStringLiteral("position"), QStringLiteral("top")}}},
        {QStringLiteral("grid"),
         QJsonObject{{QStringLiteral("top"), 64},
                     {QStringLiteral("left"), QStringLiteral("8%")},
                     {QStringLiteral("right"), QStringLiteral("8%")},
                     {QStringLiteral("bottom"), 64}}},
        {QStringLiteral("xAxis"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("category")},
                     {QStringLiteral("data"), QJsonArray::fromStringList(hours)}}},
        {QStringLiteral("yAxis"),
         QJsonObject{{QStringLiteral("type"), QStringLiteral("category")},
                     {QStringLiteral("data"), QJsonArray::fromStringList(weekdays)}}},
        {QStringLiteral("visualMap"),
         QJsonObject{{QStringLiteral("min"), 0},
                     {QStringLiteral("max"), 10},
                     {QStringLiteral("calculable"), true},
                     {QStringLiteral("orient"), QStringLiteral("horizontal")},
                     {QStringLiteral("left"), QStringLiteral("center")},
                     {QStringLiteral("bottom"), 10}}},
        {QStringLiteral("series"),
         QJsonArray{QJsonObject{{QStringLiteral("name"), tx("ChartInterface", "Activity")},
                                {QStringLiteral("type"), QStringLiteral("heatmap")},
                                {QStringLiteral("data"),
                                 QJsonArray{QJsonArray{0, 0, 3}, QJsonArray{1, 0, 5}, QJsonArray{2, 0, 7},
                                            QJsonArray{3, 0, 6}, QJsonArray{4, 0, 4}, QJsonArray{5, 0, 2},
                                            QJsonArray{0, 1, 4}, QJsonArray{1, 1, 7}, QJsonArray{2, 1, 9},
                                            QJsonArray{3, 1, 8}, QJsonArray{4, 1, 6}, QJsonArray{5, 1, 3},
                                            QJsonArray{0, 2, 2}, QJsonArray{1, 2, 4}, QJsonArray{2, 2, 6},
                                            QJsonArray{3, 2, 8}, QJsonArray{4, 2, 7}, QJsonArray{5, 2, 5},
                                            QJsonArray{0, 3, 1}, QJsonArray{1, 3, 3}, QJsonArray{2, 3, 5},
                                            QJsonArray{3, 3, 7}, QJsonArray{4, 3, 8}, QJsonArray{5, 3, 6},
                                            QJsonArray{0, 4, 2}, QJsonArray{1, 4, 6}, QJsonArray{2, 4, 10},
                                            QJsonArray{3, 4, 9}, QJsonArray{4, 4, 7}, QJsonArray{5, 4, 4}}},
                                {QStringLiteral("label"), QJsonObject{{QStringLiteral("show"), true}}},
                                {QStringLiteral("emphasis"),
                                 QJsonObject{{QStringLiteral("itemStyle"),
                                              QJsonObject{{QStringLiteral("shadowBlur"), 10},
                                                          {QStringLiteral("shadowColor"),
                                                           QStringLiteral("rgba(0, 0, 0, 0.35)")}}}}}}}}};
}

ChartWidget *createChart(const QJsonObject &option, QWidget *parent, int minimumHeight = 320)
{
    auto *chart = new ChartWidget(option, parent);
    chart->setMinimumHeight(minimumHeight);
    return chart;
}

} // namespace

QWidget *GalleryWindow::createChartPage()
{
    auto *page = new GalleryInterface(navTx("Charts"), QStringLiteral("qfluentwidgets.components.widgets"), this);
    page->setObjectName(QStringLiteral("chartInterface"));
    const QString chartSource = QStringLiteral(FQW_REPOSITORY_URL "/blob/main/include/FluentQtWidgets/Widgets/ChartWidget.h");
    const QString waveformSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/include/FluentQtWidgets/Widgets/AudioWaveformWidget.h");
    const QString realtimeSource =
        QStringLiteral(FQW_REPOSITORY_URL "/blob/main/include/FluentQtWidgets/Widgets/RealtimePlotWidget.h");

    page->addExampleCard(tx("ChartInterface", "Audio waveform widget"), createAudioWaveform(page), waveformSource, 1);
    page->addExampleCard(tx("ChartInterface", "Realtime multi-series plot"), createRealtimePlot(page), realtimeSource, 1);
    page->addExampleCard(tx("ChartInterface", "Native bar chart"), createChart(barOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Native line chart"), createChart(lineOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Native stacked area chart"), createChart(areaOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Native mixed bar and line chart"), createChart(mixedOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Native donut chart"), createChart(pieOption(), page), chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Native scatter chart"), createChart(scatterOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Native radar chart"), createChart(radarOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Native gauge chart"), createChart(gaugeOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Native heatmap"), createChart(heatmapOption(), page, 360),
                         chartSource, 1);

    return page;
}
