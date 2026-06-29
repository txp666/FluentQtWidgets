#include "GalleryUpdateChecker.h"

#include <FluentQtWidgets/Repository.h>
#include <FluentQtWidgets/Version.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>

#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

namespace FluentQt {

namespace {

QString normalizeVersion(QString version)
{
    version = version.trimmed();
    if (version.startsWith('v') || version.startsWith('V')) {
        version.remove(0, 1);
    }
    version = version.trimmed();
    return version;
}

int compareVersions(const QString &lhs, const QString &rhs)
{
    const auto lhsParts = lhs.split('.', Qt::SkipEmptyParts);
    const auto rhsParts = rhs.split('.', Qt::SkipEmptyParts);

    const int maxCount = qMax(lhsParts.size(), rhsParts.size());
    for (int i = 0; i < maxCount; ++i) {
        const int left = (i < lhsParts.size()) ? lhsParts.at(i).toInt() : 0;
        const int right = (i < rhsParts.size()) ? rhsParts.at(i).toInt() : 0;

        if (left > right) {
            return 1;
        }
        if (left < right) {
            return -1;
        }
    }
    return 0;
}

void appendReleaseActionButton(const QString &version, const QUrl &url, QWidget *parent)
{
    auto *bar = InfoBar::newInfoBar(
        InfoBarSeverity::Info,
        QStringLiteral("Update available"),
        QStringLiteral("A new version is available: %1. Open the release page to download update.").arg(version),
        Qt::Horizontal,
        true,
        12000,
        InfoBarPosition::Top,
        parent);
    bar->addWidget(new HyperlinkButton(url.toString(), QStringLiteral("Open release"), bar));
}

void appendNoUpdateInfoBar(const QString &current, QWidget *parent)
{
    InfoBar::success(
        QStringLiteral("Already latest"),
        QStringLiteral("Current version is %1").arg(current),
        Qt::Horizontal,
        true,
        2500,
        InfoBarPosition::Top,
        parent);
}

void appendUpdateErrorBar(const QString &message, QWidget *parent)
{
    InfoBar::error(
        QStringLiteral("Update check failed"),
        message,
        Qt::Horizontal,
        true,
        4000,
        InfoBarPosition::Top,
        parent);
}

} // namespace

void checkGalleryUpdate(QWidget *parent, bool notifyUpToDate, bool startupMode)
{
    const auto currentVersion = libraryVersion();

    auto *manager = new QNetworkAccessManager(parent);
    const QUrl releaseApi(repositoryLatestReleaseApiUrl());
    QNetworkRequest request(releaseApi);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("FluentQtWidgets/" ) + currentVersion);

    auto *reply = manager->get(request);
    QPointer<QWidget> host = parent;
    QObject::connect(reply, &QNetworkReply::finished, [manager, reply, host, currentVersion, notifyUpToDate, startupMode]() {
        reply->deleteLater();
        manager->deleteLater();

        if (!host) {
            return;
        }

        const auto error = reply->error();
        if (error != QNetworkReply::NoError) {
            if (!startupMode) {
                appendUpdateErrorBar(reply->errorString(), host);
            }
            return;
        }

        const auto jsonDoc = QJsonDocument::fromJson(reply->readAll());
        if (!jsonDoc.isObject()) {
            if (!startupMode) {
                appendUpdateErrorBar(QStringLiteral("Invalid update response format."), host);
            }
            return;
        }

        const QJsonObject root = jsonDoc.object();
        const QString latestTag = root.value(QStringLiteral("tag_name")).toString();
        const QString latestVersion = normalizeVersion(latestTag);
        const QUrl releaseUrl = QUrl(root.value(QStringLiteral("html_url")).toString());

        if (latestVersion.isEmpty() || !releaseUrl.isValid()) {
            if (!startupMode) {
                appendUpdateErrorBar(QStringLiteral("Unable to parse latest release information."), host);
            }
            return;
        }

        if (compareVersions(currentVersion, latestVersion) < 0) {
            appendReleaseActionButton(latestVersion, releaseUrl, host);
            return;
        }

        if (notifyUpToDate) {
            appendNoUpdateInfoBar(currentVersion, host);
        }
    });
}

} // namespace FluentQt
