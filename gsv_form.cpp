#include "gsv_form.h"
#include "ui_gsv_form.h"

GSV_Form::GSV_Form(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GSV_Form)
{
    ui->setupUi(this);
}

GSV_Form::~GSV_Form()
{
    delete ui;
}

void GSV_Form::m_updateConstellation(GnssSystem system, const QVector<SkySat> &sats)
{
    QMap<int, SkySat> &bucket = m_sats[system];
    bucket.clear();

    for (SkySat s : sats) {
        s.lastSeen.start();
        bucket[s.prn] = s;
    }

    update();
}


QColor GSV_Form::m_colorForSystem(GnssSystem sys) const
{
    switch (sys) {
    case GnssSystem::GPS:      return QColor(250, 128, 114);
    case GnssSystem::GLONASS:  return QColor(93, 63, 211);
    case GnssSystem::BEIDOU:   return Qt::green;
    case GnssSystem::GALILEO:  return QColor(128, 0, 128);
    default:                  return Qt::gray;
    }
}

void GSV_Form::paintEvent(QPaintEvent *)
{
    m_purgeStaleSatellites();

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.fillRect(rect(), Qt::black);

    const QPointF center(width() / 2.0, height() / 2.0);
    const double R = qMin(width(), height()) * 0.45;

    /* ---------- Grid ---------- */

    p.setPen(QPen(Qt::white, 2));
    p.drawEllipse(center, R, R);

    p.setPen(QPen(Qt::gray, 1, Qt::DashLine));
    p.drawEllipse(center, R * 2 / 3, R * 2 / 3);  // 30°
    p.drawEllipse(center, R / 3, R / 3);          // 60°

    p.drawLine(center.x() - R, center.y(),
               center.x() + R, center.y());
    p.drawLine(center.x(), center.y() - R,
               center.x(), center.y() + R);

    p.setPen(Qt::white);
    p.drawText(center + QPointF(-6, -R - 5), "N");
    p.drawText(center + QPointF(R + 5, 5), "E");
    p.drawText(center + QPointF(-6, R + 15), "S");
    p.drawText(center + QPointF(-R - 15, 5), "W");

    /* ---------- Satellites ---------- */

    for (const auto &bucket : m_sats) {
        for (const SkySat &sat : bucket) {

            double r = (90.0 - sat.elevation) / 90.0;
            double az = qDegreesToRadians(sat.azimuth);

            double x = r * sin(az);
            double y = r * cos(az);

            QPointF pos(center.x() + x * R,
                        center.y() - y * R);

            QColor color = m_colorForSystem(sat.system);

            int size = (sat.snr > 30) ? 10 :
                           (sat.snr > 20) ? 8 : 6;

            p.setBrush(color);
            p.setPen(Qt::NoPen);
            p.drawEllipse(pos, size / 2.0, size / 2.0);

            p.setPen(Qt::white);
            p.drawText(pos + QPointF(5, -5),
                       QString::number(sat.prn));
        }
    }

    m_drawLegend(p, QPointF(10, 10));
}

void GSV_Form::m_purgeStaleSatellites(int maxAgeMs)
{
    for (auto &bucket : m_sats) {
        auto it = bucket.begin();
        while (it != bucket.end()) {
            if (it->lastSeen.elapsed() > maxAgeMs)
                it = bucket.erase(it);
            else
                ++it;
        }
    }
}


void GSV_Form::m_drawLegend(QPainter &p, const QPointF &topLeft)
{
    const int boxSize = 10;
    const int spacing = 6;
    const int lineHeight = 18;

    struct LegendItem {
        QString label;
        QColor color;
    };

    QVector<LegendItem> items = {
        { "GPS",      m_colorForSystem(GnssSystem::GPS) },
        { "GLONASS",  m_colorForSystem(GnssSystem::GLONASS) },
        { "BEIDOU",   m_colorForSystem(GnssSystem::BEIDOU) },
        { "GALILEO",  m_colorForSystem(GnssSystem::GALILEO) }
    };

    QPointF pos = topLeft;

    // Background
    QRectF bg(pos.x() - 6, pos.y() - 6,
              120, items.size() * lineHeight + 8);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 160)); // semi-transparent
    p.drawRoundedRect(bg, 6, 6);

    QPen boxPen(Qt::white);
    boxPen.setWidth(1);

    // Legend items
    for (const auto &item : items)
    {
        p.setPen(boxPen);
        p.setBrush(item.color);

        p.drawRect(QRectF(pos.x(),
                          pos.y() + 4,
                          boxSize,
                          boxSize));

        // Text
        p.setPen(Qt::white);
        p.drawText(QPointF(pos.x() + boxSize + spacing,
                           pos.y() + boxSize + 2),
                   item.label);

        pos.ry() += lineHeight;
    }

}
