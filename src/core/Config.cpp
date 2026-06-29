#include <FluentQtWidgets/Config.h>

#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Window/FluentWindow.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QList>
#include <QtCore/QPair>
#include <QtCore/QStandardPaths>
#include <QtCore/QtGlobal>

namespace FluentQt {

static QString defaultConfigPath()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    return QDir(base.isEmpty() ? QDir::currentPath() : base).filePath(QStringLiteral("fluentqtwidgets.json"));
}

static QString themeToString(Theme theme)
{
    switch (theme) {
    case Theme::Light:
        return QStringLiteral("Light");
    case Theme::Dark:
        return QStringLiteral("Dark");
    case Theme::Auto:
        return QStringLiteral("Auto");
    }
    return QStringLiteral("Light");
}

static Theme themeFromString(const QString &value)
{
    if (value.compare(QStringLiteral("Dark"), Qt::CaseInsensitive) == 0) {
        return Theme::Dark;
    }
    if (value.compare(QStringLiteral("Auto"), Qt::CaseInsensitive) == 0) {
        return Theme::Auto;
    }
    return Theme::Light;
}

static QString normalizedDpiScale(const QString &value)
{
    const QString cleaned = value.trimmed();
    if (cleaned.isEmpty() || cleaned.compare(QStringLiteral("Auto"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("Auto");
    }

    const QString percent = cleaned.endsWith(QLatin1Char('%')) ? cleaned.left(cleaned.size() - 1).trimmed() : QString();
    bool ok = false;
    const double scale = percent.isEmpty() ? cleaned.toDouble(&ok) : percent.toDouble(&ok) / 100.0;
    if (!ok) {
        return QStringLiteral("Auto");
    }

    static const QList<QPair<double, QString>> values = {
        {1.0, QStringLiteral("1")}, {1.25, QStringLiteral("1.25")}, {1.5, QStringLiteral("1.5")},
        {1.75, QStringLiteral("1.75")}, {2.0, QStringLiteral("2")},
    };
    for (const auto &candidate : values) {
        if (qAbs(scale - candidate.first) < 0.001) {
            return candidate.second;
        }
    }

    return QStringLiteral("Auto");
}

static int normalizedAcrylicBlurRadius(int radius)
{
    return qBound(0, radius, 40);
}

FluentConfig *FluentConfig::instance()
{
    static FluentConfig config;
    return &config;
}

FluentConfig::FluentConfig(QObject *parent)
    : QObject(parent), m_fileName(defaultConfigPath()), m_micaEnabled(isMicaEffectAvailable())
{
}

QString FluentConfig::fileName() const { return m_fileName; }

Theme FluentConfig::themeMode() const { return m_themeMode; }

QColor FluentConfig::themeColor() const { return m_themeColor; }

QStringList FluentConfig::fontFamilies() const { return m_fontFamilies; }

QString FluentConfig::localeName() const { return m_localeName; }

QString FluentConfig::dpiScale() const { return m_dpiScale; }

QString FluentConfig::downloadFolder() const { return m_downloadFolder; }

bool FluentConfig::isMicaEnabled() const { return m_micaEnabled; }

int FluentConfig::acrylicBlurRadius() const { return m_acrylicBlurRadius; }

bool FluentConfig::isAutoUpdateEnabled() const { return m_autoUpdateEnabled; }

bool FluentConfig::load(const QString &fileName)
{
    if (!fileName.isEmpty()) {
        setFileName(fileName);
    }

    QFile file(m_fileName);
    if (!file.exists()) {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        return false;
    }

    const QJsonObject root = document.object();
    const QJsonObject group = root.value(QStringLiteral("FluentQtWidgets")).toObject();
    const QJsonObject qfluentGroup = root.value(QStringLiteral("QFluentWidgets")).toObject();
    const QJsonObject mainWindowGroup = root.value(QStringLiteral("MainWindow")).toObject();
    const QJsonObject foldersGroup = root.value(QStringLiteral("Folders")).toObject();
    const QJsonObject materialGroup = root.value(QStringLiteral("Material")).toObject();

    const QJsonValue themeModeValue = group.contains(QStringLiteral("ThemeMode"))
                                         ? group.value(QStringLiteral("ThemeMode"))
                                         : qfluentGroup.value(QStringLiteral("ThemeMode"));
    setThemeMode(themeFromString(themeModeValue.toString(themeToString(m_themeMode))));

    const QJsonValue themeColorValue = group.contains(QStringLiteral("ThemeColor"))
                                          ? group.value(QStringLiteral("ThemeColor"))
                                          : qfluentGroup.value(QStringLiteral("ThemeColor"));
    setThemeColor(QColor(themeColorValue.toString(m_themeColor.name(QColor::HexRgb))));
    setLocaleName(group.value(QStringLiteral("LocaleName")).toString(m_localeName));
    const QJsonValue dpiScaleValue = group.contains(QStringLiteral("DpiScale"))
                                        ? group.value(QStringLiteral("DpiScale"))
                                        : mainWindowGroup.value(QStringLiteral("DpiScale"));
    if (!dpiScaleValue.isUndefined()) {
        setDpiScale(dpiScaleValue.toVariant().toString());
    }
    const QJsonValue downloadFolderValue = group.contains(QStringLiteral("DownloadFolder"))
                                             ? group.value(QStringLiteral("DownloadFolder"))
                                             : foldersGroup.value(QStringLiteral("Download"));
    if (!downloadFolderValue.isUndefined()) {
        setDownloadFolder(downloadFolderValue.toString());
    }
    const QJsonValue micaEnabledValue = group.contains(QStringLiteral("MicaEnabled"))
                                           ? group.value(QStringLiteral("MicaEnabled"))
                                           : mainWindowGroup.value(QStringLiteral("MicaEnabled"));
    if (!micaEnabledValue.isUndefined()) {
        setMicaEnabled(micaEnabledValue.toBool());
    }
    const QJsonValue acrylicBlurRadiusValue = group.contains(QStringLiteral("AcrylicBlurRadius"))
                                                ? group.value(QStringLiteral("AcrylicBlurRadius"))
                                                : materialGroup.value(QStringLiteral("AcrylicBlurRadius"));
    if (!acrylicBlurRadiusValue.isUndefined()) {
        setAcrylicBlurRadius(acrylicBlurRadiusValue.toInt(m_acrylicBlurRadius));
    }

    const QJsonValue autoUpdateEnabledValue = group.contains(QStringLiteral("AutoUpdateEnabled"))
                                                 ? group.value(QStringLiteral("AutoUpdateEnabled"))
                                                 : QJsonValue();
    if (!autoUpdateEnabledValue.isUndefined()) {
        setAutoUpdateEnabled(autoUpdateEnabledValue.toBool());
    }

    const QJsonArray fonts = group.contains(QStringLiteral("FontFamilies"))
                                 ? group.value(QStringLiteral("FontFamilies")).toArray()
                                 : qfluentGroup.value(QStringLiteral("FontFamilies")).toArray();
    if (!fonts.isEmpty()) {
        QStringList families;
        for (const QJsonValue &font : fonts) {
            const QString family = font.toString();
            if (!family.isEmpty()) {
                families.append(family);
            }
        }
        setFontFamilies(families);
    }

    emit configLoaded();
    return true;
}

bool FluentConfig::save(const QString &fileName)
{
    const QString target = fileName.isEmpty() ? m_fileName : fileName;
    QDir().mkpath(QFileInfo(target).absolutePath());

    QJsonArray fontArray;
    for (const QString &family : m_fontFamilies) {
        fontArray.append(family);
    }

    QJsonObject group;
    group.insert(QStringLiteral("ThemeMode"), themeToString(m_themeMode));
    group.insert(QStringLiteral("ThemeColor"), m_themeColor.name(QColor::HexRgb));
    group.insert(QStringLiteral("FontFamilies"), fontArray);
    group.insert(QStringLiteral("LocaleName"), m_localeName);
    group.insert(QStringLiteral("DpiScale"), m_dpiScale);
    group.insert(QStringLiteral("MicaEnabled"), m_micaEnabled);
    group.insert(QStringLiteral("AcrylicBlurRadius"), m_acrylicBlurRadius);
    group.insert(QStringLiteral("AutoUpdateEnabled"), m_autoUpdateEnabled);

    QJsonObject qfluentGroup;
    qfluentGroup.insert(QStringLiteral("ThemeMode"), themeToString(m_themeMode));
    qfluentGroup.insert(QStringLiteral("ThemeColor"), m_themeColor.name(QColor::HexArgb));
    qfluentGroup.insert(QStringLiteral("FontFamilies"), fontArray);

    QJsonObject mainWindowGroup;
    mainWindowGroup.insert(QStringLiteral("MicaEnabled"), m_micaEnabled);
    mainWindowGroup.insert(QStringLiteral("DpiScale"), m_dpiScale);

    QJsonObject foldersGroup;
    foldersGroup.insert(QStringLiteral("Download"), m_downloadFolder);

    QJsonObject materialGroup;
    materialGroup.insert(QStringLiteral("AcrylicBlurRadius"), m_acrylicBlurRadius);

    QJsonObject root;
    root.insert(QStringLiteral("FluentQtWidgets"), group);
    root.insert(QStringLiteral("QFluentWidgets"), qfluentGroup);
    root.insert(QStringLiteral("MainWindow"), mainWindowGroup);
    root.insert(QStringLiteral("Folders"), foldersGroup);
    root.insert(QStringLiteral("Material"), materialGroup);

    QFile file(target);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    emit configSaved();
    return true;
}

void FluentConfig::setFileName(const QString &fileName)
{
    if (fileName.isEmpty() || m_fileName == fileName) {
        return;
    }

    m_fileName = fileName;
    emit fileNameChanged(m_fileName);
}

void FluentConfig::setThemeMode(Theme theme)
{
    if (m_themeMode == theme) {
        return;
    }

    m_themeMode = theme;
    ThemeManager::instance()->setTheme(theme);
    emit themeModeChanged(m_themeMode);
}

void FluentConfig::setThemeColor(const QColor &color)
{
    if (!color.isValid() || m_themeColor == color) {
        return;
    }

    m_themeColor = color;
    ThemeManager::instance()->setAccentColor(color);
    emit themeColorChanged(m_themeColor);
}

void FluentConfig::setFontFamilies(const QStringList &families)
{
    QStringList cleaned;
    for (const QString &family : families) {
        const QString trimmed = family.trimmed();
        if (!trimmed.isEmpty() && !cleaned.contains(trimmed)) {
            cleaned.append(trimmed);
        }
    }

    if (cleaned.isEmpty() || cleaned == m_fontFamilies) {
        return;
    }

    m_fontFamilies = cleaned;
    emit fontFamiliesChanged(m_fontFamilies);
}

void FluentConfig::setLocaleName(const QString &localeName)
{
    const QString cleaned = localeName.trimmed().isEmpty() ? QStringLiteral("Auto") : localeName.trimmed();
    if (m_localeName == cleaned) {
        return;
    }

    m_localeName = cleaned;
    emit localeNameChanged(m_localeName);
}

void FluentConfig::setDpiScale(const QString &dpiScale)
{
    const QString cleaned = normalizedDpiScale(dpiScale);
    if (m_dpiScale == cleaned) {
        return;
    }

    m_dpiScale = cleaned;
    emit dpiScaleChanged(m_dpiScale);
}

void FluentConfig::setDownloadFolder(const QString &downloadFolder)
{
    const QString cleaned =
        downloadFolder.trimmed().isEmpty() ? QStringLiteral("app/download") : QDir::cleanPath(downloadFolder.trimmed());
    if (m_downloadFolder == cleaned) {
        return;
    }

    m_downloadFolder = cleaned;
    emit downloadFolderChanged(m_downloadFolder);
}

void FluentConfig::setMicaEnabled(bool enabled)
{
    const bool actual = enabled && isMicaEffectAvailable();
    if (m_micaEnabled == actual) {
        return;
    }

    m_micaEnabled = actual;
    emit micaEnabledChanged(m_micaEnabled);
}

void FluentConfig::setAcrylicBlurRadius(int radius)
{
    const int cleaned = normalizedAcrylicBlurRadius(radius);
    if (m_acrylicBlurRadius == cleaned) {
        return;
    }

    m_acrylicBlurRadius = cleaned;
    emit acrylicBlurRadiusChanged(m_acrylicBlurRadius);
}

void FluentConfig::setAutoUpdateEnabled(bool enabled)
{
    if (m_autoUpdateEnabled == enabled) {
        return;
    }

    m_autoUpdateEnabled = enabled;
    emit autoUpdateEnabledChanged(m_autoUpdateEnabled);
}

void FluentConfig::reset()
{
    setThemeMode(Theme::Light);
    setThemeColor(QColor(QStringLiteral("#009faa")));
    setFontFamilies({QStringLiteral("Segoe UI"), QStringLiteral("Microsoft YaHei UI"), QStringLiteral("PingFang SC")});
    setLocaleName(QStringLiteral("Auto"));
    setDpiScale(QStringLiteral("Auto"));
    setDownloadFolder(QStringLiteral("app/download"));
    setMicaEnabled(isMicaEffectAvailable());
    setAcrylicBlurRadius(15);
    setAutoUpdateEnabled(false);
}

} // namespace FluentQt
