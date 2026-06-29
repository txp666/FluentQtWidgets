#pragma once

#include <QtCore/QObject>

class QWidget;

namespace FluentQt {

void checkGalleryUpdate(QWidget *parent, bool notifyUpToDate = false, bool startupMode = false);

} // namespace FluentQt
