#include "SignalBus.h"

SignalBus *SignalBus::s_instance = nullptr;

SignalBus::SignalBus(QObject *parent) : QObject(parent)
{
}

SignalBus *SignalBus::instance()
{
    if (!s_instance) {
        s_instance = new SignalBus;
    }
    return s_instance;
}
