#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QList>
#include <QtCore/Qt>
#include <QtWidgets/QVBoxLayout>

class QWidget;

namespace FluentQt {

class FQW_API VBoxLayout : public QVBoxLayout
{
    Q_OBJECT

  public:
    explicit VBoxLayout(QWidget *parent = nullptr);

    const QList<QWidget *> &widgets() const;

    void addWidgets(const QList<QWidget *> &widgets, int stretch = 0, Qt::Alignment alignment = Qt::AlignTop);
    void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::AlignTop);
    void removeWidget(QWidget *widget);
    void deleteWidget(QWidget *widget);
    void removeAllWidget();

  private:
    QList<QWidget *> m_widgets;
};

} // namespace FluentQt
