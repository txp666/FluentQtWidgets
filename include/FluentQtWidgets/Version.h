#pragma once

#include <QtCore/QString>

namespace FluentQt {

#ifndef FQW_VERSION
#define FQW_VERSION "0.2.0"
#endif

inline QString libraryVersion()
{
    return QStringLiteral(FQW_VERSION);
}

} // namespace FluentQt
