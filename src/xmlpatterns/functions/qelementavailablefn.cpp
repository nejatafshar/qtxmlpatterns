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

#include "qqnameconstructor_p.h"

#include "qelementavailablefn_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

ElementAvailableFN::ElementAvailableFN() : m_xsltInstructions(allXSLTInstructions())
{
}

QSet<QString> ElementAvailableFN::allXSLTInstructions()
{
    enum
    {
        StringSetSize = 27
    };

    QSet<QString> retval;
    retval.reserve(StringSetSize);

    /* Alphabetically. */
    retval.insert(QLatin1StringView("analyze-string"));
    retval.insert(QLatin1StringView("apply-imports"));
    retval.insert(QLatin1StringView("apply-templates"));
    retval.insert(QLatin1StringView("attribute"));
    retval.insert(QLatin1StringView("attribute-set"));
    retval.insert(QLatin1StringView("call-template"));
    retval.insert(QLatin1StringView("character-map"));
    retval.insert(QLatin1StringView("choose"));
    retval.insert(QLatin1StringView("comment"));
    retval.insert(QLatin1StringView("copy"));
    retval.insert(QLatin1StringView("copy-of"));
    retval.insert(QLatin1StringView("document"));
    retval.insert(QLatin1StringView("element"));
    retval.insert(QLatin1StringView("fallback"));
    retval.insert(QLatin1StringView("for-each"));
    retval.insert(QLatin1StringView("for-each-group"));
    retval.insert(QLatin1StringView("if"));
    retval.insert(QLatin1StringView("message"));
    retval.insert(QLatin1StringView("namespace"));
    retval.insert(QLatin1StringView("next-match"));
    retval.insert(QLatin1StringView("number"));
    retval.insert(QLatin1StringView("perform-sort"));
    retval.insert(QLatin1StringView("processing-instruction"));
    retval.insert(QLatin1StringView("result-document"));
    retval.insert(QLatin1StringView("sequence"));
    retval.insert(QLatin1StringView("text"));
    retval.insert(QLatin1StringView("variable"));

    Q_ASSERT(retval.count() == StringSetSize);
    return retval;
}

bool ElementAvailableFN::evaluateEBV(const DynamicContext::Ptr &context) const
{
    const Item arg(m_operands.first()->evaluateSingleton(context));
    const QString stringName(arg.stringValue());

    const QXmlName elementName(QNameConstructor::expandQName<DynamicContext::Ptr,
                                                             ReportContext::XTDE1440,
                                                             ReportContext::XTDE1440>(stringName,
                                                                                      context,
                                                                                      staticNamespaces(),
                                                                                      this,
                                                                                      false));

    if(elementName.namespaceURI() != StandardNamespaces::xslt)
        return false;

    QString prefix;
    QString localName;
    XPathHelper::splitQName(stringName, prefix, localName);

    return m_xsltInstructions.contains(localName);
}

QT_END_NAMESPACE
