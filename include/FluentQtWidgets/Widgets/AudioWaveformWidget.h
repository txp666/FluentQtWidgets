#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QVariantList>
#include <QtCore/QVector>
#include <QtGui/QColor>
#include <QtWidgets/QWidget>

class QMouseEvent;
class QPaintEvent;

namespace FluentQt {

class FQW_API AudioWaveformWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QVariantList sampleLevels READ sampleLevels WRITE setSampleLevels NOTIFY samplesChanged)
    Q_PROPERTY(qreal progress READ progress WRITE setProgress NOTIFY progressChanged)
    Q_PROPERTY(qreal barWidth READ barWidth WRITE setBarWidth NOTIFY metricsChanged)
    Q_PROPERTY(qreal barGap READ barGap WRITE setBarGap NOTIFY metricsChanged)
    Q_PROPERTY(qreal minBarHeight READ minBarHeight WRITE setMinBarHeight NOTIFY metricsChanged)
    Q_PROPERTY(QColor lightPlayedColor READ lightPlayedColor WRITE setLightPlayedColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor darkPlayedColor READ darkPlayedColor WRITE setDarkPlayedColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor lightRemainingColor READ lightRemainingColor WRITE setLightRemainingColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor darkRemainingColor READ darkRemainingColor WRITE setDarkRemainingColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor lightBackgroundColor READ lightBackgroundColor WRITE setLightBackgroundColor NOTIFY colorsChanged)
    Q_PROPERTY(QColor darkBackgroundColor READ darkBackgroundColor WRITE setDarkBackgroundColor NOTIFY colorsChanged)

  public:
    explicit AudioWaveformWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    QVector<qreal> samples() const;
    QVariantList sampleLevels() const;
    qreal progress() const;
    qreal barWidth() const;
    qreal barGap() const;
    qreal minBarHeight() const;
    QColor lightPlayedColor() const;
    QColor darkPlayedColor() const;
    QColor lightRemainingColor() const;
    QColor darkRemainingColor() const;
    QColor lightBackgroundColor() const;
    QColor darkBackgroundColor() const;

  public slots:
    void setSamples(const QVector<qreal> &samples);
    void setSampleLevels(const QVariantList &samples);
    void setProgress(qreal progress);
    void setBarWidth(qreal width);
    void setBarGap(qreal gap);
    void setMinBarHeight(qreal height);
    void setLightPlayedColor(const QColor &color);
    void setDarkPlayedColor(const QColor &color);
    void setLightRemainingColor(const QColor &color);
    void setDarkRemainingColor(const QColor &color);
    void setLightBackgroundColor(const QColor &color);
    void setDarkBackgroundColor(const QColor &color);
    void setCustomWaveformColor(const QColor &lightPlayed, const QColor &darkPlayed,
                                const QColor &lightRemaining = QColor(), const QColor &darkRemaining = QColor());
    void clear();

  signals:
    void samplesChanged();
    void progressChanged(qreal progress);
    void metricsChanged();
    void colorsChanged();
    void waveformClicked(qreal progress);
    void waveformMoved(qreal progress);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    QColor playedColor() const;
    QColor remainingColor() const;
    QColor backgroundColor() const;
    QVector<qreal> displaySamples(int maximumBars) const;
    qreal progressForPosition(qreal x) const;
    void updateProgressFromPosition(qreal x, bool moving);

    QVector<qreal> m_samples;
    qreal m_progress = 0;
    qreal m_barWidth = 2.0;
    qreal m_barGap = 3.0;
    qreal m_minBarHeight = 2.0;
    QColor m_lightPlayedColor = QColor(36, 36, 36, 190);
    QColor m_darkPlayedColor = QColor(255, 255, 255, 210);
    QColor m_lightRemainingColor = QColor(36, 36, 36, 36);
    QColor m_darkRemainingColor = QColor(255, 255, 255, 46);
    QColor m_lightBackgroundColor = QColor(255, 255, 255);
    QColor m_darkBackgroundColor = QColor(32, 32, 32);
    bool m_dragging = false;
};

} // namespace FluentQt
