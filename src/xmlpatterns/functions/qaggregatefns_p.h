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

#ifndef Patternist_AggregateFNs_H
#define Patternist_AggregateFNs_H

#include <qaggregator_p.h>
#include <qatomiccomparator_p.h>
#include <qatomicmathematician_p.h>
#include <qcomparisonplatform_p.h>

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#aggregate-functions">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 15.4 Aggregate Functions</a>.
 *
 * @todo document that some functions have both eval funcs implented.
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Implements the function <tt>fn:count()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class CountFN : public FunctionCall
    {
    public:
        Item evaluateSingleton(const DynamicContext::Ptr &context) const override;

        /**
         * If @p reqType is CommonSequenceTypes::EBV, this function call is rewritten
         * into a call to <tt>fn:exists()</tt>. Hence, <tt>if(count(X)) then ...</tt> is
         * rewritten into <tt>if(exists(X)) then ...</tt>.
         */
        Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                  const SequenceType::Ptr &reqType) override;
        /**
         * If CountFN's operand has a Cardinality that is exact, as per Cardinality::isExact(),
         * it is rewritten to the Cardinality's count.
         */
        Expression::Ptr compress(const StaticContext::Ptr &context) override;
    };

    /**
     * @short Base class for the implementations of the <tt>fn:avg()</tt> and <tt>fn:sum()</tt> function.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class AddingAggregate : public FunctionCall
    {
    public:
        Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                  const SequenceType::Ptr &reqType) override;
    protected:
        AtomicMathematician::Ptr m_mather;
    };

    /**
     * @short Implements the function <tt>fn:avg()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class AvgFN : public AddingAggregate
    {
    public:
        Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
        Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                  const SequenceType::Ptr &reqType) override;

        SequenceType::Ptr staticType() const override;
    private:
        AtomicMathematician::Ptr m_adder;
        AtomicMathematician::Ptr m_divider;
    };

    /**
     * @short Implements the function <tt>fn:sum()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class SumFN : public AddingAggregate
    {
    public:
        Item evaluateSingleton(const DynamicContext::Ptr &context) const override;
        Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                  const SequenceType::Ptr &reqType) override;
        SequenceType::Ptr staticType() const override;
    };
}

QT_END_NAMESPACE

#endif
