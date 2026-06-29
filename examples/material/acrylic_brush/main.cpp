#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtGui/QPainterPath>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

using namespace FluentQt;

class Demo : public QWidget
{
public:
    explicit Demo(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_acrylicBrush(this, 15)
    {
        resize(400, 400);
        updateClipPath();
        m_acrylicBrush.setImagePath(QStringLiteral(":/material/acrylic_brush/resource/shoko.png"));
    }

protected:
    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);
        updateClipPath();
    }

    void paintEvent(QPaintEvent *event) override
    {
        m_acrylicBrush.paint();
        QWidget::paintEvent(event);
    }

private:
    void updateClipPath()
    {
        QPainterPath path;
        path.addEllipse(rect());
        m_acrylicBrush.setClipPath(path);
    }

    AcrylicBrush m_acrylicBrush;
};

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
    QApplication app(argc, argv);

    Demo window;
    window.show();

    return app.exec();
}
