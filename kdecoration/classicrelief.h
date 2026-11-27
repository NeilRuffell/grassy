/*
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */
#pragma once
#include "breeze.h"
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
// Logical-coordinate, stepped Metacity silhouette. The five rows are
// mirrored at eligible corners; no raster masks are used.
inline QPainterPath steppedPath(const QRectF &r, Corners corners, qreal radius)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    const qreal u = std::min({radius / 5, r.width() / 10, r.height() / 10});
    if (u <= 0 || r.isEmpty()) {
        path.addRect(r);
        return path;
    }
    constexpr int inset[] = {5, 3, 2, 1, 1};
    for (int i = 0; i < 5; ++i) {
        const qreal a = corners.testFlag(CornerTopLeft) ? inset[i] * u : 0;
        const qreal b = corners.testFlag(CornerTopRight) ? inset[i] * u : 0;
        path.addRect(QRectF(r.left() + a, r.top() + i * u, r.width() - a - b, u));
        const qreal c = corners.testFlag(CornerBottomLeft) ? inset[i] * u : 0;
        const qreal d = corners.testFlag(CornerBottomRight) ? inset[i] * u : 0;
        path.addRect(QRectF(r.left() + c, r.bottom() - (i + 1) * u, r.width() - c - d, u));
    }
    path.addRect(QRectF(r.left(), r.top() + 5 * u, r.width(), r.height() - 10 * u));
    return path.simplified();
}

inline QColor overlay(QColor c, qreal alpha)
{
    c.setAlphaF(c.alphaF() * alpha);
    return c;
}

// Draw the historical 10-unit glyph silhouettes as vector primitives, with
// the offset dark relief behind the foreground. Return false for other buttons.
inline bool glyph(QPainter *p, const QRectF &area, DecorationButtonType type,
                  bool checked, const QColor &foreground, const QColor &base, bool active)
{
    if (type != DecorationButtonType::Minimize && type != DecorationButtonType::Maximize
        && type != DecorationButtonType::Close)
        return false;
    const qreal side = qMin(area.width(), area.height());
    if (side <= 0)
        return true;
    p->save();
    p->translate(area.center() - QPointF(side / 2, side / 2));
    p->scale(side / 10, side / 10);
    p->setRenderHint(QPainter::Antialiasing, true);
    QPainterPath symbol;
    symbol.setFillRule(Qt::WindingFill);
    const auto box = [&symbol](qreal x, qreal y, qreal w, qreal h) {
        symbol.addRect(QRectF(x, y, w, 1.7));
        symbol.addRect(QRectF(x, y + 1.7, 1.2, h - 2.9));
        symbol.addRect(QRectF(x + w - 1.2, y + 1.7, 1.2, h - 2.9));
        symbol.addRect(QRectF(x, y + h - 1.2, w, 1.2));
    };
    if (type == DecorationButtonType::Minimize) {
        symbol.addRect(QRectF(1, 7, 8, 1.7));
    } else if (type == DecorationButtonType::Maximize) {
        if (!checked) {
            box(1, 1, 8, 8);
        } else {
            // Back window visible above and to the right of the front window.
            symbol.addRect(QRectF(3, 0.7, 6.3, 1.3));
            symbol.addRect(QRectF(8, 2, 1.3, 5));
            box(0.7, 3, 6.3, 6.3);
        }
    } else {
        QPainterPath lines;
        lines.moveTo(1.5, 1.5);
        lines.lineTo(8.5, 8.5);
        lines.moveTo(8.5, 1.5);
        lines.lineTo(1.5, 8.5);
        QPainterPathStroker stroker;
        stroker.setWidth(1.8);
        stroker.setCapStyle(Qt::SquareCap);
        symbol = stroker.createStroke(lines);
    }
    p->save();
    p->translate(0.6, 0.8);
    p->fillPath(symbol, overlay(shade(base, 0.35), active ? 0.85 : 0.45));
    p->restore();
    p->fillPath(symbol, foreground);
    p->restore();
    return true;
}

inline void gloss(QPainter *p, const QRectF &r, const QColor &base, qreal a, qreal b, qreal c, qreal d)
{
    if (r.isEmpty())
        return;
    // Paint the two-stage gloss in one operation. Separate antialiased
    // rectangles can reveal the underlying frame at their fractional boundary.
    // Closely spaced midpoint stops preserve the intended sharp colour change
    // without painting a separate line or leaving any uncovered pixels.
    QLinearGradient gradient(r.topLeft(), r.bottomLeft());
    gradient.setColorAt(0.0, shade(base, a));
    gradient.setColorAt(0.499, shade(base, b));
    gradient.setColorAt(0.501, shade(base, c));
    gradient.setColorAt(1.0, shade(base, d));
    p->fillRect(r, gradient);
}
inline void titlebar(QPainter *p, const QRectF &r, const QPainterPath &path, const QColor &base,
                     bool active, bool leftEdge, bool rightEdge)
{
    p->save();
    p->setClipPath(path, Qt::IntersectClip);
    // Start the upper gradient below the outer outline, as in Human.
    const QRectF interior = r.adjusted(0, 1, 0, 0);
    if (active)
        gloss(p, interior, base, 1.30, 1.00, 0.97, 1.10);
    else
        gloss(p, interior, base, 1.05, 1.00, 0.99, 1.00);
    const qreal x = r.x(), y = r.y(), w = r.width(), h = r.height();
    p->setCompositionMode(QPainter::CompositionMode_SourceOver);
    p->fillRect(QRectF(x, y, w, 1), shade(base, active ? 0.55 : 0.45));
    QColor highlight = shade(base, active ? 2.0 : 1.30);
    if (active)
        highlight.setAlphaF(highlight.alphaF() * 0.35);
    p->fillRect(QRectF(x, y + 1, w, 1), highlight);
    if (active && w > 14)
        p->fillRect(QRectF(x + 7, y + 1, w - 14, 1), overlay(Qt::white, 0.10));
    // Preserve the step's boundary pixels, not just the horizontal edges.
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(shade(base, active ? 0.48 : 0.45), 1.6));
    p->drawPath(path);
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
    const QColor inner = shade(base, 0.76);
    if (left > 0) {
        p->fillRect(QRectF(x, y, 1, h), dark);
        if (left >= 2) p->fillRect(QRectF(x + 1, y, 1, h), light);
        if (left >= 3) p->fillRect(QRectF(x + left - 1, y, 1, h), inner);
        if (left >= 4) p->fillRect(QRectF(x + left - 2, y, 1, h), shadow);
    }
    if (right > 0) {
        p->fillRect(QRectF(x + w - 1, y, 1, h), dark);
        if (right >= 2) p->fillRect(QRectF(x + w - 2, y, 1, h), shadow);
        if (right >= 3) p->fillRect(QRectF(x + w - right, y, 1, h), inner);
        if (right >= 4) p->fillRect(QRectF(x + w - right + 1, y, 1, h), light);
    }
    if (bottom > 0) {
        p->fillRect(QRectF(x, y + h - 1, w, 1), dark);
        if (bottom >= 2) p->fillRect(QRectF(x, y + h - 2, w, 1), shadow);
        if (bottom >= 3) p->fillRect(QRectF(x + left, y + h - bottom, qMax(qreal(0), w - left - right), 1), inner);
        if (bottom >= 4) p->fillRect(QRectF(x + left, y + h - bottom + 1, qMax(qreal(0), w - left - right), 1), light);
    }
    // Trace the stepped outer perimeter only where a real frame exists.
    p->save();
    QPainterPath borders;
    if (left > 0) borders.addRect(QRectF(x, y, left, h));
    if (right > 0) borders.addRect(QRectF(x + w - right, y, right, h));
    if (bottom > 0) borders.addRect(QRectF(x, y + h - bottom, w, bottom));
    borders.setFillRule(Qt::WindingFill);
    p->setClipPath(borders, Qt::IntersectClip);
    p->setBrush(Qt::NoBrush);
    p->setPen(QPen(dark, 1.6));
    p->drawPath(path);
    p->restore();
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
    const QColor light = overlay(Qt::white, 0.46), dark = overlay(Qt::black, 0.42);
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
