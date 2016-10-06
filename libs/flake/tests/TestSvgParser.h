/*
 *  Copyright (c) 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TESTSVGPARSER_H
#define TESTSVGPARSER_H

#include <QtTest>

class TestSvgParser : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testUnitPx();
    void testUnitPxResolution();
    void testUnitPt();
    void testUnitIn();
    void testUnitPercentInitial();
    void testScalingViewport();
    void testScalingViewportKeepMeet1();
    void testScalingViewportKeepMeet2();
    void testScalingViewportKeepMeetAlign();
    void testScalingViewportKeepSlice1();
    void testScalingViewportKeepSlice2();
    void testScalingViewportResolution();
    void testScalingViewportPercentInternal();
    void testParsePreserveAspectRatio();

};

#endif // TESTSVGPARSER_H
