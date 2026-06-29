#include <FluentQtWidgets/DateTime/CalendarPicker.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QLocale>
#include <QtCore/QPoint>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QCloseEvent>
#include <QtGui/QPaintEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QPainter>
#include <QtGui/QScreen>
#include <QtGui/QShowEvent>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

CalendarView::CalendarView(QWidget *parent) : CalendarView(parent, true) {}

CalendarView::CalendarView(QWidget *parent, bool popup)
    : QFrame(parent, popup ? (Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint) : Qt::Widget)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_DeleteOnClose, false);
    FluentStyleSheet::setRole(this, QStringLiteral("CalendarView"));

    auto *effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);
    effect->setOpacity(0);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(8);

    auto *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(4, 0, 4, 0);
    headerLayout->setSpacing(4);

    m_titleButton = new QPushButton(this);
    m_titleButton->setCursor(Qt::PointingHandCursor);
    m_titleButton->setFlat(true);
    m_titleButton->setProperty("calendarTitle", true);
    m_titleButton->setObjectName(QStringLiteral("titleButton"));
    m_titleButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(m_titleButton, &QPushButton::clicked, this, &CalendarView::titleClicked);

    m_resetButton = createNavigationButton(FluentQt::icon(FluentIcon::Close), QStringLiteral("resetButton"));
    m_upButton = createNavigationButton(FluentQt::icon(FluentIcon::Back), QStringLiteral("upButton"));
    m_downButton = createNavigationButton(FluentQt::icon(FluentIcon::Forward), QStringLiteral("downButton"));
    connect(m_resetButton, &QToolButton::clicked, this, &CalendarView::reset);
    connect(m_upButton, &QToolButton::clicked, this, &CalendarView::scrollUp);
    connect(m_downButton, &QToolButton::clicked, this, &CalendarView::scrollDown);

    headerLayout->addWidget(m_titleButton);
    headerLayout->addWidget(m_resetButton);
    headerLayout->addWidget(m_upButton);
    headerLayout->addWidget(m_downButton);
    layout->addLayout(headerLayout);

    auto *gridLayout = new QGridLayout;
    gridLayout->setContentsMargins(0, 0, 0, 0);
    gridLayout->setHorizontalSpacing(2);
    gridLayout->setVerticalSpacing(2);

    const int firstDay = static_cast<int>(m_locale.firstDayOfWeek());
    for (int column = 0; column < 7; ++column) {
        const int day = ((firstDay - 1 + column) % 7) + 1;
        auto *label = new QLabel(m_locale.standaloneDayName(day, QLocale::ShortFormat), this);
        label->setAlignment(Qt::AlignCenter);
        label->setProperty("calendarWeekday", true);
        m_weekdayLabels[column] = label;
        gridLayout->addWidget(label, 0, column);
    }

    for (int i = 0; i < 42; ++i) {
        auto *button = new QPushButton(this);
        button->setCursor(Qt::PointingHandCursor);
        button->setFlat(true);
        button->setMinimumSize(36, 32);
        button->setProperty("calendarDay", true);
        connect(button, &QPushButton::clicked, this, [this, i]() {
            const QDate date = m_dayDates[i];
            if (!date.isValid()) {
                return;
            }
            if (m_date != date) {
                m_date = date;
                emit dateChanged(date);
            }
            hide();
        });
        m_dayButtons[i] = button;
        gridLayout->addWidget(button, 1 + i / 7, i % 7);
    }

    layout->addLayout(gridLayout);
    setResetEnabled(false);
    setDate(QDate());
}

QDate CalendarView::date() const { return m_date; }

QDate CalendarView::currentPageDate() const { return m_displayMonth; }

QLocale CalendarView::locale() const { return m_locale; }

bool CalendarView::isResetEnabled() const { return m_resetButton && !m_resetButton->isHidden(); }

QString CalendarView::title() const { return m_titleButton ? m_titleButton->text() : QString(); }

QPushButton *CalendarView::titleButton() const { return m_titleButton; }

QToolButton *CalendarView::resetButton() const { return m_resetButton; }

QToolButton *CalendarView::upButton() const { return m_upButton; }

QToolButton *CalendarView::downButton() const { return m_downButton; }

QList<QPushButton *> CalendarView::dayButtons() const
{
    QList<QPushButton *> buttons;
    buttons.reserve(42);
    for (QPushButton *button : m_dayButtons) {
        buttons.append(button);
    }
    return buttons;
}

void CalendarView::exec(const QPoint &pos, bool animated)
{
    adjustSize();
    move(clampedPosition(pos));
    if (!animated) {
        show();
        raise();
        return;
    }
    show();
    raise();
}

void CalendarView::setDate(const QDate &date)
{
    m_date = date;
    const QDate baseDate = date.isValid() ? date : QDate::currentDate();
    if (!m_displayMonth.isValid() || date.isValid()) {
        m_displayMonth = QDate(baseDate.year(), baseDate.month(), 1);
    }
    refreshCalendar();
}

void CalendarView::setLocale(const QLocale &locale)
{
    m_locale = locale;
    refreshCalendar();
}

void CalendarView::setResetEnabled(bool enabled)
{
    if (m_resetButton) {
        m_resetButton->setVisible(enabled);
    }
}

void CalendarView::reset()
{
    m_date = QDate();
    emit resetted();
    hide();
    refreshCalendar();
}

void CalendarView::scrollUp()
{
    m_displayMonth = m_displayMonth.addMonths(-1);
    refreshCalendar();
}

void CalendarView::scrollDown()
{
    m_displayMonth = m_displayMonth.addMonths(1);
    refreshCalendar();
}

void CalendarView::scrollToDate(const QDate &date)
{
    if (!date.isValid()) {
        return;
    }
    m_displayMonth = QDate(date.year(), date.month(), 1);
    refreshCalendar();
}

void CalendarView::showEvent(QShowEvent *event)
{
    QFrame::showEvent(event);

    if (auto *effect = qobject_cast<QGraphicsOpacityEffect *>(graphicsEffect())) {
        effect->setOpacity(0);
        auto *ani = new QPropertyAnimation(effect, "opacity", this);
        ani->setDuration(200);
        ani->setStartValue(0);
        ani->setEndValue(1);
        ani->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void CalendarView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    painter.setPen(dark ? QColor(255, 255, 255, 26) : QColor(0, 0, 0, 26));
    painter.setBrush(dark ? QColor(37, 37, 37) : QColor(255, 255, 255));
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), 8, 8);
}

QToolButton *CalendarView::createNavigationButton(const QIcon &icon, const QString &objectName)
{
    auto *button = new QToolButton(this);
    button->setCursor(Qt::PointingHandCursor);
    button->setIcon(icon);
    button->setProperty("calendarNavigation", true);
    button->setObjectName(objectName);
    button->setAutoRaise(true);
    button->setFixedSize(32, 32);
    return button;
}

void CalendarView::setTitle(const QString &title)
{
    if (m_titleButton) {
        m_titleButton->setText(title);
    }
}

void CalendarView::refreshCalendar()
{
    if (!m_displayMonth.isValid()) {
        const QDate today = QDate::currentDate();
        m_displayMonth = QDate(today.year(), today.month(), 1);
    }

    const int firstDay = static_cast<int>(m_locale.firstDayOfWeek());
    for (int column = 0; column < 7; ++column) {
        if (!m_weekdayLabels[column]) {
            continue;
        }
        const int day = ((firstDay - 1 + column) % 7) + 1;
        m_weekdayLabels[column]->setText(m_locale.standaloneDayName(day, QLocale::ShortFormat));
    }

    QString monthName = m_locale.standaloneMonthName(m_displayMonth.month(), QLocale::LongFormat);
    if (monthName.isEmpty()) {
        monthName = m_locale.monthName(m_displayMonth.month(), QLocale::LongFormat);
    }
    setTitle(QStringLiteral("%1 %2").arg(monthName).arg(m_displayMonth.year()));

    const int offset = (m_displayMonth.dayOfWeek() - firstDay + 7) % 7;
    const QDate startDate = m_displayMonth.addDays(-offset);
    const QDate today = QDate::currentDate();

    for (int i = 0; i < 42; ++i) {
        const QDate day = startDate.addDays(i);
        auto *button = m_dayButtons[i];
        m_dayDates[i] = day;
        button->setText(QString::number(day.day()));
        button->setProperty("outOfMonth", day.month() != m_displayMonth.month());
        button->setProperty("selected", m_date.isValid() && day == m_date);
        button->setProperty("today", day == today);
        FluentStyleSheet::polish(button);
    }
}

QPoint CalendarView::clampedPosition(const QPoint &preferred) const
{
    QScreen *screen = QGuiApplication::screenAt(preferred);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return preferred;
    }

    const QRect available = screen->availableGeometry();
    QPoint pos = preferred;
    pos.setX(qBound(available.left(), pos.x(), available.right() - width()));
    pos.setY(qBound(available.top(), pos.y(), available.bottom() - height()));
    return pos;
}

FastCalendarView::FastCalendarView(QWidget *parent) : FlyoutViewBase(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("FastCalendarView"));
    setAttribute(Qt::WA_TranslucentBackground, true);
    setFixedSize(314, 355);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);

    m_stackedWidget = new QStackedWidget(this);
    m_stackedWidget->setObjectName(QStringLiteral("stackedWidget"));
    m_layout->addWidget(m_stackedWidget);

    createDayPage();
    createMonthPage();
    createYearPage();

    m_stackedWidget->addWidget(m_dayView);
    m_stackedWidget->addWidget(m_monthView);
    m_stackedWidget->addWidget(m_yearView);

    const QDate today = QDate::currentDate();
    m_dayPageDate = QDate(today.year(), today.month(), 1);
    m_monthPageDate = QDate(today.year(), 1, 1);
    m_yearPageStart = today.year() - today.year() % 10;

    refreshResetButtons();
    refreshDayView();
    refreshMonthView();
    refreshYearView();
    switchToDayView();
}

QDate FastCalendarView::date() const { return m_date; }

bool FastCalendarView::isResetEnabled() const { return m_resetEnabled; }

QStackedWidget *FastCalendarView::stackedWidget() const { return m_stackedWidget; }

QWidget *FastCalendarView::dayView() const { return m_dayView; }

QWidget *FastCalendarView::monthView() const { return m_monthView; }

QWidget *FastCalendarView::yearView() const { return m_yearView; }

QList<QPushButton *> FastCalendarView::dayButtons() const { return m_dayButtons; }

QList<QPushButton *> FastCalendarView::monthButtons() const { return m_monthButtons; }

QList<QPushButton *> FastCalendarView::yearButtons() const { return m_yearButtons; }

void FastCalendarView::setDate(const QDate &date)
{
    m_date = date;
    const QDate page = date.isValid() ? date : QDate::currentDate();
    m_dayPageDate = QDate(page.year(), page.month(), 1);
    m_monthPageDate = QDate(page.year(), 1, 1);
    m_yearPageStart = page.year() - page.year() % 10;
    refreshDayView();
    refreshMonthView();
    refreshYearView();
}

void FastCalendarView::setResetEnabled(bool enabled)
{
    m_resetEnabled = enabled;
    refreshResetButtons();
}

QFrame *FastCalendarView::createPage(const QString &objectName, QPushButton **titleButton, QToolButton **resetButton,
                                     QToolButton **upButton, QToolButton **downButton, QVBoxLayout **bodyLayout)
{
    auto *page = new QFrame(this);
    page->setObjectName(objectName);
    page->setFixedSize(314, 355);
    FluentStyleSheet::setRole(page, QStringLiteral("FastCalendarPage"));

    auto *layout = new QVBoxLayout(page);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto *headerLayout = new QHBoxLayout;
    headerLayout->setContentsMargins(9, 8, 9, 8);
    headerLayout->setSpacing(7);

    *titleButton = new QPushButton(page);
    (*titleButton)->setObjectName(QStringLiteral("titleButton"));
    (*titleButton)->setCursor(Qt::PointingHandCursor);
    (*titleButton)->setFlat(true);
    (*titleButton)->setFixedHeight(34);
    (*titleButton)->setProperty("fastCalendarTitle", true);

    const auto createNavButton = [page](const QIcon &iconValue, const QString &name) {
        auto *button = new QToolButton(page);
        button->setObjectName(name);
        button->setCursor(Qt::PointingHandCursor);
        button->setIcon(iconValue);
        button->setIconSize(QSize(10, 10));
        button->setAutoRaise(true);
        button->setFixedSize(32, 34);
        button->setProperty("fastCalendarNavigation", true);
        return button;
    };

    *resetButton = createNavButton(icon(FluentIcon::Close), QStringLiteral("resetButton"));
    *upButton = createNavButton(icon(FluentIcon::Back), QStringLiteral("upButton"));
    *downButton = createNavButton(icon(FluentIcon::Forward), QStringLiteral("downButton"));

    headerLayout->addWidget(*titleButton, 1, Qt::AlignVCenter);
    headerLayout->addWidget(*resetButton, 0, Qt::AlignVCenter);
    headerLayout->addWidget(*upButton, 0, Qt::AlignVCenter);
    headerLayout->addWidget(*downButton, 0, Qt::AlignVCenter);
    layout->addLayout(headerLayout);

    *bodyLayout = new QVBoxLayout;
    (*bodyLayout)->setContentsMargins(0, 0, 0, 0);
    (*bodyLayout)->setSpacing(0);
    layout->addLayout(*bodyLayout);
    return page;
}

QPushButton *FastCalendarView::createGridButton(QWidget *parent, const QString &objectName)
{
    auto *button = new QPushButton(parent);
    button->setObjectName(objectName);
    button->setCursor(Qt::PointingHandCursor);
    button->setFlat(true);
    button->setProperty("fastCalendarItem", true);
    return button;
}

void FastCalendarView::createDayPage()
{
    QVBoxLayout *body = nullptr;
    m_dayView = createPage(QStringLiteral("dayView"), &m_dayTitleButton, &m_dayResetButton, &m_dayUpButton,
                           &m_dayDownButton, &body);

    auto *weekDayGroup = new QWidget(m_dayView);
    weekDayGroup->setObjectName(QStringLiteral("weekDayGroup"));
    auto *weekDayLayout = new QHBoxLayout(weekDayGroup);
    weekDayLayout->setContentsMargins(3, 12, 3, 12);
    weekDayLayout->setSpacing(0);
    const QStringList weekDays = {tr("Mo"), tr("Tu"), tr("We"), tr("Th"), tr("Fr"), tr("Sa"), tr("Su")};
    for (const QString &day : weekDays) {
        auto *label = new QLabel(day, weekDayGroup);
        label->setObjectName(QStringLiteral("weekDayLabel"));
        label->setAlignment(Qt::AlignCenter);
        weekDayLayout->addWidget(label, 1, Qt::AlignHCenter);
    }
    body->addWidget(weekDayGroup);

    auto *gridWidget = new QWidget(m_dayView);
    gridWidget->setObjectName(QStringLiteral("dayGrid"));
    auto *grid = new QGridLayout(gridWidget);
    grid->setContentsMargins(1, 0, 1, 0);
    grid->setSpacing(0);
    for (int row = 0; row < 6; ++row) {
        for (int column = 0; column < 7; ++column) {
            auto *button = createGridButton(gridWidget, QStringLiteral("dayButton"));
            button->setFixedSize(44, 44);
            m_dayButtons.append(button);
            grid->addWidget(button, row, column);
            connect(button, &QPushButton::clicked, this, [this, button]() {
                selectDay(button->property("date").toDate());
            });
        }
    }
    body->addWidget(gridWidget);

    connect(m_dayTitleButton, &QPushButton::clicked, this, &FastCalendarView::switchToMonthView);
    connect(m_dayResetButton, &QToolButton::clicked, this, [this]() {
        m_date = QDate();
        emit resetted();
        close();
    });
    connect(m_dayUpButton, &QToolButton::clicked, this, [this]() { scrollDayPage(-1); });
    connect(m_dayDownButton, &QToolButton::clicked, this, [this]() { scrollDayPage(1); });
}

void FastCalendarView::createMonthPage()
{
    QVBoxLayout *body = nullptr;
    m_monthView = createPage(QStringLiteral("monthView"), &m_monthTitleButton, &m_monthResetButton, &m_monthUpButton,
                             &m_monthDownButton, &body);

    auto *gridWidget = new QWidget(m_monthView);
    gridWidget->setObjectName(QStringLiteral("monthGrid"));
    auto *grid = new QGridLayout(gridWidget);
    grid->setContentsMargins(5, 0, 5, 0);
    grid->setSpacing(0);
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            auto *button = createGridButton(gridWidget, QStringLiteral("monthButton"));
            button->setFixedSize(76, 76);
            m_monthButtons.append(button);
            grid->addWidget(button, row, column);
            connect(button, &QPushButton::clicked, this, [this, button]() {
                selectMonth(button->property("date").toDate());
            });
        }
    }
    body->addWidget(gridWidget);

    connect(m_monthTitleButton, &QPushButton::clicked, this, &FastCalendarView::switchToYearView);
    connect(m_monthResetButton, &QToolButton::clicked, this, [this]() {
        m_date = QDate();
        emit resetted();
        close();
    });
    connect(m_monthUpButton, &QToolButton::clicked, this, [this]() { scrollMonthPage(-1); });
    connect(m_monthDownButton, &QToolButton::clicked, this, [this]() { scrollMonthPage(1); });
}

void FastCalendarView::createYearPage()
{
    QVBoxLayout *body = nullptr;
    m_yearView = createPage(QStringLiteral("yearView"), &m_yearTitleButton, &m_yearResetButton, &m_yearUpButton,
                            &m_yearDownButton, &body);
    m_yearTitleButton->setEnabled(false);

    auto *gridWidget = new QWidget(m_yearView);
    gridWidget->setObjectName(QStringLiteral("yearGrid"));
    auto *grid = new QGridLayout(gridWidget);
    grid->setContentsMargins(5, 0, 5, 0);
    grid->setSpacing(0);
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            auto *button = createGridButton(gridWidget, QStringLiteral("yearButton"));
            button->setFixedSize(76, 76);
            m_yearButtons.append(button);
            grid->addWidget(button, row, column);
            connect(button, &QPushButton::clicked, this, [this, button]() {
                selectYear(button->property("date").toDate());
            });
        }
    }
    body->addWidget(gridWidget);

    connect(m_yearResetButton, &QToolButton::clicked, this, [this]() {
        m_date = QDate();
        emit resetted();
        close();
    });
    connect(m_yearUpButton, &QToolButton::clicked, this, [this]() { scrollYearPage(-1); });
    connect(m_yearDownButton, &QToolButton::clicked, this, [this]() { scrollYearPage(1); });
}

void FastCalendarView::switchToDayView()
{
    if (m_stackedWidget && m_dayView) {
        m_stackedWidget->setCurrentWidget(m_dayView);
    }
}

void FastCalendarView::switchToMonthView()
{
    m_monthPageDate = QDate(pageDate().year(), 1, 1);
    refreshMonthView();
    if (m_stackedWidget && m_monthView) {
        m_stackedWidget->setCurrentWidget(m_monthView);
    }
}

void FastCalendarView::switchToYearView()
{
    m_yearPageStart = m_monthPageDate.year() - m_monthPageDate.year() % 10;
    refreshYearView();
    if (m_stackedWidget && m_yearView) {
        m_stackedWidget->setCurrentWidget(m_yearView);
    }
}

void FastCalendarView::refreshDayView()
{
    if (!m_dayPageDate.isValid()) {
        const QDate today = QDate::currentDate();
        m_dayPageDate = QDate(today.year(), today.month(), 1);
    }

    QString monthName = locale().standaloneMonthName(m_dayPageDate.month(), QLocale::LongFormat);
    if (monthName.isEmpty()) {
        monthName = locale().monthName(m_dayPageDate.month(), QLocale::LongFormat);
    }
    m_dayTitleButton->setText(QStringLiteral("%1 %2").arg(monthName).arg(m_dayPageDate.year()));

    const QDate today = QDate::currentDate();
    const QDate start = m_dayPageDate.addDays(-(m_dayPageDate.dayOfWeek() - 1));
    for (int i = 0; i < m_dayButtons.size(); ++i) {
        const QDate date = start.addDays(i);
        auto *button = m_dayButtons.at(i);
        button->setText(QString::number(date.day()));
        button->setProperty("date", date);
        button->setProperty("outOfRange", date.month() != m_dayPageDate.month());
        button->setProperty("today", date == today);
        button->setProperty("selected", m_date.isValid() && date == m_date);
        FluentStyleSheet::polish(button);
    }
}

void FastCalendarView::refreshMonthView()
{
    if (!m_monthPageDate.isValid()) {
        const QDate page = pageDate();
        m_monthPageDate = QDate(page.year(), 1, 1);
    }

    const int baseYear = m_monthPageDate.year();
    m_monthTitleButton->setText(QString::number(baseYear));
    const QDate today = QDate::currentDate();
    for (int i = 0; i < m_monthButtons.size(); ++i) {
        const int month = i % 12 + 1;
        const int year = baseYear + i / 12;
        const QDate date(year, month, 1);
        QString text = locale().standaloneMonthName(month, QLocale::ShortFormat);
        if (text.isEmpty()) {
            text = locale().monthName(month, QLocale::ShortFormat);
        }
        auto *button = m_monthButtons.at(i);
        button->setText(text);
        button->setProperty("date", date);
        button->setProperty("outOfRange", year != baseYear);
        button->setProperty("today", year == today.year() && month == today.month());
        button->setProperty("selected", m_date.isValid() && year == m_date.year() && month == m_date.month());
        FluentStyleSheet::polish(button);
    }
}

void FastCalendarView::refreshYearView()
{
    if (m_yearPageStart <= 0) {
        const QDate page = pageDate();
        m_yearPageStart = page.year() - page.year() % 10;
    }

    m_yearTitleButton->setText(QStringLiteral("%1 - %2").arg(m_yearPageStart).arg(m_yearPageStart + 9));
    const int visibleStart = m_yearPageStart - m_yearPageStart % 4;
    const int currentYear = QDate::currentDate().year();
    for (int i = 0; i < m_yearButtons.size(); ++i) {
        const int year = visibleStart + i;
        const QDate date(year, 1, 1);
        auto *button = m_yearButtons.at(i);
        button->setText(QString::number(year));
        button->setProperty("date", date);
        button->setProperty("outOfRange", year < m_yearPageStart || year > m_yearPageStart + 9);
        button->setProperty("today", year == currentYear);
        button->setProperty("selected", m_date.isValid() && year == m_date.year());
        FluentStyleSheet::polish(button);
    }
}

void FastCalendarView::refreshResetButtons()
{
    const QList<QToolButton *> buttons = {m_dayResetButton, m_monthResetButton, m_yearResetButton};
    for (QToolButton *button : buttons) {
        if (button) {
            button->setVisible(m_resetEnabled);
        }
    }
}

void FastCalendarView::selectDay(const QDate &date)
{
    if (!date.isValid()) {
        return;
    }

    const bool changed = m_date != date;
    close();
    if (changed) {
        m_date = date;
        emit dateChanged(date);
    }
}

void FastCalendarView::selectMonth(const QDate &date)
{
    if (!date.isValid()) {
        return;
    }
    m_dayPageDate = QDate(date.year(), date.month(), 1);
    refreshDayView();
    switchToDayView();
}

void FastCalendarView::selectYear(const QDate &date)
{
    if (!date.isValid()) {
        return;
    }
    m_monthPageDate = QDate(date.year(), 1, 1);
    refreshMonthView();
    if (m_stackedWidget && m_monthView) {
        m_stackedWidget->setCurrentWidget(m_monthView);
    }
}

void FastCalendarView::scrollDayPage(int monthOffset)
{
    m_dayPageDate = pageDate().addMonths(monthOffset);
    m_dayPageDate = QDate(m_dayPageDate.year(), m_dayPageDate.month(), 1);
    refreshDayView();
}

void FastCalendarView::scrollMonthPage(int yearOffset)
{
    if (!m_monthPageDate.isValid()) {
        m_monthPageDate = QDate(pageDate().year(), 1, 1);
    }
    m_monthPageDate = m_monthPageDate.addYears(yearOffset);
    refreshMonthView();
}

void FastCalendarView::scrollYearPage(int decadeOffset)
{
    if (m_yearPageStart <= 0) {
        m_yearPageStart = pageDate().year() - pageDate().year() % 10;
    }
    m_yearPageStart += decadeOffset * 10;
    refreshYearView();
}

QDate FastCalendarView::pageDate() const
{
    if (m_dayPageDate.isValid()) {
        return m_dayPageDate;
    }
    if (m_date.isValid()) {
        return m_date;
    }
    const QDate today = QDate::currentDate();
    return QDate(today.year(), today.month(), 1);
}

void FastCalendarView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    painter.setBrush(dark ? QColor(40, 40, 40) : QColor(248, 248, 248));
    painter.setPen(dark ? QColor(23, 23, 23) : QColor(234, 234, 234));
    painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
}

void FastCalendarView::closeEvent(QCloseEvent *event)
{
    emit closed();
    FlyoutViewBase::closeEvent(event);
}

CalendarPicker::CalendarPicker(QWidget *parent) : QPushButton(parent)
{
    m_dateFormat = QLocale().dateFormat(QLocale::ShortFormat);

    setMinimumHeight(32);
    setCursor(Qt::PointingHandCursor);
    setText(tr("Pick a date"));
    setProperty("hasDate", false);
    FluentStyleSheet::setRole(this, QStringLiteral("CalendarPicker"));

    connect(this, &QPushButton::clicked, this, [this]() { showCalendarPopup(); });
}

QDate CalendarPicker::date() const { return m_date; }

void CalendarPicker::setDate(const QDate &date)
{
    if (!date.isValid()) {
        reset();
        return;
    }

    if (m_date == date) {
        return;
    }

    m_date = date;
    updateDateDisplay();
    emit dateChanged(m_date);
}

void CalendarPicker::reset()
{
    m_date = QDate();
    setText(tr("Pick a date"));
    setProperty("hasDate", false);
    FluentStyleSheet::polish(this);
    update();
}

bool CalendarPicker::isResetEnabled() const { return m_resetEnabled; }

void CalendarPicker::setResetEnabled(bool enabled) { m_resetEnabled = enabled; }

QString CalendarPicker::dateFormat() const { return m_dateFormat; }

void CalendarPicker::setDateFormat(const QString &format)
{
    m_dateFormat = format;
    if (m_date.isValid()) {
        updateDateDisplay();
    }
}

void CalendarPicker::paintEvent(QPaintEvent *event)
{
    QPushButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (!property("hasDate").toBool()) {
        painter.setOpacity(0.6);
    }

    const int iconSize = 12;
    const QRect iconRect(width() - 23, (height() - iconSize) / 2, iconSize, iconSize);
    FluentQt::icon(FluentIcon::Calendar).paint(&painter, iconRect);
}

void CalendarPicker::showCalendarPopup()
{
    auto *popup = m_popup;
    if (!popup) {
        popup = new CalendarView(window());
        m_popup = popup;
        connect(popup, &CalendarView::dateChanged, this, &CalendarPicker::onPopupDateSelected);
        connect(popup, &CalendarView::resetted, this, &CalendarPicker::onPopupReset);
    }

    popup->setResetEnabled(m_resetEnabled);
    popup->setDate(m_date);

    popup->adjustSize();
    const int x = width() / 2 - popup->width() / 2;
    popup->exec(mapToGlobal(QPoint(x, height())));
}

void CalendarPicker::onPopupDateSelected(const QDate &date) { setDate(date); }

void CalendarPicker::onPopupReset() { reset(); }

void CalendarPicker::updateDateDisplay()
{
    setText(m_date.toString(m_dateFormat));
    setProperty("hasDate", true);
    FluentStyleSheet::polish(this);
    update();
}

FastCalendarPicker::FastCalendarPicker(QWidget *parent) : CalendarPicker(parent)
{
}

FlyoutAnimationType FastCalendarPicker::flyoutAnimationType() const { return m_flyoutAnimationType; }

void FastCalendarPicker::setFlyoutAnimationType(FlyoutAnimationType type) { m_flyoutAnimationType = type; }

void FastCalendarPicker::showCalendarPopup()
{
    auto *view = new FastCalendarView(window());
    view->setResetEnabled(isResetEnabled());
    if (date().isValid()) {
        view->setDate(date());
    }

    connect(view, &FastCalendarView::resetted, this, [this]() { onPopupReset(); });
    connect(view, &FastCalendarView::dateChanged, this,
            [this](const QDate &selectedDate) { onPopupDateSelected(selectedDate); });

    Flyout::make(view, this, window(), m_flyoutAnimationType);
}

} // namespace FluentQt
