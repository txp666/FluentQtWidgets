#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QJsonObject>
#include <QtCore/QSize>
#include <QtWidgets/QWidget>

class QResizeEvent;

namespace FluentQt {

class FQW_API ChartWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QJsonObject option READ option WRITE setOption NOTIFY optionChanged)
    Q_PROPERTY(QString chartTheme READ chartTheme WRITE setChartTheme NOTIFY chartThemeChanged)

  public:
    explicit ChartWidget(QWidget *parent = nullptr);
    explicit ChartWidget(const QJsonObject &option, QWidget *parent = nullptr);

    QJsonObject option() const;
    QString chartTheme() const;
    QSize sizeHint() const override;

  public slots:
    void setOption(const QJsonObject &option);
    void setOptionJson(const QString &json);
    void setChartTheme(const QString &theme);
    void reload();

  signals:
    void optionChanged(const QJsonObject &option);
    void chartThemeChanged(const QString &theme);
    void loadFinished(bool ok);

  protected:
    void resizeEvent(QResizeEvent *event) override;

  private:
    void init();
    void renderChart();
    void applyPendingOption();
    void resizeChart();
    QString resolvedChartTheme() const;

    QJsonObject m_option;
    QString m_chartTheme = QStringLiteral("auto");
    QWidget *m_view = nullptr;
    bool m_loaded = false;
};

} // namespace FluentQt
