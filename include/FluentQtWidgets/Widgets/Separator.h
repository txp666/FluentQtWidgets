#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtWidgets/QWidget>

class QPaintEvent;

namespace FluentQt {

class FQW_API HorizontalSeparator : public QWidget
{
    Q_OBJECT

public:
    explicit HorizontalSeparator(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

class FQW_API VerticalSeparator : public QWidget
{
    Q_OBJECT

public:
    explicit VerticalSeparator(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};

} // namespace FluentQt
