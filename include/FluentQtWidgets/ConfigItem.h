#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Theme.h>

#include <functional>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtGui/QColor>

namespace FluentQt {

class SwitchSettingCard;
class ComboBoxSettingCard;
class ColorSettingCard;
class CustomColorSettingCard;
class RangeSettingCard;
class OptionsSettingCard;

class FQW_API ConfigItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

  public:
    ConfigItem(const QString &group, const QString &name, const QVariant &defaultValue, QObject *parent = nullptr);

    QString group() const;
    QString name() const;
    QString key() const;
    QVariant value() const;
    QVariant defaultValue() const;
    bool restartRequired() const;

    void setRestartRequired(bool restart);

  public slots:
    void setValue(const QVariant &value);
    void reset();

  signals:
    void valueChanged(const QVariant &value);

  protected:
    virtual QVariant correctValue(const QVariant &value) const;

    QString m_group;
    QString m_name;
    QVariant m_value;
    QVariant m_defaultValue;
    bool m_restartRequired = false;
};

class FQW_API OptionsConfigItem : public ConfigItem
{
    Q_OBJECT

  public:
    OptionsConfigItem(const QString &group, const QString &name, const QVariant &defaultValue,
                      const QVariantList &options, QObject *parent = nullptr);

    QVariantList options() const;

  protected:
    QVariant correctValue(const QVariant &value) const override;

  private:
    QVariantList m_options;
};

class FQW_API RangeConfigItem : public ConfigItem
{
    Q_OBJECT

  public:
    RangeConfigItem(const QString &group, const QString &name, int defaultValue, int minimum, int maximum,
                    QObject *parent = nullptr);

    int minimum() const;
    int maximum() const;

  protected:
    QVariant correctValue(const QVariant &value) const override;

  private:
    int m_minimum = 0;
    int m_maximum = 100;
};

class FQW_API ColorConfigItem : public ConfigItem
{
    Q_OBJECT

  public:
    ColorConfigItem(const QString &group, const QString &name, const QColor &defaultColor, QObject *parent = nullptr);

  protected:
    QVariant correctValue(const QVariant &value) const override;
};

class FQW_API ConfigItemRegistry : public QObject
{
    Q_OBJECT

  public:
    static ConfigItemRegistry *instance();

    void registerItem(ConfigItem *item);
    ConfigItem *item(const QString &key) const;
    QList<ConfigItem *> items() const;

    void loadFromJson(const QJsonObject &object);
    QJsonObject toJson() const;

  signals:
    void itemValueChanged(ConfigItem *item);

  private:
    explicit ConfigItemRegistry(QObject *parent = nullptr);

    QHash<QString, ConfigItem *> m_items;
};

FQW_API void bindSwitchSettingCard(SwitchSettingCard *card, ConfigItem *item);
FQW_API void bindComboBoxSettingCard(ComboBoxSettingCard *card, OptionsConfigItem *item,
                                     const std::function<QVariant(int)> &indexToValue,
                                     const std::function<int(const QVariant &)> &valueToIndex);
FQW_API void bindColorSettingCard(ColorSettingCard *card, ColorConfigItem *item);
FQW_API void bindCustomColorSettingCard(CustomColorSettingCard *card, ColorConfigItem *item);
FQW_API void bindRangeSettingCard(RangeSettingCard *card, RangeConfigItem *item);
FQW_API void bindOptionsSettingCard(OptionsSettingCard *card, OptionsConfigItem *item);

} // namespace FluentQt
