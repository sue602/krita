/* This file is part of the KDE project
 * Copyright (C) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SvgUtil.h"
#include "SvgGraphicContext.h"

#include <KoUnit.h>
#include <KoXmlReader.h>

#include <QString>
#include <QRectF>
#include <QStringList>
#include <QFontMetrics>
#include <QRegExp>

#include <math.h>
#include "kis_debug.h"

#define DPI 72.0

#define DEG2RAD(degree) degree/180.0*M_PI

double SvgUtil::fromUserSpace(double value)
{
    return value;
}

double SvgUtil::toUserSpace(double value)
{
    return value;
}

double SvgUtil::ptToPx(SvgGraphicsContext *gc, double value)
{
    return value * gc->pixelsPerInch / DPI;
}

QPointF SvgUtil::toUserSpace(const QPointF &point)
{
    return QPointF(toUserSpace(point.x()), toUserSpace(point.y()));
}

QRectF SvgUtil::toUserSpace(const QRectF &rect)
{
    return QRectF(toUserSpace(rect.topLeft()), toUserSpace(rect.size()));
}

QSizeF SvgUtil::toUserSpace(const QSizeF &size)
{
    return QSizeF(toUserSpace(size.width()), toUserSpace(size.height()));
}

double SvgUtil::toPercentage(QString s)
{
    if (s.endsWith('%'))
        return s.remove('%').toDouble();
    else
        return s.toDouble() * 100.0;
}

double SvgUtil::fromPercentage(QString s)
{
    if (s.endsWith('%'))
        return s.remove('%').toDouble() / 100.0;
    else
        return s.toDouble();
}

QPointF SvgUtil::objectToUserSpace(const QPointF &position, const QRectF &objectBound)
{
    qreal x = objectBound.left() + position.x() * objectBound.width();
    qreal y = objectBound.top() + position.y() * objectBound.height();
    return QPointF(x, y);
}

QSizeF SvgUtil::objectToUserSpace(const QSizeF &size, const QRectF &objectBound)
{
    qreal w = size.width() * objectBound.width();
    qreal h = size.height() * objectBound.height();
    return QSizeF(w, h);
}

QPointF SvgUtil::userSpaceToObject(const QPointF &position, const QRectF &objectBound)
{
    qreal x = 0.0;
    if (objectBound.width() != 0)
        x = (position.x() - objectBound.x()) / objectBound.width();
    qreal y = 0.0;
    if (objectBound.height() != 0)
        y = (position.y() - objectBound.y()) / objectBound.height();
    return QPointF(x, y);
}

QSizeF SvgUtil::userSpaceToObject(const QSizeF &size, const QRectF &objectBound)
{
    qreal w = objectBound.width() != 0 ? size.width() / objectBound.width() : 0.0;
    qreal h = objectBound.height() != 0 ? size.height() / objectBound.height() : 0.0;
    return QSizeF(w, h);
}

QTransform SvgUtil::parseTransform(const QString &transform)
{
    QTransform result;

    // Split string for handling 1 transform statement at a time
    QStringList subtransforms = transform.split(')', QString::SkipEmptyParts);
    QStringList::ConstIterator it = subtransforms.constBegin();
    QStringList::ConstIterator end = subtransforms.constEnd();
    for (; it != end; ++it) {
        QStringList subtransform = (*it).simplified().split('(', QString::SkipEmptyParts);
        if (subtransform.count() < 2)
            continue;

        subtransform[0] = subtransform[0].trimmed().toLower();
        subtransform[1] = subtransform[1].simplified();
        QRegExp reg("[,( ]");
        QStringList params = subtransform[1].split(reg, QString::SkipEmptyParts);

        if (subtransform[0].startsWith(';') || subtransform[0].startsWith(','))
            subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

        if (subtransform[0] == "rotate") {
            if (params.count() == 3) {
                double x = params[1].toDouble();
                double y = params[2].toDouble();

                result.translate(x, y);
                result.rotate(params[0].toDouble());
                result.translate(-x, -y);
            } else {
                result.rotate(params[0].toDouble());
            }
        } else if (subtransform[0] == "translate") {
            if (params.count() == 2) {
                result.translate(SvgUtil::fromUserSpace(params[0].toDouble()),
                                 SvgUtil::fromUserSpace(params[1].toDouble()));
            } else {   // Spec : if only one param given, assume 2nd param to be 0
                result.translate(SvgUtil::fromUserSpace(params[0].toDouble()) , 0);
            }
        } else if (subtransform[0] == "scale") {
            if (params.count() == 2) {
                result.scale(params[0].toDouble(), params[1].toDouble());
            } else {   // Spec : if only one param given, assume uniform scaling
                result.scale(params[0].toDouble(), params[0].toDouble());
            }
        } else if (subtransform[0].toLower() == "skewx") {
            result.shear(tan(DEG2RAD(params[0].toDouble())), 0.0F);
        } else if (subtransform[0].toLower() == "skewy") {
            result.shear(0.0F, tan(DEG2RAD(params[0].toDouble())));
        } else if (subtransform[0] == "matrix") {
            if (params.count() >= 6) {
                result.setMatrix(params[0].toDouble(), params[1].toDouble(), 0,
                                 params[2].toDouble(), params[3].toDouble(), 0,
                                 SvgUtil::fromUserSpace(params[4].toDouble()),
                                 SvgUtil::fromUserSpace(params[5].toDouble()), 1);
            }
        }
    }

    return result;
}

QString SvgUtil::transformToString(const QTransform &transform)
{
    if (transform.isIdentity())
        return QString();

    if (transform.type() == QTransform::TxTranslate) {
        return QString("translate(%1, %2)")
                     .arg(toUserSpace(transform.dx()))
                     .arg(toUserSpace(transform.dy()));
    } else {
        return QString("matrix(%1 %2 %3 %4 %5 %6)")
                     .arg(transform.m11()).arg(transform.m12())
                     .arg(transform.m21()).arg(transform.m22())
                     .arg(toUserSpace(transform.dx()))
                     .arg(toUserSpace(transform.dy()));
    }
}

bool SvgUtil::parseViewBox(SvgGraphicsContext *gc, const KoXmlElement &e,
                           const QRectF &elementBounds,
                           QRectF *_viewRect, QTransform *_viewTransform)
{
    KIS_ASSERT(_viewRect);
    KIS_ASSERT(_viewTransform);

    QString viewBoxStr = e.attribute("viewBox");
    if (viewBoxStr.isEmpty()) return false;

    bool result = false;

    QRectF viewBoxRect;
    // this is a workaround for bug 260429 for a file generated by blender
    // who has px in the viewbox which is wrong.
    // reported as bug http://projects.blender.org/tracker/?group_id=9&atid=498&func=detail&aid=30971
    viewBoxStr.remove("px");

    QStringList points = viewBoxStr.replace(',', ' ').simplified().split(' ');
    if (points.count() == 4) {
        viewBoxRect.setX(SvgUtil::fromUserSpace(points[0].toFloat()));
        viewBoxRect.setY(SvgUtil::fromUserSpace(points[1].toFloat()));
        viewBoxRect.setWidth(SvgUtil::fromUserSpace(points[2].toFloat()));
        viewBoxRect.setHeight(SvgUtil::fromUserSpace(points[3].toFloat()));

        result = true;
    } else {
        // TODO: WARNING!
    }

    if (!result) return false;

    QTransform viewBoxTransform =
        QTransform::fromTranslate(-viewBoxRect.x(), -viewBoxRect.y()) *
        QTransform::fromScale(elementBounds.width() / viewBoxRect.width(),
                                  elementBounds.height() / viewBoxRect.height());;

    const QString aspectString = e.attribute("preserveAspectRatio");
    if (!aspectString.isEmpty()) {
        PreserveAspectRatioParser p(aspectString);
        if (p.mode != Qt::IgnoreAspectRatio) {
            const qreal tan1 = viewBoxRect.height() / viewBoxRect.width();
            const qreal tan2 = elementBounds.height() / elementBounds.width();

            const qreal uniformScale =
                (p.mode == Qt::KeepAspectRatioByExpanding) ^ (tan1 > tan2) ?
                elementBounds.height() / viewBoxRect.height() :
                elementBounds.width() / viewBoxRect.width();


            viewBoxTransform =
                QTransform::fromTranslate(-viewBoxRect.x(), -viewBoxRect.y()) *
                QTransform::fromScale(uniformScale, uniformScale);

            const QPointF viewBoxAnchor = viewBoxTransform.map(p.rectAnchorPoint(viewBoxRect));
            const QPointF elementAnchor = p.rectAnchorPoint(elementBounds);
            const QPointF offset = elementAnchor - viewBoxAnchor;

            viewBoxTransform = viewBoxTransform * QTransform::fromTranslate(offset.x(), offset.y());
        }

        if (p.defer && e.tagName() == "image") {
            // TODO:
        }
    }

    *_viewRect = viewBoxRect;
    *_viewTransform = viewBoxTransform;

    return result;
}

qreal SvgUtil::parseUnit(SvgGraphicsContext *gc, const QString &unit, bool horiz, bool vert, const QRectF &bbox)
{
    if (unit.isEmpty())
        return 0.0;
    QByteArray unitLatin1 = unit.toLatin1();
    // TODO : percentage?
    const char *start = unitLatin1.data();
    if (!start) {
        return 0.0;
    }
    qreal value = 0.0;
    const char *end = parseNumber(start, value);

    if (int(end - start) < unit.length()) {
        if (unit.right(2) == "px")
            value = SvgUtil::fromUserSpace(value);
        else if (unit.right(2) == "pt")
            value = ptToPx(gc, value);
        else if (unit.right(2) == "cm")
            value = ptToPx(gc, CM_TO_POINT(value));
        else if (unit.right(2) == "pc")
            value = ptToPx(gc, PI_TO_POINT(value));
        else if (unit.right(2) == "mm")
            value = ptToPx(gc, MM_TO_POINT(value));
        else if (unit.right(2) == "in")
            value = ptToPx(gc, INCH_TO_POINT(value));
        else if (unit.right(2) == "em")
            value = ptToPx(gc, value * gc->font.pointSize());
        else if (unit.right(2) == "ex") {
            QFontMetrics metrics(gc->font);
            value = ptToPx(gc, value * metrics.xHeight());
        } else if (unit.right(1) == "%") {
            if (horiz && vert)
                value = (value / 100.0) * (sqrt(pow(bbox.width(), 2) + pow(bbox.height(), 2)) / sqrt(2.0));
            else if (horiz)
                value = (value / 100.0) * bbox.width();
            else if (vert)
                value = (value / 100.0) * bbox.height();
        }
    } else {
        value = SvgUtil::fromUserSpace(value);
    }
    /*else
    {
        if( m_gc.top() )
        {
            if( horiz && vert )
                value *= sqrt( pow( m_gc.top()->matrix.m11(), 2 ) + pow( m_gc.top()->matrix.m22(), 2 ) ) / sqrt( 2.0 );
            else if( horiz )
                value /= m_gc.top()->matrix.m11();
            else if( vert )
                value /= m_gc.top()->matrix.m22();
        }
    }*/
    //value *= 90.0 / DPI;

    return value;
}

qreal SvgUtil::parseUnitX(SvgGraphicsContext *gc, const QString &unit)
{
    if (gc->forcePercentage) {
        return SvgUtil::fromPercentage(unit) * gc->currentBoundingBox.width();
    } else {
        return SvgUtil::parseUnit(gc, unit, true, false, gc->currentBoundingBox);
    }
}

qreal SvgUtil::parseUnitY(SvgGraphicsContext *gc, const QString &unit)
{
    if (gc->forcePercentage) {
        return SvgUtil::fromPercentage(unit) * gc->currentBoundingBox.height();
    } else {
        return SvgUtil::parseUnit(gc, unit, false, true, gc->currentBoundingBox);
    }
}

qreal SvgUtil::parseUnitXY(SvgGraphicsContext *gc, const QString &unit)
{
    if (gc->forcePercentage) {
        const qreal value = SvgUtil::fromPercentage(unit);
        return value * sqrt(pow(gc->currentBoundingBox.width(), 2) + pow(gc->currentBoundingBox.height(), 2)) / sqrt(2.0);
    } else {
        return SvgUtil::parseUnit(gc, unit, true, true, gc->currentBoundingBox);
    }
}

const char * SvgUtil::parseNumber(const char *ptr, qreal &number)
{
    int integer, exponent;
    qreal decimal, frac;
    int sign, expsign;

    exponent = 0;
    integer = 0;
    frac = 1.0;
    decimal = 0;
    sign = 1;
    expsign = 1;

    // read the sign
    if (*ptr == '+') {
        ptr++;
    } else if (*ptr == '-') {
        ptr++;
        sign = -1;
    }

    // read the integer part
    while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
        integer = (integer * 10) + *(ptr++) - '0';
    if (*ptr == '.') { // read the decimals
        ptr++;
        while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
            decimal += (*(ptr++) - '0') * (frac *= 0.1);
    }

    if (*ptr == 'e' || *ptr == 'E') { // read the exponent part
        ptr++;

        // read the sign of the exponent
        if (*ptr == '+') {
            ptr++;
        } else if (*ptr == '-') {
            ptr++;
            expsign = -1;
        }

        exponent = 0;
        while (*ptr != '\0' && *ptr >= '0' && *ptr <= '9') {
            exponent *= 10;
            exponent += *ptr - '0';
            ptr++;
        }
    }
    number = integer + decimal;
    number *= sign * pow((double)10, double(expsign * exponent));

    return ptr;
}

SvgUtil::PreserveAspectRatioParser::PreserveAspectRatioParser(const QString &str)
{
    QRegExp rexp("(defer)?\\s+(none|(x(Min|Max|Mid)Y(Min|Max|Mid)))\\s+(meet|slice)?", Qt::CaseInsensitive);
    int index = rexp.indexIn(str.toLower());

    if (index >= 0) {
        if (rexp.cap(1) == "defer") {
            defer = true;
        }

        if (rexp.cap(2) != "none") {
            xAlignment = alignmentFromString(rexp.cap(4));
            yAlignment = alignmentFromString(rexp.cap(5));
            mode = rexp.cap(6) == "slice" ?
                Qt::KeepAspectRatioByExpanding : Qt::KeepAspectRatio;
        }
    }
}

QPointF SvgUtil::PreserveAspectRatioParser::rectAnchorPoint(const QRectF &rc) const
{
    return QPointF(alignedValue(rc.x(), rc.x() + rc.width(), xAlignment),
                   alignedValue(rc.y(), rc.y() + rc.height(), yAlignment));
}

SvgUtil::PreserveAspectRatioParser::Alignment SvgUtil::PreserveAspectRatioParser::alignmentFromString(const QString &str) {
    return
        str == "max" ? Max :
        str == "mid" ? Middle : Min;
}

qreal SvgUtil::PreserveAspectRatioParser::alignedValue(qreal min, qreal max, SvgUtil::PreserveAspectRatioParser::Alignment alignment)
{
    qreal result = min;

    switch (alignment) {
    case Min:
        result = min;
        break;
    case Middle:
        result = 0.5 * (min + max);
        break;
    case Max:
        result = max;
        break;
    }

    return result;
}
