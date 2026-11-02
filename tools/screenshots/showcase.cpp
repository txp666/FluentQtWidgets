// README showcase: a "device console" window built from FluentQtWidgets widgets.
// usage: showcase <zh_CN|en> <light|dark> <out.jpg>
#include "GalleryTranslator.h"
#include "shotutil.h"

#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>
#include <QtGui/QCursor>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QVBoxLayout>

#include <cmath>

using namespace FluentQt;

static bool g_zh = true;
static QString T(const char *zh, const char *en) { return g_zh ? QString::fromUtf8(zh) : QString::fromUtf8(en); }

static HeaderCardWidget *makeCard(const QString &title, QWidget *content, QWidget *parent)
{
    auto *card = new HeaderCardWidget(title, parent);
    card->viewLayout()->addWidget(content);
    QFont f = card->titleLabel()->font();
    f.setPixelSize(20);
    card->titleLabel()->setFont(f);
    return card;
}

static QWidget *buttonsContent()
{
    auto *w = new QWidget;
    auto *grid = new QGridLayout(w);
    grid->setContentsMargins(0, 8, 0, 8);
    grid->setHorizontalSpacing(10);
    grid->setVerticalSpacing(16);
    auto *connect = new PrimaryPushButton(icon(FluentIcon::Connect), T("开始连接", "Connect"));
    auto *exportLog = new PushButton(icon(FluentIcon::Save), T("导出日志", "Export log"));
    auto *send = new DropDownPushButton(icon(FluentIcon::Send), T("发送", "Send"));
    auto *autoScroll = new ToggleButton(T("自动滚动", "Auto scroll"));
    autoScroll->setChecked(true);
    for (QWidget *b : {static_cast<QWidget *>(connect), static_cast<QWidget *>(exportLog), static_cast<QWidget *>(send),
                       static_cast<QWidget *>(autoScroll)})
        b->setMinimumWidth(150);
    grid->addWidget(connect, 0, 0);
    grid->addWidget(exportLog, 0, 1);
    grid->addWidget(send, 1, 0);
    grid->addWidget(autoScroll, 1, 1);

    auto *pills = new QHBoxLayout;
    pills->setSpacing(8);
    auto *live = new PillPushButton(T("实时", "Live"));
    live->setChecked(true);
    auto *history = new PillPushButton(T("历史", "History"));
    auto *docs = new HyperlinkButton(QStringLiteral("https://github.com/txp666/FluentQtWidgets"), T("文档", "Docs"));
    pills->addWidget(live);
    pills->addWidget(history);
    pills->addWidget(docs);
    pills->addStretch(1);
    grid->addLayout(pills, 2, 0, 1, 2);
    return w;
}

static QWidget *inputsContent()
{
    auto *w = new QWidget;
    auto *grid = new QGridLayout(w);
    grid->setContentsMargins(0, 4, 0, 4);
    grid->setHorizontalSpacing(24);
    grid->setVerticalSpacing(12);
    auto *sw = new SwitchButton(w);
    sw->setOffText(T("自动重连", "Auto reconnect"));
    sw->setOnText(T("自动重连", "Auto reconnect"));
    sw->setChecked(false);
    auto *combo = new ComboBox(w);
    combo->addItems({QStringLiteral("115200"), QStringLiteral("57600"), QStringLiteral("38400"), QStringLiteral("9600")});
    combo->setCurrentIndex(0);
    auto *ts = new CheckBox(T("时间戳", "Timestamps"), w);
    ts->setChecked(true);
    auto *hex = new CheckBox(T("HEX 显示", "HEX view"), w);
    auto *utf8 = new RadioButton(QStringLiteral("UTF-8"), w);
    auto *gbk = new RadioButton(QStringLiteral("GBK"), w);
    auto *group = new QButtonGroup(w);
    group->addButton(utf8);
    group->addButton(gbk);
    utf8->setChecked(true);
    auto *slider = new Slider(Qt::Horizontal, w);
    slider->setRange(0, 100);
    slider->setValue(35);
    grid->addWidget(sw, 0, 0);
    grid->addWidget(combo, 0, 1);
    grid->addWidget(ts, 1, 0);
    grid->addWidget(hex, 1, 1);
    grid->addWidget(utf8, 2, 0);
    grid->addWidget(gbk, 2, 1);
    grid->addWidget(slider, 3, 0, 1, 2);
    grid->setColumnStretch(0, 1);
    grid->setColumnStretch(1, 1);
    return w;
}

static QWidget *statusContent()
{
    auto *w = new QWidget;
    auto *v = new QVBoxLayout(w);
    v->setContentsMargins(0, 4, 0, 4);
    v->setSpacing(14);
    auto *top = new QHBoxLayout;
    top->setSpacing(20);
    auto *ring = new ProgressRing(w, false);
    ring->setFixedSize(76, 76);
    ring->setRange(0, 100);
    ring->setValue(72);
    ring->setTextVisible(true);
    ring->setFormat(QStringLiteral("%p%"));
    top->addWidget(ring);

    auto *right = new QVBoxLayout;
    right->setSpacing(10);
    auto *badges = new QHBoxLayout;
    badges->setSpacing(6);
    badges->addWidget(InfoBadge::attention(8, w));
    badges->addWidget(InfoBadge::success(QStringLiteral("OK"), w));
    badges->addWidget(InfoBadge::warning(QStringLiteral("!"), w));
    badges->addStretch(1);
    right->addLayout(badges);
    auto *cpuRow = new QHBoxLayout;
    cpuRow->setSpacing(14);
    auto *spinner = new IndeterminateProgressRing(w);
    spinner->setFixedSize(28, 28);
    spinner->setStrokeWidth(3);
    cpuRow->addWidget(spinner);
    cpuRow->addWidget(new BodyLabel(QStringLiteral("CPU 72%"), w));
    cpuRow->addStretch(1);
    right->addLayout(cpuRow);
    top->addLayout(right, 1);
    v->addLayout(top);

    auto *bar = new ProgressBar(w, false);
    bar->setRange(0, 100);
    bar->setValue(64);
    v->addWidget(bar);
    auto *busy = new IndeterminateProgressBar(w);
    v->addWidget(busy);
    return w;
}

static qreal wave(qreal t, int s)
{
    const qreal base[] = {50.0, 46.0, 40.0};
    return qBound<qreal>(9.0, base[s] + 22.0 * std::sin(t * 0.016 + s * 1.9) + 8.0 * std::sin(t * 0.043 + s * 0.7) +
                                  4.0 * std::sin(t * 0.11 + s),
                         84.0);
}

static QWidget *plotContent(int samples)
{
    auto *plot = new RealtimePlotWidget;
    plot->setMinimumHeight(220);
    plot->setCapacity(5000);
    plot->setSeriesName(0, T("温度", "Temperature"));
    plot->setSeriesColor(0, QColor(15, 108, 189));
    const int hum = plot->addSeries(T("湿度", "Humidity"), QColor(22, 163, 74));
    const int volt = plot->addSeries(T("电压", "Voltage"), QColor(245, 158, 11));
    QVector<qreal> a, b, c;
    for (int i = 0; i < samples; ++i) {
        a.append(wave(i, 0));
        b.append(wave(i, 1));
        c.append(wave(i, 2));
    }
    plot->setSamples(0, a);
    plot->setSamples(hum, b);
    plot->setSamples(volt, c);
    return plot;
}

static QWidget *donutContent()
{
    QJsonArray data;
    const QList<QPair<QString, int>> items{{T("串口", "Serial"), 42}, {T("网络", "Network"), 28},
                                           {T("蓝牙", "Bluetooth"), 18}, {QStringLiteral("USB"), 12}};
    for (const auto &it : items)
        data.append(QJsonObject{{QStringLiteral("name"), it.first}, {QStringLiteral("value"), it.second}});
    const QJsonObject option{
        {QStringLiteral("tooltip"), QJsonObject{{QStringLiteral("trigger"), QStringLiteral("item")}}},
        {QStringLiteral("legend"), QJsonObject{{QStringLiteral("bottom"), 0}}},
        {QStringLiteral("series"),
         QJsonArray{QJsonObject{{QStringLiteral("type"), QStringLiteral("pie")},
                                {QStringLiteral("radius"), QJsonArray{QStringLiteral("40%"), QStringLiteral("70%")}},
                                {QStringLiteral("data"), data}}}}};
    auto *chart = new ChartWidget(option);
    chart->setMinimumHeight(220);
    return chart;
}

int main(int argc, char *argv[])
{
    QStandardPaths::setTestModeEnabled(true);
    QApplication app(argc, argv);
    const QStringList args = app.arguments();
    if (args.size() < 4) {
        std::fprintf(stderr, "usage: showcase <zh_CN|en> <light|dark> <out.jpg>\n");
        return 2;
    }
    g_zh = args.at(1) != QLatin1String("en");
    const bool dark = args.at(2) == QLatin1String("dark");
    app.setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    const QIcon logo(QStringLiteral(":/gallery/images/logo.png"));
    QApplication::setWindowIcon(logo);
    QLocale::setDefault(QLocale(g_zh ? QStringLiteral("zh_CN") : QStringLiteral("en_US")));
    GalleryTranslation::installTranslators(&app, g_zh ? QStringLiteral("zh_CN") : QStringLiteral("en"));
    ThemeManager::instance()->setTheme(dark ? Theme::Dark : Theme::Light);
    ThemeManager::instance()->setAccentColor(FluentConfig::instance()->themeColor());

    auto *window = new FluentWindow;
    window->setWindowTitle(QStringLiteral("FluentQtWidgets Showcase"));
    window->setWindowIcon(logo);

    auto *home = new QWidget;
    home->setObjectName(QStringLiteral("homeInterface"));
    auto *root = new QVBoxLayout(home);
    root->setContentsMargins(28, 16, 28, 22);
    root->setSpacing(16);

    auto *header = new QHBoxLayout;
    auto *titles = new QVBoxLayout;
    titles->setSpacing(4);
    titles->addWidget(new TitleLabel(T("设备控制台", "Device Console"), home));
    titles->addWidget(new CaptionLabel(T("由 FluentQtWidgets 构建 · C++17 · Qt 6 Widgets",
                                         "Built with FluentQtWidgets · C++17 · Qt 6 Widgets"),
                                       home));
    header->addLayout(titles);
    header->addStretch(1);
    auto *search = new SearchLineEdit(home);
    search->setPlaceholderText(T("搜索设备、日志或命令", "Search devices, logs or commands"));
    search->setFixedWidth(280);
    header->addWidget(search, 0, Qt::AlignVCenter);
    header->addSpacing(14);
    auto *themeBtn = new ToolButton(icon(FluentIcon::Brightness), home);
    themeBtn->setFixedSize(54, 48);
    header->addWidget(themeBtn, 0, Qt::AlignVCenter);
    root->addLayout(header);

    auto *bar = new InfoBar(InfoBarSeverity::Success, T("已连接", "Connected"),
                            T("COM3 · 115200 bps · 设备运行正常", "COM3 · 115200 bps · Device running normally"),
                            Qt::Horizontal, true, -1, InfoBarPosition::None, home);
    root->addWidget(bar);

    auto *row1 = new QHBoxLayout;
    row1->setSpacing(16);
    row1->addWidget(makeCard(T("按钮", "Buttons"), buttonsContent(), home), 4);
    row1->addWidget(makeCard(T("输入与选择", "Input & selection"), inputsContent(), home), 4);
    row1->addWidget(makeCard(T("状态", "Status"), statusContent(), home), 3);
    root->addLayout(row1);

    auto *row2 = new QHBoxLayout;
    row2->setSpacing(16);
    row2->addWidget(makeCard(T("实时曲线", "Live plot"), plotContent(dark ? 767 : 668), home), 7);
    row2->addWidget(makeCard(T("流量来源", "Traffic sources"), donutContent(), home), 4);
    root->addLayout(row2, 1);

    window->addSubInterface(home, icon(FluentIcon::Home), T("主页", "Home"), QStringLiteral("homeInterface"));
    const QList<QPair<FluentIcon, QString>> others{{FluentIcon::PieSingle, T("统计", "Statistics")},
                                                   {FluentIcon::IOT, T("设备", "Devices")},
                                                   {FluentIcon::Chat, T("日志", "Logs")}};
    int n = 0;
    for (const auto &o : others) {
        auto *page = new QWidget;
        page->setObjectName(QStringLiteral("page%1").arg(n++));
        window->addSubInterface(page, icon(o.first), o.second, page->objectName());
    }
    auto *settings = new QWidget;
    settings->setObjectName(QStringLiteral("settingsInterface"));
    window->addSubInterface(settings, icon(FluentIcon::Setting), T("设置", "Settings"), settings->objectName(),
                            NavigationItemPosition::Bottom);

    window->setGeometry(100, 60, 1280, 800);
    window->show();
    QCursor::setPos(1590, 990);
    window->switchTo(home);
    waitMs(2200);
    return saveShot(grabComposite(window, {}), args.at(3)) ? 0 : 1;
}
