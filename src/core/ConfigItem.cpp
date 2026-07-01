#include <FluentQtWidgets/ConfigItem.h>

#include <FluentQtWidgets/Settings/SettingCard.h>

#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>
#include <QtCore/QSignalBlocker>

namespace FluentQt {

ConfigItem::ConfigItem(const QString &group, const QString &name, const QVariant &defaultValue, QObject *parent)
    : QObject(parent), m_group(group), m_name(name), m_defaultValue(defaultValue)
{
    m_value = correctValue(defaultValue);
    ConfigItemRegistry::instance()->registerItem(this);
}

QString ConfigItem::group() const { return m_group; }

QString ConfigItem::name() const { return m_name; }

QString ConfigItem::key() const { return m_name.isEmpty() ? m_group : m_group + QLatin1Char('.') + m_name; }

QVariant ConfigItem::value() const { return m_value; }

QVariant ConfigItem::defaultValue() const { return m_defaultValue; }

bool ConfigItem::restartRequired() const { return m_restartRequired; }

void ConfigItem::setRestartRequired(bool restart) { m_restartRequired = restart; }

void ConfigItem::setValue(const QVariant &value)
{
    const QVariant corrected = correctValue(value);
    if (m_value == corrected) {
        return;
    }

    m_value = corrected;
    emit valueChanged(m_value);
    emit ConfigItemRegistry::instance()->itemValueChanged(this);
}

void ConfigItem::reset() { setValue(m_defaultValue); }

QVariant ConfigItem::correctValue(const QVariant &value) const { return value; }

OptionsConfigItem::OptionsConfigItem(const QString &group, const QString &name, const QVariant &defaultValue,
                                     const QVariantList &options, QObject *parent)
    : ConfigItem(group, name, defaultValue, parent), m_options(options)
{
}

QVariantList OptionsConfigItem::options() const { return m_options; }

QVariant OptionsConfigItem::correctValue(const QVariant &value) const
{
    for (const QVariant &option : m_options) {
        if (option == value) {
            return option;
        }
    }
    return m_options.isEmpty() ? value : m_options.first();
}

RangeConfigItem::RangeConfigItem(const QString &group, const QString &name, int defaultValue, int minimum, int maximum,
                                 QObject *parent)
    : ConfigItem(group, name, defaultValue, parent), m_minimum(minimum), m_maximum(maximum)
{
    m_value = correctValue(defaultValue);
}

int RangeConfigItem::minimum() const { return m_minimum; }

int RangeConfigItem::maximum() const { return m_maximum; }

QVariant RangeConfigItem::correctValue(const QVariant &value) const
{
    bool ok = false;
    const int number = value.toInt(&ok);
    if (!ok) {
        return m_defaultValue;
    }
    return qBound(m_minimum, number, m_maximum);
}

ColorConfigItem::ColorConfigItem(const QString &group, const QString &name, const QColor &defaultColor,
                                 QObject *parent)
    : ConfigItem(group, name, defaultColor, parent)
{
}

QVariant ColorConfigItem::correctValue(const QVariant &value) const
{
    const QColor color = value.value<QColor>();
    return color.isValid() ? QVariant(color) : m_defaultValue;
}

ConfigItemRegistry *ConfigItemRegistry::instance()
{
    static ConfigItemRegistry registry;
    return &registry;
}

ConfigItemRegistry::ConfigItemRegistry(QObject *parent) : QObject(parent) {}

void ConfigItemRegistry::registerItem(ConfigItem *item)
{
    if (!item || item->key().isEmpty()) {
        return;
    }

    m_items.insert(item->key(), item);
    connect(item, &ConfigItem::valueChanged, this, [this, item]() { emit itemValueChanged(item); });
}

ConfigItem *ConfigItemRegistry::item(const QString &key) const { return m_items.value(key, nullptr); }

QList<ConfigItem *> ConfigItemRegistry::items() const { return m_items.values(); }

void ConfigItemRegistry::loadFromJson(const QJsonObject &object)
{
    for (ConfigItem *configItem : m_items) {
        const QJsonValue groupValue = object.value(configItem->group());
        if (configItem->name().isEmpty()) {
            if (groupValue.isUndefined()) {
                continue;
            }
            configItem->setValue(groupValue.toVariant());
            continue;
        }

        if (!groupValue.isObject()) {
            continue;
        }

        const QJsonObject groupObject = groupValue.toObject();
        const QJsonValue itemValue = groupObject.value(configItem->name());
        if (itemValue.isUndefined()) {
            continue;
        }

        if (auto *colorItem = qobject_cast<ColorConfigItem *>(configItem)) {
            colorItem->setValue(QColor(itemValue.toString()));
        } else {
            configItem->setValue(itemValue.toVariant());
        }
    }
}

QJsonObject ConfigItemRegistry::toJson() const
{
    QJsonObject root;
    for (ConfigItem *configItem : m_items) {
        if (configItem->name().isEmpty()) {
            root.insert(configItem->group(), QJsonValue::fromVariant(configItem->value()));
            continue;
        }

        QJsonObject groupObject = root.value(configItem->group()).toObject();
        if (auto *colorItem = qobject_cast<const ColorConfigItem *>(configItem)) {
            groupObject.insert(configItem->name(), colorItem->value().value<QColor>().name(QColor::HexArgb));
        } else {
            groupObject.insert(configItem->name(), QJsonValue::fromVariant(configItem->value()));
        }
        root.insert(configItem->group(), groupObject);
    }
    return root;
}

void bindSwitchSettingCard(SwitchSettingCard *card, ConfigItem *item)
{
    if (!card || !item) {
        return;
    }

    card->setChecked(item->value().toBool());
    QObject::connect(card, &SwitchSettingCard::checkedChanged, item, [item](bool checked) {
        item->setValue(checked);
    });
    QObject::connect(item, &ConfigItem::valueChanged, card, [card](const QVariant &value) {
        const QSignalBlocker blocker(card);
        card->setChecked(value.toBool());
    });
}

void bindComboBoxSettingCard(ComboBoxSettingCard *card, OptionsConfigItem *item,
                             const std::function<QVariant(int)> &indexToValue,
                             const std::function<int(const QVariant &)> &valueToIndex)
{
    if (!card || !item) {
        return;
    }

    card->setCurrentIndex(valueToIndex(item->value()));
    QObject::connect(card, &ComboBoxSettingCard::currentIndexChanged, item,
                     [item, indexToValue](int index) { item->setValue(indexToValue(index)); });
    QObject::connect(item, &ConfigItem::valueChanged, card, [card, valueToIndex](const QVariant &value) {
        const QSignalBlocker blocker(card);
        card->setCurrentIndex(valueToIndex(value));
    });
}

void bindColorSettingCard(ColorSettingCard *card, ColorConfigItem *item)
{
    if (!card || !item) {
        return;
    }

    card->setColor(item->value().value<QColor>());
    QObject::connect(card, &ColorSettingCard::colorChanged, item,
                     [item](const QColor &color) { item->setValue(color); });
    QObject::connect(item, &ConfigItem::valueChanged, card, [card](const QVariant &value) {
        const QSignalBlocker blocker(card);
        card->setColor(value.value<QColor>());
    });
}

void bindCustomColorSettingCard(CustomColorSettingCard *card, ColorConfigItem *item)
{
    if (!card || !item) {
        return;
    }

    card->setDefaultColor(item->defaultValue().value<QColor>());
    card->setColor(item->value().value<QColor>());
    QObject::connect(card, &CustomColorSettingCard::colorChanged, item,
                     [item](const QColor &color) { item->setValue(color); });
    QObject::connect(item, &ConfigItem::valueChanged, card, [card](const QVariant &value) {
        const QSignalBlocker blocker(card);
        card->setColor(value.value<QColor>());
    });
}

void bindRangeSettingCard(RangeSettingCard *card, RangeConfigItem *item)
{
    if (!card || !item) {
        return;
    }

    card->setRange(item->minimum(), item->maximum());
    card->setValue(item->value().toInt());
    QObject::connect(card, &RangeSettingCard::valueChanged, item, [item](int value) { item->setValue(value); });
    QObject::connect(item, &ConfigItem::valueChanged, card, [card](const QVariant &value) {
        const QSignalBlocker blocker(card);
        card->setValue(value.toInt());
    });
}

void bindOptionsSettingCard(OptionsSettingCard *card, OptionsConfigItem *item)
{
    if (!card || !item) {
        return;
    }

    card->setValue(item->value());
    QObject::connect(card, &OptionsSettingCard::valueChanged, item, [item](const QVariant &value) {
        item->setValue(value);
    });
    QObject::connect(item, &ConfigItem::valueChanged, card, [card](const QVariant &value) {
        const QSignalBlocker blocker(card);
        card->setValue(value);
    });
}

} // namespace FluentQt
