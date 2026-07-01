#include <FluentQtWidgets/Widgets/ChartWidget.h>

#include <FluentQtWidgets/Theme.h>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtCore/QFile>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

#if defined(FQW_HAS_WEBENGINE_WIDGETS)
#include <QtWebEngineCore/QWebEnginePage>
#include <QtWebEngineWidgets/QWebEngineView>
#endif

namespace FluentQt {

namespace {

QString colorName(const QColor &color)
{
    return color.name(QColor::HexRgb);
}

QString chartBackgroundColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QStringLiteral("#202020")
                                                                     : QStringLiteral("#f9f9f9");
}

QString chartTextColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QStringLiteral("#f3f3f3")
                                                                     : QStringLiteral("#1f1f1f");
}

QString bundledEChartsScript()
{
    QFile file(QStringLiteral(":/qfluentwidgets/chart/echarts.min.js"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }
    QString script = QString::fromUtf8(file.readAll());
    script.replace(QStringLiteral("</script>"), QStringLiteral("<\\/script>"));
    return script;
}

QString htmlWithOption(const QJsonObject &option, const QString &theme)
{
    QString optionJson = QString::fromUtf8(QJsonDocument(option).toJson(QJsonDocument::Compact));
    optionJson.replace(QStringLiteral("</script>"), QStringLiteral("<\\/script>"));

    const QString background = chartBackgroundColor();
    const QString text = chartTextColor();
    const QString accent = colorName(ThemeManager::instance()->accentColor());
    const QString chartTheme = theme == QStringLiteral("dark") ? QStringLiteral("\"dark\"") : QStringLiteral("null");
    const QString echartsScript = bundledEChartsScript();

    QString html = QStringLiteral(R"HTML(<!doctype html>
<html>
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html, body, #chart {
      width: 100%;
      height: 100%;
      margin: 0;
      overflow: hidden;
      background: __BACKGROUND_COLOR__;
      color: __TEXT_COLOR__;
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    }
    #fallback {
      display: none;
      box-sizing: border-box;
      padding: 24px;
      color: __TEXT_COLOR__;
      font-size: 13px;
      line-height: 1.5;
    }
  </style>
  <script>__ECHARTS_SCRIPT__</script>
</head>
<body>
  <div id="chart"></div>
  <div id="fallback">ECharts could not be loaded from the Qt resource system. Rebuild the qfluentwidgets resources and reload this chart.</div>
  <script>
    const initialOption = __OPTION_JSON__;
    const accentColor = "__ACCENT_COLOR__";
    let currentOption = initialOption;
    let chart = null;

    function defaultize(option) {
      const merged = Object.assign({
        color: [accentColor, "#16a34a", "#f59e0b", "#ef4444", "#8b5cf6"],
        textStyle: { color: "__TEXT_COLOR__" },
        backgroundColor: "transparent"
      }, option || {});
      return merged;
    }

    function showFallback(message) {
      document.getElementById("chart").style.display = "none";
      const fallback = document.getElementById("fallback");
      if (message) {
        fallback.textContent = message;
      }
      fallback.style.display = "block";
    }

    function resizeChart() {
      if (chart) {
        chart.resize();
      }
    }

    function initializeChart() {
      if (!window.echarts) {
        showFallback();
        return;
      }

      try {
        chart = echarts.init(document.getElementById("chart"), __CHART_THEME__, { renderer: "canvas" });
        chart.setOption(defaultize(currentOption), true);
        resizeChart();
        requestAnimationFrame(resizeChart);
        setTimeout(resizeChart, 100);
        setTimeout(resizeChart, 500);
        window.addEventListener("resize", resizeChart);
      } catch (error) {
        showFallback("ECharts failed to initialize: " + error);
        console.error(error);
      }
    }

    window.setChartOption = function(option, notMerge) {
      currentOption = option || {};
      if (chart) {
        chart.setOption(defaultize(currentOption), notMerge !== false);
        resizeChart();
      }
    };

    window.resizeChart = resizeChart;

    window.addEventListener("load", initializeChart);
    setTimeout(() => {
      if (!chart && !window.echarts) {
        showFallback();
      }
    }, 6000);
  </script>
</body>
</html>)HTML");

    html.replace(QStringLiteral("__BACKGROUND_COLOR__"), background);
    html.replace(QStringLiteral("__TEXT_COLOR__"), text);
    html.replace(QStringLiteral("__ECHARTS_SCRIPT__"), echartsScript);
    html.replace(QStringLiteral("__OPTION_JSON__"), optionJson);
    html.replace(QStringLiteral("__ACCENT_COLOR__"), accent);
    html.replace(QStringLiteral("__CHART_THEME__"), chartTheme);
    return html;
}

#if !defined(FQW_HAS_WEBENGINE_WIDGETS)
QString unavailableText()
{
    return QObject::tr("ChartWidget requires Qt WebEngineWidgets. Reconfigure Qt with WebEngine support to render ECharts.");
}
#endif

} // namespace

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    init();
}

ChartWidget::ChartWidget(const QJsonObject &option, QWidget *parent)
    : QWidget(parent)
    , m_option(option)
{
    init();
}

QJsonObject ChartWidget::option() const { return m_option; }

QString ChartWidget::chartTheme() const { return m_chartTheme; }

QSize ChartWidget::sizeHint() const { return QSize(520, 320); }

void ChartWidget::setOption(const QJsonObject &option)
{
    if (m_option == option) {
        return;
    }
    m_option = option;
    applyPendingOption();
    emit optionChanged(m_option);
}

void ChartWidget::setOptionJson(const QString &json)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return;
    }
    setOption(document.object());
}

void ChartWidget::setChartTheme(const QString &theme)
{
    const QString normalized = theme.trimmed().isEmpty() ? QStringLiteral("auto") : theme.trimmed().toLower();
    if (m_chartTheme == normalized) {
        return;
    }
    m_chartTheme = normalized;
    reload();
    emit chartThemeChanged(m_chartTheme);
}

void ChartWidget::reload()
{
    m_loaded = false;
    renderChart();
}

void ChartWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    resizeChart();
}

void ChartWidget::init()
{
    setAttribute(Qt::WA_StyledBackground, true);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

#if defined(FQW_HAS_WEBENGINE_WIDGETS)
    auto *webView = new QWebEngineView(this);
    webView->setContextMenuPolicy(Qt::NoContextMenu);
    webView->setAttribute(Qt::WA_StyledBackground, false);
    m_view = webView;
    layout->addWidget(webView);

    connect(webView, &QWebEngineView::loadFinished, this, [this](bool ok) {
        m_loaded = ok;
        if (ok) {
            applyPendingOption();
            resizeChart();
            QTimer::singleShot(100, this, &ChartWidget::resizeChart);
            QTimer::singleShot(500, this, &ChartWidget::resizeChart);
        }
        emit loadFinished(ok);
    });
    connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged, this, [this](Theme) {
        if (m_chartTheme == QStringLiteral("auto")) {
            reload();
        }
    });
    connect(ThemeManager::instance(), &ThemeManager::accentColorChanged, this, [this](const QColor &) {
        reload();
    });
#else
    auto *label = new QLabel(unavailableText(), this);
    label->setWordWrap(true);
    label->setAlignment(Qt::AlignCenter);
    m_view = label;
    layout->addWidget(label);
#endif

    QTimer::singleShot(0, this, &ChartWidget::renderChart);
}

void ChartWidget::renderChart()
{
#if defined(FQW_HAS_WEBENGINE_WIDGETS)
    auto *webView = qobject_cast<QWebEngineView *>(m_view);
    if (!webView) {
        return;
    }
    webView->setHtml(htmlWithOption(m_option, resolvedChartTheme()),
                     QUrl(QStringLiteral("https://cdn.jsdelivr.net/npm/echarts@5/dist/")));
#else
    if (m_view) {
        m_view->update();
    }
#endif
}

void ChartWidget::applyPendingOption()
{
#if defined(FQW_HAS_WEBENGINE_WIDGETS)
    if (!m_loaded) {
        return;
    }

    auto *webView = qobject_cast<QWebEngineView *>(m_view);
    if (!webView || !webView->page()) {
        return;
    }

    QString optionJson = QString::fromUtf8(QJsonDocument(m_option).toJson(QJsonDocument::Compact));
    optionJson.replace(QStringLiteral("</script>"), QStringLiteral("<\\/script>"));
    webView->page()->runJavaScript(QStringLiteral("window.setChartOption(%1, true);").arg(optionJson));
#endif
}

void ChartWidget::resizeChart()
{
#if defined(FQW_HAS_WEBENGINE_WIDGETS)
    if (!m_loaded) {
        return;
    }

    auto *webView = qobject_cast<QWebEngineView *>(m_view);
    if (!webView || !webView->page()) {
        return;
    }
    webView->page()->runJavaScript(QStringLiteral("window.resizeChart && window.resizeChart();"));
#else
    if (m_view) {
        m_view->update();
    }
#endif
}

QString ChartWidget::resolvedChartTheme() const
{
    if (m_chartTheme != QStringLiteral("auto")) {
        return m_chartTheme;
    }
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QStringLiteral("dark") : QStringLiteral("light");
}

} // namespace FluentQt
