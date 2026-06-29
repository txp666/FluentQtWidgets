#pragma once

#include "../../GalleryTranslator.h"
#include "../../components/GalleryComponents.h"
#include <FluentQtWidgets/Repository.h>

#include <FluentQtWidgets/ConfigItem.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDate>
#include <QtCore/QDir>
#include <QtCore/QPoint>
#include <QtCore/QPointer>
#include <QtCore/QStandardPaths>
#include <QtCore/QTimer>
#include <QtCore/QUrl>
#include <QtGui/QActionGroup>
#include <QtGui/QColor>
#include <QtGui/QFont>
#include <QtGui/QPainterPath>
#include <QtGui/QPixmap>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QVBoxLayout>

#include <memory>

inline QString tx(const char *context, const char *source)
{
    return QCoreApplication::translate(context, source);
}

inline QString mainTx(const char *source) { return tx("MainWindow", source); }

inline QString navTx(const char *source) { return tx("Translator", source); }

inline QString settingTx(const char *source) { return tx("SettingInterface", source); }

inline QString exampleSourceUrl(const char *examplePath)
{
    return QStringLiteral(FQW_REPOSITORY_URL "/blob/main/examples/%1/main.cpp")
        .arg(QString::fromLatin1(examplePath));
}

class ProfileCard : public QWidget
{
  public:
    explicit ProfileCard(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedSize(307, 82);

        auto *avatar = new FluentQt::AvatarWidget(this);
        avatar->setImagePath(QStringLiteral(":/gallery/images/shoko.png"));
        avatar->setRadius(24);
        avatar->move(2, 6);

        auto *nameLabel = new FluentQt::BodyLabel(QStringLiteral("Shoko"), this);
        nameLabel->move(64, 13);

        auto *emailLabel = new FluentQt::CaptionLabel(QStringLiteral("shokokawaii@outlook.com"), this);
        emailLabel->setTextColor(QColor(96, 96, 96), QColor(206, 206, 206));
        emailLabel->move(64, 32);

        auto *logoutButton = new FluentQt::HyperlinkButton(QStringLiteral(FQW_REPOSITORY_URL),
                                                           QStringLiteral("Logout"), this);
        logoutButton->move(52, 48);
    }
};

class AcrylicBrushCard : public QWidget
{
  public:
    explicit AcrylicBrushCard(QWidget *parent = nullptr)
        : QWidget(parent)
        , m_acrylicBrush(this, 15)
    {
        setFixedSize(180, 180);
        m_acrylicBrush.setImagePath(QStringLiteral(":/gallery/images/shoko.png"));
        updateClipPath();
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

    FluentQt::AcrylicBrush m_acrylicBrush;
};
