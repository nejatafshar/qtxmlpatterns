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

#ifndef Patternist_RangeExpression_H
#define Patternist_RangeExpression_H

#include <qpaircontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Implements XPath 2.0's @c to expression.
     *
     * Despite its name, RangeExpression is not related to RangeVariableDeclaration.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#construct_seq">XML Path Language
     * (XPath) 2.0, 3.3.1 Constructing Sequences</a>
     * @see RangeIterator
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_expressions
     */
    class RangeExpression : public PairContainer
    {
    public:
        RangeExpression(const Expression::Ptr &operand1, const Expression::Ptr &operand2);

        Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;
        /**
         * It's likely that this function gets called if staticType() inferred
         * the cardinality to an exact number. In that case, we know that the
         * first arguments is the same as the second argument.
         */
        Item evaluateSingleton(const DynamicContext::Ptr &) const override;

        SequenceType::List expectedOperandTypes() const override;

        /**
         * @returns always CommonSequenceTypes::ZeroOrMoreIntegers
         */
        SequenceType::Ptr staticType() const override;

        /**
         * Disables compression for optimization reasons. For example, the
         * expression "1 to 1000" would consume thousand allocated instances
         * of Integer, and RangeIterator is well suited for dynamic evaluation.
         *
         * @returns Expression::DisableElimination
         */
        Expression::Properties properties() const override;

        ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;
    };
}

QT_END_NAMESPACE

#endif
