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

#include <QtDebug>

#include <private/qacceltreeresourceloader_p.h>
#include <private/qnetworkaccessdelegator_p.h>

#include "Global.h"
#include "TestBaseLine.h"
#include "TestGroup.h"

#include "XSLTTestSuiteHandler.h"

using namespace QPatternistSDK;

static QNetworkAccessManager *s_networkAccessManager = 0;

static void cleanupNetworkAccessManager()
{
    delete s_networkAccessManager;
    s_networkAccessManager = 0;

}
static QNetworkAccessManager *networkAccessManager()
{
    if (!s_networkAccessManager) {
        s_networkAccessManager = new QNetworkAccessManager;
        qAddPostRoutine(cleanupNetworkAccessManager);
    }
    return s_networkAccessManager;
}

XSLTTestSuiteHandler::XSLTTestSuiteHandler(const QUrl &catalogFile) : m_ts(0)
                                                                    , m_tc(0)
                                                                    , m_baseLine(0)
                                                                    , m_catalogFile(catalogFile)
                                                                    , m_removeTestcase(false)
{
    const QPatternist::NetworkAccessDelegator::Ptr networkDelegator(new QPatternist::NetworkAccessDelegator(networkAccessManager(), networkAccessManager()));

    m_resourceLoader = QPatternist::ResourceLoader::Ptr(new QPatternist::AccelTreeResourceLoader(Global::namePool(),
                                                                                                 networkDelegator));
    Q_ASSERT(!m_catalogFile.isRelative());
}

bool XSLTTestSuiteHandler::startElement(QStringView namespaceURI, QStringView localName,
                                        QStringView  /*qName*/, const QXmlStreamAttributes &atts)
{
    if(namespaceURI != Global::xsltsCatalogNS)
        return true;

    /* The elements are handled roughly in the order of highest occurrence in the catalog file. */
    if(localName == QLatin1StringView("testcase"))
    {
        /* We pass m_ts temporarily, and change it later. */
        m_tc = new XQTSTestCase(TestCase::Standard, 0, QXmlQuery::XSLT20);

        m_currentQueryPath =
                m_queryOffset.resolved(QUrl(atts.value(QLatin1StringView("FilePath")).toString()));
        m_currentBaselinePath =
                m_baselineOffset.resolved(QUrl(atts.value(QLatin1StringView("FilePath")).toString()));
    }
    else if(localName == QLatin1StringView("stylesheet"))
        m_tc->setQueryPath(
                m_currentQueryPath.resolved(atts.value(QLatin1StringView("file")).toString()));
    else if(localName == QLatin1StringView("error"))
    {
        m_baseLine = new TestBaseLine(TestBaseLine::ExpectedError);
        m_baseLine->setDetails(atts.value(QLatin1StringView("error-id")).toString());
        m_tc->addBaseLine(m_baseLine);
    }
    else if(localName == QLatin1StringView("testcases"))
    {
        m_ts = new TestSuite();
        m_ts->setVersion(atts.value(QLatin1StringView("testSuiteVersion")).toString());

        m_queryOffset =
                m_catalogFile.resolved(atts.value(QLatin1StringView("InputOffsetPath")).toString());
        m_baselineOffset =
                m_catalogFile.resolved(atts.value(QLatin1StringView("ResultOffsetPath")).toString());
        m_sourceOffset =
                m_catalogFile.resolved(atts.value(QLatin1StringView("InputOffsetPath")).toString());
    }
    else if(localName == QLatin1StringView("source-document"))
    {
        if(atts.value(QLatin1StringView("role")) == QLatin1StringView("principal"))
            m_tc->setContextItemSource(
                    m_sourceOffset.resolved(QUrl(atts.value(QLatin1StringView("file")).toString())));
    }
    else if(localName == QLatin1StringView("result-document"))
    {
        m_baseLine = new TestBaseLine(
                TestBaseLine::identifierFromString(atts.value(QLatin1StringView("type")).toString()));
        m_baseLine->setDetails(
                m_currentBaselinePath.resolved(atts.value(QLatin1StringView("file")).toString())
                        .toString());
        m_tc->addBaseLine(m_baseLine);
    }
    else if(localName == QLatin1StringView("discretionary-feature"))
    {
        const QString feature(atts.value(QLatin1StringView("name")).toString());

        m_removeTestcase = feature == QLatin1StringView("schema_aware") ||
                           feature == QLatin1StringView("namespace_axis") ||
                           feature == QLatin1StringView("disabling_output_escaping") ||
                           feature == QLatin1StringView("XML_1.1");
    }
    else if(localName == QLatin1StringView("discretionary-choice"))
    {
        m_baseLine = new TestBaseLine(TestBaseLine::ExpectedError);
        m_baseLine->setDetails(atts.value(QLatin1StringView("name")).toString());
        m_tc->addBaseLine(m_baseLine);
        const QString feature(atts.value(QLatin1StringView("name")).toString());

        m_removeTestcase = feature == QLatin1StringView("schema_aware") ||
                           feature == QLatin1StringView("namespace_axis") ||
                           feature == QLatin1StringView("disabling_output_escaping") ||
                           feature == QLatin1StringView("XML_1.1");
    }
    else if(localName == QLatin1StringView("entry-named-template"))
    {
        const QString name(atts.value(QLatin1StringView("qname")).toString());

        if(!name.contains(QLatin1Char(':')))
        {
            // TODO do namespace processing
            const QXmlName complete(Global::namePool()->allocateQName(QString(), name));

            m_tc->setInitialTemplateName(complete);
        }
    }

    return true;
}

TestGroup *XSLTTestSuiteHandler::containerFor(const QString &name)
{
    TestGroup *& c = m_containers[name];

    if(!c)
    {
        c = new TestGroup(m_ts);
        c->setTitle(name);
        Q_ASSERT(c);
        m_ts->appendChild(c);
    }

    return c;
}

bool XSLTTestSuiteHandler::endElement(QStringView namespaceURI,
                                      QStringView localName,
                                      QStringView /*qName*/)
{
    if(namespaceURI != Global::xsltsCatalogNS)
        return true;

    /* The elements are handled roughly in the order of highest occurrence in the catalog file. */
    if(localName == QLatin1StringView("description"))
    {
        if(m_tc)
        {
            /* We're inside a <testcase>, so the <description> belongs
             * to the testcase. */
            m_tc->setDescription(m_ch.simplified());
        }
    }
    else if(localName == QLatin1StringView("testcase"))
    {
        Q_ASSERT(m_tc->baseLines().count() >= 1);
        Q_ASSERT(m_resourceLoader);
        // TODO can this be removed?
        m_tc->setExternalVariableLoader(QPatternist::ExternalVariableLoader::Ptr
                                                (new ExternalSourceLoader(m_tcSourceInputs,
                                                                          m_resourceLoader)));
        m_tcSourceInputs.clear();

        if(!m_removeTestcase)
        {
            /*
            TestContainer *const container = containerFor(m_currentCategory);
            delete m_tc;
            container->removeLast();
            */
            TestContainer *const container = containerFor(m_currentCategory);
            m_tc->setParent(container);
            Q_ASSERT(m_tc);
            container->appendChild(m_tc);
        }

        m_tc = 0;
        m_removeTestcase = false;
    }
    else if(localName == QLatin1StringView("name"))
        m_tc->setName(m_ch);
    else if(localName == QLatin1StringView("creator"))
        m_tc->setCreator(m_ch);
    else if(localName == QLatin1StringView("contextItem"))
        m_contextItemSource = m_ch;
    else if(localName == QLatin1StringView("category"))
        m_currentCategory = m_ch;

    return true;
}

bool XSLTTestSuiteHandler::characters(QStringView ch)
{
    m_ch = ch.toString();
    return true;
}

TestSuite *XSLTTestSuiteHandler::testSuite() const
{
    return m_ts;
}

// vim: et:ts=4:sw=4:sts=4

