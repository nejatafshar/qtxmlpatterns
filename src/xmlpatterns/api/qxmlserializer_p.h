/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtXmlPatterns module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QXMLSERIALIZER_P_H
#define QXMLSERIALIZER_P_H

#include <QtCore/QIODevice>
#include <QtCore/QStack>
#include <qxmlquery.h>
#include <qxmlnamepool.h>
#include <qxmlserializer.h>

#include <qnamepool_p.h>
#include <qabstractxmlreceiver_p.h>

QT_BEGIN_NAMESPACE

class QXmlSerializerPrivate : public QAbstractXmlReceiverPrivate
{
public:
    QXmlSerializerPrivate(const QXmlQuery &q,
                          QIODevice *outputDevice);

    QStack<QPair<QXmlName, bool> >      hasClosedElement;
    bool                                isPreviousAtomic;
    QXmlSerializer::State               state;
    const QPatternist::NamePool::Ptr    np;

    /**
     * This member worries me a bit. We never use it but nevertheless
     * it is pushed and pops linear to startElement() and endElement().
     * An optimization would be to at least merge it with hasClosedElement,
     * but even better to push it on demand. That is, namespaceBinding()
     * pushes it up to the tree depth first when it is needed.
     */
    QStack<QVector<QXmlName> >          namespaces;

    QIODevice *                         device;
    QString                             encoding;
    /**
     * Name cache. Since encoding QStrings are rather expensive
     * operations to do, and we on top of that would have to do
     * it each time a name appears, we here map names to their
     * encoded equivalents.
     *
     * This means that when writing out large documents, the serialization
     * of names after a while is reduced to a hash lookup and passing an
     * existing byte array.
     *
     * We use QXmlName::Code as key as opposed to merely QName, because the
     * prefix is of significance.
     */
    QHash<QXmlName::Code, QByteArray>   nameCache;
    const QXmlQuery                     query;

    inline void write(const char c);

private:
    enum Constants
    {
        EstimatedTreeDepth = 10,

        /**
         * We use a high count to avoid rehashing. We can afford it since we
         * only allocate one hash for this.
         */
        EstimatedNameCount = 60
    };
};

void QXmlSerializerPrivate::write(const char c)
{
    device->putChar(c);
}
QT_END_NAMESPACE

#endif
