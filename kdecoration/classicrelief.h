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

// Recreate compact Human glyphs in a scalable 10-unit design space.
// The surrounding relief is a vector outline, not a displaced drop shadow.
// Return false for other buttons.
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
    if (type == DecorationButtonType::Maximize) {
        // Draw the window outlines directly. Combining tiny rectangular
        // fills with path simplification can lose edges at this scale.
        QPainterPath windows;
        if (!checked) {
            windows.addRect(QRectF(1.8, 1.8, 6.4, 6.4));
        } else {
            // Only the exposed portion of the rear window is drawn.
            // The front window is a complete, independent closed outline.
            windows.moveTo(3.6, 3.0);
            windows.lineTo(3.6, 1.5);
            windows.lineTo(8.5, 1.5);
            windows.lineTo(8.5, 6.4);
            windows.lineTo(7.0, 6.4);
            windows.addRect(QRectF(1.5, 3.6, 4.9, 4.9));
        }

        p->setBrush(Qt::NoBrush);
        QPen outline(overlay(shade(base, 0.35), active ? 0.72 : 0.50));
        outline.setWidthF(checked ? 1.9 : 2.2);
        outline.setJoinStyle(Qt::MiterJoin);
        outline.setCapStyle(Qt::FlatCap);
        p->setPen(outline);
        p->drawPath(windows);

        QPen face(foreground);
        face.setWidthF(checked ? 1.2 : 1.45);
        face.setJoinStyle(Qt::MiterJoin);
        face.setCapStyle(Qt::FlatCap);
        p->setPen(face);
        p->drawPath(windows);
        p->restore();
        return true;
    }

    // Keep the already accepted minimize and close glyphs unchanged.
    QPainterPath symbol;
    symbol.setFillRule(Qt::WindingFill);
    if (type == DecorationButtonType::Minimize) {
        symbol.addRoundedRect(QRectF(1.6, 6.6, 6.8, 1.8), 0.3, 0.3);
    } else {
        QPainterPath lines;
        lines.moveTo(2.5, 2.5);
        lines.lineTo(7.5, 7.5);
        lines.moveTo(7.5, 2.5);
        lines.lineTo(2.5, 7.5);
        QPainterPathStroker stroker;
        stroker.setWidth(2.3);
        stroker.setCapStyle(Qt::SquareCap);
        symbol = stroker.createStroke(lines);
    }
    symbol.setFillRule(Qt::WindingFill);
    symbol = symbol.simplified();
    QPainterPathStroker relief;
    relief.setWidth(1.25);
    relief.setJoinStyle(Qt::RoundJoin);
    p->fillPath(relief.createStroke(symbol).united(symbol),
                overlay(shade(base, 0.35), active ? 0.72 : 0.50));
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
    // Human's 26x24 reference button slot contains a 24x18 visible
    // surround. Its two one-unit outlines are concentric, not a
    // conventional top-lit raised bevel. Scale the layers with the surface.
    const qreal unit = qMin(r.width() / 24.0, r.height() / 18.0);
    const QRectF inner = r.adjusted(unit, unit, -unit, -unit);
    const qreal outerRadius = qBound(qreal(0), radius, qMin(r.width(), r.height()) / 2);
    const qreal innerRadius = qMax(qreal(0), outerRadius - unit);
    QPainterPath outerPath;
    outerPath.addRoundedRect(r, outerRadius, outerRadius);
    QPainterPath innerPath;
    innerPath.addRoundedRect(inner, innerRadius, innerRadius);

    // The translucent outer outline is composited directly over the
    // titlebar. Do not fill the outline with the button's gradient first.
    p->fillPath(outerPath.subtracted(innerPath), overlay(Qt::black, 0.45));
    p->save();
    p->setClipPath(innerPath, Qt::IntersectClip);
    if (active) {
        if (pressed) gloss(p, inner, base, 1.15, 0.75, 0.70, 0.75);
        else if (hovered) gloss(p, inner, base, 1.45, 1.05, 0.95, 1.05);
        else gloss(p, inner, base, 1.35, 0.95, 0.90, 0.95);
    } else {
        if (pressed) gloss(p, inner, base, 0.76, 0.75, 0.73, 0.75);
        else if (hovered) gloss(p, inner, base, 1.30, 1.10, 1.05, 1.10);
        else gloss(p, inner, base, 1.10, 1.00, 0.98, 1.00);
    }

    // The reference uses the same white40 inner outline on every edge
    // even when pressed; the pressed state changes the gradient instead.
    const QRectF core = inner.adjusted(unit, unit, -unit, -unit);
    QPainterPath corePath;
    const qreal coreRadius = qMax(qreal(0), innerRadius - unit);
    corePath.addRoundedRect(core, coreRadius, coreRadius);
    const QPainterPath innerHighlight = innerPath.subtracted(corePath);
    p->fillPath(innerHighlight, overlay(Qt::white, 0.40));

    // Slightly darken the four highlight corner junctions, matching the
    // reference's black20 accents without relying on bitmap corner masks.
    p->setClipPath(innerHighlight, Qt::IntersectClip);
    const QColor corner = overlay(Qt::black, 0.20);
    for (const qreal x : {inner.left(), inner.right() - unit}) {
        for (const qreal y : {inner.top(), inner.bottom() - unit})
            p->fillRect(QRectF(x, y, unit, unit), corner);
    }
    p->restore();
    p->restore();
}
}
