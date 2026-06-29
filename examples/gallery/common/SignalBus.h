#pragma once

#include <QtCore/QObject>

class SignalBus : public QObject
{
    Q_OBJECT

public:
    static SignalBus *instance();

signals:
    void switchToSampleCard(const QString &routeKey, int index);
    void micaEnableChanged(bool enabled);
    void supportSignal();

private:
    explicit SignalBus(QObject *parent = nullptr);
    static SignalBus *s_instance;
};
