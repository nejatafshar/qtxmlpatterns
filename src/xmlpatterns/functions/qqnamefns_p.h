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

#ifndef Patternist_QNameFNs_H
#define Patternist_QNameFNs_H

#include <qfunctioncall_p.h>

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#QName-funcs">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 11 Functions Related to QNames</a>.
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Implements the function <tt>fn:QXmlName()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class QNameFN : public FunctionCall
    {
    public:
        Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
    };

    /**
     * @short Implements the function <tt>fn:resolve-QXmlName()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class ResolveQNameFN : public FunctionCall
    {
    public:
        Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
    };

    /**
     * @short Implements the function <tt>fn:prefix-from-QXmlName()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class PrefixFromQNameFN : public FunctionCall
    {
    public:
        Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
    };

    /**
     * @short Implements the function <tt>fn:local-name-from-QXmlName()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class LocalNameFromQNameFN : public FunctionCall
    {
    public:
        Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
    };

    /**
     * @short Implements the function <tt>fn:local-name-from-QXmlName()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class NamespaceURIFromQNameFN : public FunctionCall
    {
    public:
        Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
    };

    /**
     * @short Implements the function <tt>fn:namespace-uri-from-QXmlName()</tt>.
     *
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_functions
     */
    class NamespaceURIForPrefixFN : public FunctionCall
    {
    public:
        Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
    };

    /**
     * @short Implements the function <tt>fn:in-scope-prefixes()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class InScopePrefixesFN : public FunctionCall
    {
    public:
        Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &context) const override;
    };
}

QT_END_NAMESPACE

#endif
