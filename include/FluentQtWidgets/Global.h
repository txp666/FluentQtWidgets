#pragma once

#include <QtCore/qglobal.h>

#if defined(FQW_STATIC)
#define FQW_API
#elif defined(FQW_BUILD_LIBRARY)
#define FQW_API Q_DECL_EXPORT
#else
#define FQW_API Q_DECL_IMPORT
#endif

#define FQW_DISABLE_COPY(Class)                                                                                        \
    Class(const Class &) = delete;                                                                                     \
    Class &operator=(const Class &) = delete
