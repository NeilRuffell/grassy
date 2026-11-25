/*
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#pragma once
#include <QColor>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace Klassy::ClassicRelief
{
inline QColor shade(const QColor &color, qreal factor)
{
    return QColor::fromHslF(color.hslHueF(),
        std::clamp(qreal(color.hslSaturationF()) * factor, qreal(0), qreal(1)),
        std::clamp(qreal(color.lightnessF()) * factor, qreal(0), qreal(1)), color.alphaF());
}
inline void gloss(QPainter *p, const QRectF &r, const QColor &base, qreal a, qreal b, qreal c, qreal d)
{
    if (r.isEmpty())
        return;
    const qreal half = r.height() / 2;
    const QRectF top(r.x(), r.y(), r.width(), half);
    const QRectF bottom(r.x(), r.y() + half, r.width(), r.height() - half);
    QLinearGradient upper(top.topLeft(), top.bottomLeft());
    upper.setColorAt(0, shade(base, a));
    upper.setColorAt(1, shade(base, b));
    QLinearGradient lower(bottom.topLeft(), bottom.bottomLeft());
    lower.setColorAt(0, shade(base, c));
    lower.setColorAt(1, shade(base, d));
    p->fillRect(top, upper);
    p->fillRect(bottom, lower);
}
inline void titlebar(QPainter *p, const QRectF &r, const QPainterPath &path, const QColor &base,
                     bool active, bool leftEdge, bool rightEdge)
{
    p->save();
    p->setClipPath(path, Qt::IntersectClip);
    if (active)
        gloss(p, r, base, 1.30, 1.00, 0.97, 1.10);
    else
        gloss(p, r, base, 1.05, 1.00, 0.99, 1.00);
    const qreal x = r.x(), y = r.y(), w = r.width(), h = r.height();
    p->setCompositionMode(QPainter::CompositionMode_SourceOver);
    p->fillRect(QRectF(x, y, w, 1), shade(base, active ? 0.55 : 0.45));
    QColor highlight = shade(base, active ? 2.0 : 1.30);
    if (active)
        highlight.setAlphaF(highlight.alphaF() * 0.35);
    p->fillRect(QRectF(x, y + 1, w, 1), highlight);
    p->fillRect(QRectF(x, y + h - 2, w, 1), shade(base, active ? 0.97 : 0.99));
    p->fillRect(QRectF(x, y + h - 1, w, 1), shade(base, 0.70));
    if (!leftEdge) {
        p->fillRect(QRectF(x, y, 1, h), shade(base, active ? 0.32 : 0.45));
        p->fillRect(QRectF(x + 1, y + 2, 1, qMax(qreal(0), h - 3)), highlight);
    }
    if (!rightEdge) {
        p->fillRect(QRectF(x + w - 1, y, 1, h), shade(base, active ? 0.32 : 0.45));
        p->fillRect(QRectF(x + w - 2, y + 2, 1, qMax(qreal(0), h - 3)), shade(base, 0.88));
    }
    p->restore();
}
inline void frame(QPainter *p, const QRectF &r, const QPainterPath &path, const QColor &base,
                  qreal left, qreal right, qreal bottom)
{
    p->save();
    p->setClipPath(path, Qt::IntersectClip);
    const qreal x = r.x(), y = r.y(), w = r.width(), h = r.height();
    const QColor dark = shade(base, 0.25), shadow = shade(base, 0.88), light = shade(base, 1.30);
    if (left > 0) {
        p->fillRect(QRectF(x, y, 1, h), dark);
        if (left >= 2) p->fillRect(QRectF(x + 1, y, 1, h), light);
        if (left >= 3) p->fillRect(QRectF(x + left - 1, y, 1, h), shadow);
    }
    if (right > 0) {
        p->fillRect(QRectF(x + w - 1, y, 1, h), dark);
        if (right >= 2) p->fillRect(QRectF(x + w - 2, y, 1, h), shadow);
        if (right >= 3) p->fillRect(QRectF(x + w - right, y, 1, h), light);
    }
    if (bottom > 0) {
        p->fillRect(QRectF(x, y + h - 1, w, 1), dark);
        if (bottom >= 2) p->fillRect(QRectF(x, y + h - 2, w, 1), shadow);
        if (bottom >= 3) p->fillRect(QRectF(x + left, y + h - bottom, qMax(qreal(0), w - left - right), 1), light);
    }
    p->restore();
}
inline void button(QPainter *p, const QRectF &r, qreal radius, const QColor &base,
                   bool active, bool hovered, bool pressed)
{
    if (r.width() < 4 || r.height() < 4)
        return;
    p->save();
    QPainterPath path;
    path.addRoundedRect(r, radius, radius);
    p->setClipPath(path, Qt::IntersectClip);
    if (active) {
        if (pressed) gloss(p, r, base, 1.15, 0.75, 0.70, 0.75);
        else if (hovered) gloss(p, r, base, 1.45, 1.05, 0.95, 1.05);
        else gloss(p, r, base, 1.35, 0.95, 0.90, 0.95);
    } else {
        if (pressed) gloss(p, r, base, 0.76, 0.75, 0.73, 0.75);
        else if (hovered) gloss(p, r, base, 1.30, 1.10, 1.05, 1.10);
        else gloss(p, r, base, 1.10, 1.00, 0.98, 1.00);
    }
    const qreal x = r.x(), y = r.y(), w = r.width(), h = r.height();
    const QColor light = shade(base, 1.50), dark = shade(base, 0.55);
    const QColor upper = pressed ? dark : light, lower = pressed ? light : dark;
    p->fillRect(QRectF(x + 1, y + 1, w - 2, 1), upper);
    p->fillRect(QRectF(x + 1, y + 1, 1, h - 2), upper);
    p->fillRect(QRectF(x + 1, y + h - 2, w - 2, 1), lower);
    p->fillRect(QRectF(x + w - 2, y + 1, 1, h - 2), lower);
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(shade(base, active ? 0.40 : 0.55), 1));
    p->drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), radius, radius);
    p->restore();
}
}
