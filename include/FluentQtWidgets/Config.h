#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtGui/QColor>

namespace FluentQt {

class FQW_API FluentConfig : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString fileName READ fileName WRITE setFileName NOTIFY fileNameChanged)
    Q_PROPERTY(FluentQt::Theme themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(QColor themeColor READ themeColor WRITE setThemeColor NOTIFY themeColorChanged)
    Q_PROPERTY(QStringList fontFamilies READ fontFamilies WRITE setFontFamilies NOTIFY fontFamiliesChanged)
    Q_PROPERTY(QString localeName READ localeName WRITE setLocaleName NOTIFY localeNameChanged)
    Q_PROPERTY(QString dpiScale READ dpiScale WRITE setDpiScale NOTIFY dpiScaleChanged)
    Q_PROPERTY(QString downloadFolder READ downloadFolder WRITE setDownloadFolder NOTIFY downloadFolderChanged)
    Q_PROPERTY(bool micaEnabled READ isMicaEnabled WRITE setMicaEnabled NOTIFY micaEnabledChanged)
    Q_PROPERTY(int acrylicBlurRadius READ acrylicBlurRadius WRITE setAcrylicBlurRadius NOTIFY acrylicBlurRadiusChanged)
    Q_PROPERTY(bool autoUpdateEnabled READ isAutoUpdateEnabled WRITE setAutoUpdateEnabled NOTIFY autoUpdateEnabledChanged)

  public:
    static FluentConfig *instance();

    QString fileName() const;
    Theme themeMode() const;
    QColor themeColor() const;
    QStringList fontFamilies() const;
    QString localeName() const;
    QString dpiScale() const;
    QString downloadFolder() const;
    bool isMicaEnabled() const;
    int acrylicBlurRadius() const;
    bool isAutoUpdateEnabled() const;

    bool load(const QString &fileName = QString());
    bool save(const QString &fileName = QString());

  public slots:
    void setFileName(const QString &fileName);
    void setThemeMode(FluentQt::Theme theme);
    void setThemeColor(const QColor &color);
    void setFontFamilies(const QStringList &families);
    void setLocaleName(const QString &localeName);
    void setDpiScale(const QString &dpiScale);
    void setDownloadFolder(const QString &downloadFolder);
    void setMicaEnabled(bool enabled);
    void setAcrylicBlurRadius(int radius);
    void setAutoUpdateEnabled(bool enabled);
    void reset();

  signals:
    void fileNameChanged(const QString &fileName);
    void themeModeChanged(FluentQt::Theme theme);
    void themeColorChanged(const QColor &color);
    void fontFamiliesChanged(const QStringList &families);
    void localeNameChanged(const QString &localeName);
    void dpiScaleChanged(const QString &dpiScale);
    void downloadFolderChanged(const QString &downloadFolder);
    void micaEnabledChanged(bool enabled);
    void acrylicBlurRadiusChanged(int radius);
    void autoUpdateEnabledChanged(bool enabled);
    void configLoaded();
    void configSaved();

  private:
    explicit FluentConfig(QObject *parent = nullptr);

    QString m_fileName;
    Theme m_themeMode = Theme::Light;
    QColor m_themeColor = QColor(QStringLiteral("#009faa"));
    QStringList m_fontFamilies = {QStringLiteral("Segoe UI"), QStringLiteral("Microsoft YaHei UI"),
                                  QStringLiteral("PingFang SC")};
    QString m_localeName = QStringLiteral("Auto");
    QString m_dpiScale = QStringLiteral("Auto");
    QString m_downloadFolder = QStringLiteral("app/download");
    bool m_micaEnabled = false;
    int m_acrylicBlurRadius = 15;
    bool m_autoUpdateEnabled = false;
};

} // namespace FluentQt
