#pragma once

#include <QtCore/QString>

namespace FluentQt {

#ifndef FQW_VERSION
#define FQW_VERSION "0.3.2"
#endif

inline QString libraryVersion()
{
    return QStringLiteral(FQW_VERSION);
}

} // namespace FluentQt
