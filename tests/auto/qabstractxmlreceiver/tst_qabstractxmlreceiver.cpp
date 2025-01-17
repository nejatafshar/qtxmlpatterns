/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QtTest/QtTest>

#include <QtXmlPatterns/QAbstractXmlReceiver>
#include <QtXmlPatterns/QXmlName>

#include "TestAbstractXmlReceiver.h"

/*!
 \class tst_QAbstractXmlReceiver
 \internal
 \since 4.4
 \brief Tests class QXmlNamePool.

 */
class tst_QAbstractXmlReceiver : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void attributeQXmlNameStringRef() const;
    void charactersStringRef() const;
};

void tst_QAbstractXmlReceiver::attributeQXmlNameStringRef() const
{
    TestAbstractXmlReceiver receiver;

    const QString input(QLatin1StringView("input"));

    QCOMPARE(receiver.receivedFromAttribute, QString());
    receiver.attribute(QXmlName(), QStringView(input));
    QCOMPARE(receiver.receivedFromAttribute, QString::fromLatin1("input"));
}

void tst_QAbstractXmlReceiver::charactersStringRef() const
{
    TestAbstractXmlReceiver receiver;

    const QString input(QLatin1StringView("input"));

    QCOMPARE(receiver.receivedFromCharacters, QString());
    receiver.characters(QStringView(input));
    QCOMPARE(receiver.receivedFromCharacters, QString::fromLatin1("input"));
}

QTEST_MAIN(tst_QAbstractXmlReceiver)

#include "tst_qabstractxmlreceiver.moc"

// vim: et:ts=4:sw=4:sts=4
