#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QVector>

#include <cmath>

using namespace FluentQt;

namespace {

QVector<qreal> waveformSamples()
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

AudioWaveformWidget *createAudioWaveform(QWidget *parent)
{
    auto *waveform = new AudioWaveformWidget(parent);
    waveform->setSamples(waveformSamples());
    waveform->setProgress(0.53);
    waveform->setMinimumHeight(220);
    waveform->setBarWidth(2.0);
    waveform->setBarGap(3.0);
    return waveform;
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
                           {QStringLiteral("Mon"), QStringLiteral("Tue"), QStringLiteral("Wed"), QStringLiteral("Thu"),
                            QStringLiteral("Fri"), QStringLiteral("Sat"), QStringLiteral("Sun")},
                           QJsonArray{120, 200, 150, 80, 70, 110, 130},
                           tx("ChartInterface", "Users"), QStringLiteral("bar"));
}

QJsonObject lineOption()
{
    return axisChartOption(tx("ChartInterface", "Revenue trend"),
                           {QStringLiteral("Jan"), QStringLiteral("Feb"), QStringLiteral("Mar"), QStringLiteral("Apr"),
                            QStringLiteral("May"), QStringLiteral("Jun")},
                           QJsonArray{820, 932, 901, 934, 1290, 1330},
                           tx("ChartInterface", "Revenue"), QStringLiteral("line"));
}

QJsonObject areaOption()
{
    const QStringList days{QStringLiteral("Mon"), QStringLiteral("Tue"), QStringLiteral("Wed"), QStringLiteral("Thu"),
                           QStringLiteral("Fri"), QStringLiteral("Sat"), QStringLiteral("Sun")};
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
    const QStringList months{QStringLiteral("Jan"), QStringLiteral("Feb"), QStringLiteral("Mar"), QStringLiteral("Apr"),
                             QStringLiteral("May"), QStringLiteral("Jun")};
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
              QJsonArray{QJsonObject{{QStringLiteral("value"), 1048}, {QStringLiteral("name"), QStringLiteral("Search")}},
                         QJsonObject{{QStringLiteral("value"), 735}, {QStringLiteral("name"), QStringLiteral("Direct")}},
                         QJsonObject{{QStringLiteral("value"), 580}, {QStringLiteral("name"), QStringLiteral("Email")}},
                         QJsonObject{{QStringLiteral("value"), 484}, {QStringLiteral("name"), QStringLiteral("Ads")}},
                         QJsonObject{{QStringLiteral("value"), 300}, {QStringLiteral("name"), QStringLiteral("Video")}}}}}}}};
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
                                                        {QStringLiteral("name"), QStringLiteral("Qt")}},
                                            QJsonObject{{QStringLiteral("value"), QJsonArray{84, 82, 78, 88, 90}},
                                                        {QStringLiteral("name"), QStringLiteral("Web")}}}}}}}};
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
    const QStringList weekdays{QStringLiteral("Mon"), QStringLiteral("Tue"), QStringLiteral("Wed"), QStringLiteral("Thu"),
                               QStringLiteral("Fri")};
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

    page->addExampleCard(tx("ChartInterface", "Audio waveform widget"), createAudioWaveform(page), waveformSource, 1);
    page->addExampleCard(tx("ChartInterface", "Bar chart powered by ECharts"), createChart(barOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Line chart powered by ECharts"), createChart(lineOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Stacked area chart powered by ECharts"), createChart(areaOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Mixed bar and line chart powered by ECharts"),
                         createChart(mixedOption(), page), chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Donut chart powered by ECharts"), createChart(pieOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Scatter chart powered by ECharts"), createChart(scatterOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Radar chart powered by ECharts"), createChart(radarOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Gauge chart powered by ECharts"), createChart(gaugeOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Heatmap powered by ECharts"), createChart(heatmapOption(), page, 360),
                         chartSource, 1);

    return page;
}
