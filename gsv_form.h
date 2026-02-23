#ifndef GSV_FORM_H
#define GSV_FORM_H

#include <QWidget>
#include <StructData.h>
#include <QPainter>

namespace Ui {
class GSV_Form;
}

class GSV_Form : public QWidget
{
    Q_OBJECT

public:
    explicit GSV_Form(QWidget *parent = nullptr);
    ~GSV_Form();
    void m_updateConstellation(GnssSystem system, const QVector<SkySat> &sats);
protected:
    void paintEvent(QPaintEvent *event) override;

private:
    Ui::GSV_Form *ui;
    QMap<GnssSystem, QMap<int, SkySat>> m_sats;
    QColor m_colorForSystem(GnssSystem sys) const;
    void m_purgeStaleSatellites(int maxAgeMs = 15000);
    // void m_drawLegend(QPainter &p, const QPointF &topLeft);
    void m_drawLegend(QPainter &p);
};

#endif // GSV_FORM_H
