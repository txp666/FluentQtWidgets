#include "../../GalleryWindow.h"

#include "GalleryViewHelpers.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

using namespace FluentQt;

namespace {

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

ChartWidget *createChart(const QJsonObject &option, QWidget *parent)
{
    auto *chart = new ChartWidget(option, parent);
    chart->setMinimumHeight(320);
    return chart;
}

} // namespace

QWidget *GalleryWindow::createChartPage()
{
    auto *page = new GalleryInterface(navTx("Charts"), QStringLiteral("qfluentwidgets.components.widgets"), this);
    page->setObjectName(QStringLiteral("chartInterface"));
    const QString chartSource = QStringLiteral(FQW_REPOSITORY_URL "/blob/main/include/FluentQtWidgets/Widgets/ChartWidget.h");

    page->addExampleCard(tx("ChartInterface", "Bar chart powered by ECharts"), createChart(barOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Line chart powered by ECharts"), createChart(lineOption(), page),
                         chartSource, 1);
    page->addExampleCard(tx("ChartInterface", "Donut chart powered by ECharts"), createChart(pieOption(), page),
                         chartSource, 1);

    return page;
}
