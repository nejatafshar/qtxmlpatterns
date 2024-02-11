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

#include <QtDebug>

#include "private/qxmlutils_p.h"
#include "qxpathhelper_p.h"

#include "qnamepool_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

NamePool::NamePool()
{
    m_localNameMapping  .reserve(DefaultLocalNameCapacity   + StandardLocalNameCount);
    m_localNames        .reserve(DefaultLocalNameCapacity   + StandardLocalNameCount);
    m_namespaceMapping  .reserve(DefaultURICapacity         + StandardNamespaceCount);
    m_namespaces        .reserve(DefaultURICapacity         + StandardNamespaceCount);
    m_prefixes          .reserve(DefaultPrefixCapacity      + StandardPrefixCount);
    m_prefixMapping     .reserve(DefaultPrefixCapacity      + StandardPrefixCount);

    /* Namespaces. */
    {
        unlockedAllocateNamespace(QString());
        unlockedAllocateNamespace(QLatin1StringView("http://www.w3.org/2005/xpath-functions"));
        unlockedAllocateNamespace(QLatin1StringView("http://www.w3.org/2005/xquery-local-functions"));
        unlockedAllocateNamespace(QLatin1StringView("http://www.w3.org/XML/1998/namespace"));
        unlockedAllocateNamespace(QLatin1StringView("http://www.w3.org/2000/xmlns/"));
        unlockedAllocateNamespace(QLatin1StringView("http://www.w3.org/2001/XMLSchema"));
        unlockedAllocateNamespace(QLatin1StringView("http://www.w3.org/2001/XMLSchema-instance"));
        unlockedAllocateNamespace(QLatin1StringView("http://www.w3.org/1999/XSL/Transform"));

        /* For UndeclarePrefix, StopNamespaceInheritance and InternalXSLT. We use two
         * arbitrary strings that aren't used. For instance, if we just used an
         * empty QString, unlockedAllocateNamespace() would assign it
         * StandardNamespaces::empty. However, it's important that the string
         * is an invalid namespace, since otherwise user strings would get
         * assigned the same IDs. */
        unlockedAllocateNamespace(QLatin1StringView("  |  1  "));
        unlockedAllocateNamespace(QLatin1StringView("  |  2  "));
        unlockedAllocateNamespace(QLatin1StringView("  |  InternalXSLT"));

        Q_ASSERT_X(m_namespaces.count() == StandardNamespaceCount, Q_FUNC_INFO,
                   qPrintable(QString::fromLatin1("Expected is %1, actual is %2.").arg(StandardNamespaceCount).arg(m_namespaces.count())));
    }

    /* Prefixes. */
    {
        unlockedAllocatePrefix(QString());
        unlockedAllocatePrefix(QLatin1StringView("fn"));
        unlockedAllocatePrefix(QLatin1StringView("local"));
        unlockedAllocatePrefix(QLatin1StringView("xml"));
        unlockedAllocatePrefix(QLatin1StringView("xmlns"));
        unlockedAllocatePrefix(QLatin1StringView("xs"));
        unlockedAllocatePrefix(QLatin1StringView("xsi"));
        unlockedAllocatePrefix(QLatin1StringView("ns0"));
        unlockedAllocatePrefix(QLatin1StringView("|||")); /* Invalid NCName, for StopNamespaceInheritance. */

        Q_ASSERT_X(m_prefixes.count() == StandardPrefixCount, Q_FUNC_INFO,
                   qPrintable(QString::fromLatin1("Expected is %1, actual is %2.").arg(StandardPrefixCount).arg(m_prefixes.count())));
    }

    /* Local names. */
    {
        /* Same order as the enum in StandardLocalNames. That is, alphabetically. */
        unlockedAllocateLocalName(QLatin1StringView("abs"));
        unlockedAllocateLocalName(QLatin1StringView("adjust-dateTime-to-timezone"));
        unlockedAllocateLocalName(QLatin1StringView("adjust-date-to-timezone"));
        unlockedAllocateLocalName(QLatin1StringView("adjust-time-to-timezone"));
        unlockedAllocateLocalName(QLatin1StringView("all"));
        unlockedAllocateLocalName(QLatin1StringView("arity"));
        unlockedAllocateLocalName(QLatin1StringView("avg"));
        unlockedAllocateLocalName(QLatin1StringView("base"));
        unlockedAllocateLocalName(QLatin1StringView("base-uri"));
        unlockedAllocateLocalName(QLatin1StringView("boolean"));
        unlockedAllocateLocalName(QLatin1StringView("ceiling"));
        unlockedAllocateLocalName(QLatin1StringView("codepoint-equal"));
        unlockedAllocateLocalName(QLatin1StringView("codepoints-to-string"));
        unlockedAllocateLocalName(QLatin1StringView("collection"));
        unlockedAllocateLocalName(QLatin1StringView("compare"));
        unlockedAllocateLocalName(QLatin1StringView("concat"));
        unlockedAllocateLocalName(QLatin1StringView("contains"));
        unlockedAllocateLocalName(QLatin1StringView("count"));
        unlockedAllocateLocalName(QLatin1StringView("current"));
        unlockedAllocateLocalName(QLatin1StringView("current-date"));
        unlockedAllocateLocalName(QLatin1StringView("current-dateTime"));
        unlockedAllocateLocalName(QLatin1StringView("current-time"));
        unlockedAllocateLocalName(QLatin1StringView("data"));
        unlockedAllocateLocalName(QLatin1StringView("dateTime"));
        unlockedAllocateLocalName(QLatin1StringView("day-from-date"));
        unlockedAllocateLocalName(QLatin1StringView("day-from-dateTime"));
        unlockedAllocateLocalName(QLatin1StringView("days-from-duration"));
        unlockedAllocateLocalName(QLatin1StringView("deep-equal"));
        unlockedAllocateLocalName(QLatin1StringView("default"));
        unlockedAllocateLocalName(QLatin1StringView("default-collation"));
        unlockedAllocateLocalName(QLatin1StringView("distinct-values"));
        unlockedAllocateLocalName(QLatin1StringView("doc"));
        unlockedAllocateLocalName(QLatin1StringView("doc-available"));
        unlockedAllocateLocalName(QLatin1StringView("document"));
        unlockedAllocateLocalName(QLatin1StringView("document-uri"));
        unlockedAllocateLocalName(QLatin1StringView("element-available"));
        unlockedAllocateLocalName(QLatin1StringView("empty"));
        unlockedAllocateLocalName(QLatin1StringView("encode-for-uri"));
        unlockedAllocateLocalName(QLatin1StringView("ends-with"));
        unlockedAllocateLocalName(QLatin1StringView("error"));
        unlockedAllocateLocalName(QLatin1StringView("escape-html-uri"));
        unlockedAllocateLocalName(QLatin1StringView("exactly-one"));
        unlockedAllocateLocalName(QLatin1StringView("exists"));
        unlockedAllocateLocalName(QLatin1StringView("false"));
        unlockedAllocateLocalName(QLatin1StringView("floor"));
        unlockedAllocateLocalName(QLatin1StringView("function-available"));
        unlockedAllocateLocalName(QLatin1StringView("function-name"));
        unlockedAllocateLocalName(QLatin1StringView("generate-id"));
        unlockedAllocateLocalName(QLatin1StringView("generic-string-join"));
        unlockedAllocateLocalName(QLatin1StringView("hours-from-dateTime"));
        unlockedAllocateLocalName(QLatin1StringView("hours-from-duration"));
        unlockedAllocateLocalName(QLatin1StringView("hours-from-time"));
        unlockedAllocateLocalName(QLatin1StringView("id"));
        unlockedAllocateLocalName(QLatin1StringView("idref"));
        unlockedAllocateLocalName(QLatin1StringView("implicit-timezone"));
        unlockedAllocateLocalName(QLatin1StringView("index-of"));
        unlockedAllocateLocalName(QLatin1StringView("in-scope-prefixes"));
        unlockedAllocateLocalName(QLatin1StringView("insert-before"));
        unlockedAllocateLocalName(QLatin1StringView("iri-to-uri"));
        unlockedAllocateLocalName(QLatin1StringView("is-schema-aware"));
        unlockedAllocateLocalName(QLatin1StringView("key"));
        unlockedAllocateLocalName(QLatin1StringView("lang"));
        unlockedAllocateLocalName(QLatin1StringView("last"));
        unlockedAllocateLocalName(QLatin1StringView("local-name"));
        unlockedAllocateLocalName(QLatin1StringView("local-name-from-QName"));
        unlockedAllocateLocalName(QLatin1StringView("lower-case"));
        unlockedAllocateLocalName(QLatin1StringView("matches"));
        unlockedAllocateLocalName(QLatin1StringView("max"));
        unlockedAllocateLocalName(QLatin1StringView("min"));
        unlockedAllocateLocalName(QLatin1StringView("minutes-from-dateTime"));
        unlockedAllocateLocalName(QLatin1StringView("minutes-from-duration"));
        unlockedAllocateLocalName(QLatin1StringView("minutes-from-time"));
        unlockedAllocateLocalName(QLatin1StringView("month-from-date"));
        unlockedAllocateLocalName(QLatin1StringView("month-from-dateTime"));
        unlockedAllocateLocalName(QLatin1StringView("months-from-duration"));
        unlockedAllocateLocalName(QLatin1StringView("name"));
        unlockedAllocateLocalName(QLatin1StringView("namespace-uri"));
        unlockedAllocateLocalName(QLatin1StringView("namespace-uri-for-prefix"));
        unlockedAllocateLocalName(QLatin1StringView("namespace-uri-from-QName"));
        unlockedAllocateLocalName(QLatin1StringView("nilled"));
        unlockedAllocateLocalName(QLatin1StringView("node-name"));
        unlockedAllocateLocalName(QLatin1StringView("normalize-space"));
        unlockedAllocateLocalName(QLatin1StringView("normalize-unicode"));
        unlockedAllocateLocalName(QLatin1StringView("not"));
        unlockedAllocateLocalName(QLatin1StringView("number"));
        unlockedAllocateLocalName(QLatin1StringView("one-or-more"));
        unlockedAllocateLocalName(QLatin1StringView("position"));
        unlockedAllocateLocalName(QLatin1StringView("prefix-from-QName"));
        unlockedAllocateLocalName(QLatin1StringView("product-name"));
        unlockedAllocateLocalName(QLatin1StringView("product-version"));
        unlockedAllocateLocalName(QLatin1StringView("property-name"));
        unlockedAllocateLocalName(QLatin1StringView("QName"));
        unlockedAllocateLocalName(QLatin1StringView("remove"));
        unlockedAllocateLocalName(QLatin1StringView("replace"));
        unlockedAllocateLocalName(QLatin1StringView("resolve-QName"));
        unlockedAllocateLocalName(QLatin1StringView("resolve-uri"));
        unlockedAllocateLocalName(QLatin1StringView("reverse"));
        unlockedAllocateLocalName(QLatin1StringView("root"));
        unlockedAllocateLocalName(QLatin1StringView("round"));
        unlockedAllocateLocalName(QLatin1StringView("round-half-to-even"));
        unlockedAllocateLocalName(QLatin1StringView("seconds-from-dateTime"));
        unlockedAllocateLocalName(QLatin1StringView("seconds-from-duration"));
        unlockedAllocateLocalName(QLatin1StringView("seconds-from-time"));
        unlockedAllocateLocalName(QLatin1StringView("sourceValue"));
        unlockedAllocateLocalName(QLatin1StringView("starts-with"));
        unlockedAllocateLocalName(QLatin1StringView("static-base-uri"));
        unlockedAllocateLocalName(QLatin1StringView("string"));
        unlockedAllocateLocalName(QLatin1StringView("string-join"));
        unlockedAllocateLocalName(QLatin1StringView("string-length"));
        unlockedAllocateLocalName(QLatin1StringView("string-to-codepoints"));
        unlockedAllocateLocalName(QLatin1StringView("subsequence"));
        unlockedAllocateLocalName(QLatin1StringView("substring"));
        unlockedAllocateLocalName(QLatin1StringView("substring-after"));
        unlockedAllocateLocalName(QLatin1StringView("substring-before"));
        unlockedAllocateLocalName(QLatin1StringView("sum"));
        unlockedAllocateLocalName(QLatin1StringView("supports-backwards-compatibility"));
        unlockedAllocateLocalName(QLatin1StringView("supports-serialization"));
        unlockedAllocateLocalName(QLatin1StringView("system-property"));
        unlockedAllocateLocalName(QLatin1StringView("timezone-from-date"));
        unlockedAllocateLocalName(QLatin1StringView("timezone-from-dateTime"));
        unlockedAllocateLocalName(QLatin1StringView("timezone-from-time"));
        unlockedAllocateLocalName(QLatin1StringView("tokenize"));
        unlockedAllocateLocalName(QLatin1StringView("trace"));
        unlockedAllocateLocalName(QLatin1StringView("translate"));
        unlockedAllocateLocalName(QLatin1StringView("true"));
        unlockedAllocateLocalName(QLatin1StringView("type-available"));
        unlockedAllocateLocalName(QLatin1StringView("unordered"));
        unlockedAllocateLocalName(QLatin1StringView("unparsed-entity-public-id"));
        unlockedAllocateLocalName(QLatin1StringView("unparsed-entity-uri"));
        unlockedAllocateLocalName(QLatin1StringView("unparsed-text"));
        unlockedAllocateLocalName(QLatin1StringView("unparsed-text-available"));
        unlockedAllocateLocalName(QLatin1StringView("upper-case"));
        unlockedAllocateLocalName(QLatin1StringView("vendor"));
        unlockedAllocateLocalName(QLatin1StringView("vendor-url"));
        unlockedAllocateLocalName(QLatin1StringView("version"));
        unlockedAllocateLocalName(QLatin1StringView("xml"));
        unlockedAllocateLocalName(QLatin1StringView("xmlns"));
        unlockedAllocateLocalName(QLatin1StringView("year-from-date"));
        unlockedAllocateLocalName(QLatin1StringView("year-from-dateTime"));
        unlockedAllocateLocalName(QLatin1StringView("years-from-duration"));
        unlockedAllocateLocalName(QLatin1StringView("zero-or-one"));
        Q_ASSERT(m_localNames.count() == StandardLocalNameCount);
    }
}

QXmlName NamePool::allocateQName(const QString &uri, const QString &localName, const QString &prefix)
{
    QWriteLocker l(&lock);

    /*
    QString codepoints;
    for(int i = 0; i < localName.length(); ++i)
        codepoints.append(QString::number(localName.at(i).unicode()) + QLatin1Char(' '));

    pDebug() << Q_FUNC_INFO << localName << "codepoints:" << codepoints;
    */

    Q_ASSERT_X(QXmlUtils::isNCName(localName), Q_FUNC_INFO,
               qPrintable(QString::fromLatin1("'%1' is an invalid NCName.").arg(localName)));

    const QXmlName::NamespaceCode nsCode = unlockedAllocateNamespace(uri);
    const QXmlName::LocalNameCode localCode  = unlockedAllocateLocalName(localName);

    Q_ASSERT(prefix.isEmpty() || QXmlUtils::isNCName(prefix));
    const QXmlName::PrefixCode prefixCode = unlockedAllocatePrefix(prefix);

    return QXmlName(nsCode, localCode, prefixCode);
}

QXmlName NamePool::allocateBinding(const QString &prefix, const QString &uri)
{
    QWriteLocker l(&lock);

    Q_ASSERT_X(prefix.isEmpty() || QXmlUtils::isNCName(prefix), Q_FUNC_INFO,
               qPrintable(QString::fromLatin1("%1 is an invalid prefix.").arg(prefix)));
    const QXmlName::NamespaceCode nsCode = unlockedAllocateNamespace(uri);

    Q_ASSERT(prefix.isEmpty() || QXmlUtils::isNCName(prefix));
    const QXmlName::PrefixCode prefixCode = unlockedAllocatePrefix(prefix);

    return QXmlName(nsCode, StandardLocalNames::empty, prefixCode);
}

QXmlName::LocalNameCode NamePool::unlockedAllocateLocalName(const QString &ln)
{
    Q_ASSERT_X(QXmlUtils::isNCName(ln), Q_FUNC_INFO,
               qPrintable(QString::fromLatin1("Invalid local name: \"%1\"").arg(ln)));

    int indexInLocalNames = m_localNameMapping.value(ln, NoSuchValue);

    if(indexInLocalNames == NoSuchValue)
    {
        indexInLocalNames = m_localNames.count();
        m_localNames.append(ln);
        m_localNameMapping.insert(ln, indexInLocalNames);
    }

    return indexInLocalNames;
}

QXmlName::PrefixCode NamePool::unlockedAllocatePrefix(const QString &prefix)
{
    int indexInPrefixes = m_prefixMapping.value(prefix, NoSuchValue);

    if(indexInPrefixes == NoSuchValue)
    {
        indexInPrefixes = m_prefixes.count();
        m_prefixes.append(prefix);
        m_prefixMapping.insert(prefix, indexInPrefixes);
    }

    return indexInPrefixes;
}

QXmlName::NamespaceCode NamePool::unlockedAllocateNamespace(const QString &uri)
{
    int indexInURIs = m_namespaceMapping.value(uri, NoSuchValue);

    if(indexInURIs == NoSuchValue)
    {
        indexInURIs = m_namespaces.count();
        m_namespaces.append(uri);
        m_namespaceMapping.insert(uri, indexInURIs);
    }

    return indexInURIs;
}

const QString &NamePool::displayPrefix(const QXmlName::NamespaceCode nc) const
{
    switch(nc)
    {
        case StandardNamespaces::xmlns: return m_prefixes.at(StandardPrefixes::xmlns);
        case StandardNamespaces::local: return m_prefixes.at(StandardPrefixes::local);
        case StandardNamespaces::xs:    return m_prefixes.at(StandardPrefixes::xs);
        case StandardNamespaces::xml:   return m_prefixes.at(StandardPrefixes::xml);
        case StandardNamespaces::fn:    return m_prefixes.at(StandardPrefixes::fn);
        default:                        return m_prefixes.at(StandardPrefixes::empty);
    }
}

QString NamePool::displayName(const QXmlName qName) const
{
    QReadLocker l(&lock);

    if(qName.hasNamespace())
    {
        if(qName.namespaceURI() == StandardNamespaces::InternalXSLT)
            return QLatin1Char('#') + m_localNames.at(qName.localName());

        const QString &p = displayPrefix(qName.namespaceURI());

        if(p.isEmpty())
            return QLatin1Char('{') + m_namespaces.at(qName.namespaceURI()) + QLatin1Char('}') + toLexical(qName);
        else
            return p + QLatin1Char(':') + m_localNames.at(qName.localName());
    }
    else
        return m_localNames.at(qName.localName());
}

QString NamePool::toClarkName(const QXmlName &name) const
{
    if(name.isNull())
        return QLatin1StringView("QXmlName(null)");
    else
    {
        if(name.hasNamespace())
        {
            const QString ns(stringForNamespace(name.namespaceURI()));
            const QString p(stringForPrefix(name.prefix()));
            const QString l(stringForLocalName(name.localName()));

            return   QChar::fromLatin1('{')
                   + ns
                   + QChar::fromLatin1('}')
                   + (p.isEmpty() ? l : p + QChar::fromLatin1(':') + l);
        }
        else
            return stringForLocalName(name.localName());
    }
}

QXmlName NamePool::fromClarkName(const QString &clarkName)
{
    if(clarkName.isEmpty())
        return QXmlName();

    if(clarkName.at(0) == QLatin1Char('{'))
    {
        const int indexOfRight = clarkName.indexOf(QLatin1Char('}'));
        const QString qName(clarkName.right((clarkName.length() - indexOfRight) - 1));

        if(!XPathHelper::isQName(qName))
            return QXmlName();

        QString localName;
        QString prefix;

        XPathHelper::splitQName(qName, prefix, localName);

        return allocateQName(clarkName.mid(1, indexOfRight - 1),
                                         localName, prefix);
    }
    else
    {
        if(QXmlName::isNCName(clarkName))
            return allocateQName(QString(), clarkName);
        else
            return QXmlName();
    }
}
QT_END_NAMESPACE
