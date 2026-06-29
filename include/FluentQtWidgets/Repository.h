#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QString>
#include <QtCore/QStringList>

#ifndef FQW_REPOSITORY_URL
#define FQW_REPOSITORY_URL "https://github.com/txp666/FluentQtWidgets"
#endif

namespace FluentQt {

inline QString repositoryUrl()
{
    return QStringLiteral(FQW_REPOSITORY_URL);
}

inline QString repositoryIssuesUrl()
{
    return QStringLiteral(FQW_REPOSITORY_URL "/issues");
}

inline QString repositoryReleasesUrl()
{
    return QStringLiteral(FQW_REPOSITORY_URL "/releases");
}

inline QString repositoryLatestReleaseApiUrl()
{
    const QString repoUrl = QStringLiteral(FQW_REPOSITORY_URL);
    const QString githubPrefix = QStringLiteral("https://github.com/");
    if (!repoUrl.startsWith(githubPrefix)) {
        return repoUrl + QStringLiteral("/releases/latest");
    }

    QString path = repoUrl.mid(githubPrefix.size());
    if (path.endsWith('/')) {
        path.chop(1);
    }
    path = path.split('?').first();

    const auto parts = path.split('/');
    if (parts.size() < 2) {
        return repoUrl + QStringLiteral("/releases/latest");
    }

    return QStringLiteral("https://api.github.com/repos/") + parts.at(0) + '/' + parts.at(1)
        + QStringLiteral("/releases/latest");
}

inline QString repositoryDocsUrl()
{
    return QStringLiteral(FQW_REPOSITORY_URL "/tree/main/docs");
}

inline QString repositoryExampleUrl()
{
    return QStringLiteral(FQW_REPOSITORY_URL "/tree/main/examples");
}

} // namespace FluentQt
