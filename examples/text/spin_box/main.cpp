#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QtGlobal>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
  public:
    explicit Demo(QWidget *parent = nullptr) : QWidget(parent)
    {
        setAutoFillBackground(true);
        QPalette p = palette();
        p.setColor(QPalette::Window, Qt::white);
        setPalette(p);

        auto *spinBox = new SpinBox(this);
        auto *compactSpinBox = new CompactSpinBox(this);
        spinBox->setAccelerated(true);
        compactSpinBox->setAccelerated(true);

        auto *doubleSpinBox = new DoubleSpinBox(this);
        auto *compactDoubleSpinBox = new CompactDoubleSpinBox(this);
        auto *timeEdit = new TimeEdit(this);
        auto *compactTimeEdit = new CompactTimeEdit(this);
        auto *dateEdit = new DateEdit(this);
        auto *compactDateEdit = new CompactDateEdit(this);
        auto *dateTimeEdit = new DateTimeEdit(this);
        auto *compactDateTimeEdit = new CompactDateTimeEdit(this);

        auto *layout = new QGridLayout(this);
        layout->setHorizontalSpacing(30);
        layout->setContentsMargins(100, 50, 100, 50);
        layout->addWidget(spinBox, 0, 0);
        layout->addWidget(compactSpinBox, 0, 1, Qt::AlignLeft);
        layout->addWidget(doubleSpinBox, 1, 0);
        layout->addWidget(compactDoubleSpinBox, 1, 1, Qt::AlignLeft);
        layout->addWidget(timeEdit, 2, 0);
        layout->addWidget(compactTimeEdit, 2, 1, Qt::AlignLeft);
        layout->addWidget(dateEdit, 3, 0);
        layout->addWidget(compactDateEdit, 3, 1, Qt::AlignLeft);
        layout->addWidget(dateTimeEdit, 4, 0);
        layout->addWidget(compactDateTimeEdit, 4, 1, Qt::AlignLeft);
        resize(500, 500);
    }
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication app(argc, argv);
    Demo w;
    w.show();
    return QApplication::exec();
}
