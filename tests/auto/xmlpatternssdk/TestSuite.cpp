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

#include <QFileInfo>
#include <QVariant>
#include <QtDebug>

#include "Global.h"
#include "TestSuiteHandler.h"
#include "TestSuiteResult.h"
#include "XmlParseHelper.h"
#include "XMLWriter.h"
#include "XSLTTestSuiteHandler.h"
#include "XSDTestSuiteHandler.h"
#include <private/qxmldebug_p.h>

#include "TestSuite.h"

using namespace QPatternistSDK;
using namespace QPatternist;

TestSuite::TestSuite()
{
}

QVariant TestSuite::data(const Qt::ItemDataRole role, int column) const
{
    if(role != Qt::DisplayRole)
        return QVariant();

    switch(column)
    {
        case 0:
            return title();
        case 1:
            return QString();
        default:
        {
            Q_ASSERT(false);
            return QString();
        }
    }
}

TestSuiteResult *TestSuite::runSuite()
{
    const QDate date(QDate::currentDate());
    TestResult::List result(execute(CompileAndRun, this));

    return new TestSuiteResult(version(), date, result);
}

TestSuite *TestSuite::openCatalog(const QUrl &catalogURI,
                                  QString &errorMsg,
                                  const bool useExclusionList,
                                  SuiteType suiteType)
{
    pDebug() << "Opening catalog:" << catalogURI.toString();
    QFile ts(catalogURI.toLocalFile());
    Q_ASSERT(catalogURI.isValid());

    if(!ts.exists())
    {
        errorMsg = QString::fromLatin1("The test suite catalog \"%1\" could not be found.\n")
                                       .arg(ts.fileName());
        return 0;
    }

    const QFileInfo info(ts);

    if(!info.isReadable())
    {
        errorMsg = QString::fromLatin1("Cannot read the test suite catalog.\n");
        return 0;
    }
    else if(!info.isFile())
    {
        errorMsg = QString::fromLatin1("The specified test suite catalog \"%1\" is not a file. "
                                       "The test suite catalog must be a file, it cannot be "
                                       "a directory, for example.\n")
                                       .arg(ts.fileName());
        return 0;
    }
    else if(!ts.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        errorMsg = QString::fromLatin1("Failed to open the test suite catalog, \"%1\".\n")
                                       .arg(ts.fileName());
        return 0;
    }

    return openCatalog(&ts, errorMsg, catalogURI, useExclusionList, suiteType);
}

TestSuite *TestSuite::openCatalog(QIODevice *input,
                                  QString &errorMsg,
                                  const QUrl &fileName,
                                  const bool useExclusionList,
                                  SuiteType suiteType)
{
    Q_ASSERT(input);

    typedef QPatternist::AutoPtr<XmlParseHelper> HandlerPtr;

    HandlerPtr loader;

    switch (suiteType) {
        case XQuerySuite: loader = HandlerPtr(new TestSuiteHandler(fileName, useExclusionList)); break;
        case XsltSuite: loader = HandlerPtr(new XSLTTestSuiteHandler(fileName)); break;
        case XsdSuite: loader = HandlerPtr(new XSDTestSuiteHandler(fileName)); break;
        default: Q_ASSERT(false); break;
    }

    if (!loader.data()->parse(input)) {
        errorMsg = QString::fromLatin1("Couldn't parse %1").arg(fileName.toString());
        return 0;
    }

    TestSuite *suite = 0;
    switch (suiteType) {
        case XQuerySuite: suite = static_cast<TestSuiteHandler *>(loader.data())->testSuite(); break;
        case XsltSuite: suite = static_cast<XSLTTestSuiteHandler *>(loader.data())->testSuite(); break;
        case XsdSuite: suite = static_cast<XSDTestSuiteHandler *>(loader.data())->testSuite(); break;
        default: Q_ASSERT(false); break;
    }

    if(suite)
        return suite;

    errorMsg = QString::fromLatin1("Failed to load \"%1\". "
                                   "It appears to have no test-suite element.\n").arg(fileName.toString());
    return 0;
}

void TestSuite::toXML(XMLWriter &receiver, TestCase *const tc) const
{
    // TODO startElement() endElement() calls can be simplified.

    Q_ASSERT(tc);

    receiver.startDocument();
    /* <test-suite> */
    QXmlStreamAttributes test_suiteAtts;
    test_suiteAtts.append(QLatin1StringView("CatalogDesignDate"), m_designDate.toString(Qt::ISODate));
    test_suiteAtts.append(QLatin1StringView("version"), m_version);
    test_suiteAtts.append(QLatin1StringView("SourceOffsetPath"), QString());
    test_suiteAtts.append(QLatin1StringView("ResultOffsetPath"), QString());
    test_suiteAtts.append(QLatin1StringView("XQueryQueryOffsetPath"), QString());
    test_suiteAtts.append(QLatin1StringView("QueryXQueryOffsetPath"), QString());
    test_suiteAtts.append(QLatin1StringView("XQueryFileExtension"), QString());
    test_suiteAtts.append(QLatin1StringView("XQueryXFileExtension"), QString());

    receiver.startPrefixMapping(QString(), Global::xqtsCatalogNS);
    receiver.startElement(QLatin1StringView("test-suite"), test_suiteAtts);

    /* <test-group> */
    QXmlStreamAttributes test_groupAtts;
    test_groupAtts.append(QLatin1StringView("GeneratedGroupByPatternistSDKRunSuite"), QString());
    receiver.startElement(QLatin1StringView("test-group"), test_groupAtts);

    /* <GroupInfo> */
    receiver.startElement(QLatin1StringView("GroupInfo"), test_groupAtts);

    /* <title> */
    receiver.startElement(QLatin1StringView("title"), test_groupAtts);
    receiver.characters(QLatin1StringView("Contains the test case generated by PatternistSDKRunSuite."));

    /* </title> */
    receiver.endElement(QLatin1StringView("title"));

    /* <description> */
    receiver.startElement(QLatin1StringView("description"), test_groupAtts);
    /* </description> */
    receiver.endElement(QLatin1StringView("description"));

    /* </GroupInfo> */
    receiver.endElement(QLatin1StringView("GroupInfo"));

    /* <test-case> */
    tc->toXML(receiver);
    /* </test-case> */

    /* </test-group> */
    receiver.endElement(QLatin1StringView("test-group"));

    /* </test-suite> */
    receiver.endElement(QLatin1StringView("test-suite"));
}

QString TestSuite::version() const
{
    return m_version;
}

QDate TestSuite::designDate() const
{
    return m_designDate;
}

void TestSuite::setVersion(const QString &ver)
{
    m_version = ver;
}

void TestSuite::setDesignDate(const QDate &date)
{
    m_designDate = date;
}

TestContainer *TestSuite::parent() const
{
    return 0;
}

// vim: et:ts=4:sw=4:sts=4
