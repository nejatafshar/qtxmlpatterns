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

#ifndef Patternist_Atomizer_H
#define Patternist_Atomizer_H

#include <qitem_p.h>
#include <qsinglecontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Performs atomization. Effectively, it is an implementation
     * of the <tt>fn:data()</tt> function.
     *
     * @see <a href="http://www.w3.org/TR/xpath-functions/#func-data">XQuery 1.0 and XPath
     * 2.0 Functions and Operators, 2.4 fn:data</a>
     * @see <a href="http://www.w3.org/TR/xpath20/#id-atomization">XML
     * Path Language (XPath) 2.0, 2.4.2 Atomization</a>
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_expressions
     */
    class Atomizer : public SingleContainer
    {
    public:
        Atomizer(const Expression::Ptr &operand);

        Item evaluateSingleton(const DynamicContext::Ptr &) const override;
        Item::Iterator::Ptr evaluateSequence(const DynamicContext::Ptr &) const override;

        SequenceType::Ptr staticType() const override;

        SequenceType::List expectedOperandTypes() const override;
        const SourceLocationReflection *actualReflection() const override;

        /**
         * Makes an early compression, by returning the result of
         * the type checked operand, if the operand has the static type
         * xs:anyAtomicType(no atomization needed).
         */
        Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                  const SequenceType::Ptr &reqType) override;

        inline Item::Iterator::Ptr mapToSequence(const Item &item,
                                                 const DynamicContext::Ptr &context) const;
        ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

    private:
        typedef QExplicitlySharedDataPointer<const Atomizer> ConstPtr;
    };
}

QT_END_NAMESPACE

#endif
