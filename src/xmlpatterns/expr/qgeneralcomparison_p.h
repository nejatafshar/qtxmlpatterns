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

#ifndef Patternist_GeneralComparison_H
#define Patternist_GeneralComparison_H

#include <qatomiccomparator_p.h>
#include <qpaircontainer_p.h>
#include <qcomparisonplatform_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Implements XPath 2.0's general comparions, such as the <tt>=</tt> operator.
     *
     * ComparisonPlatform is inherited with @c protected scope because ComparisonPlatform
     * must access members of GeneralComparison.
     *
     * @see <a href="http://www.w3.org/TR/xpath20/#id-general-comparisons">XML Path Language
     * (XPath) 2.0, 3.5.2 General Comparisons</a>
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_expressions
     */
    class Q_AUTOTEST_EXPORT GeneralComparison
      : public PairContainer, public ComparisonPlatform<GeneralComparison,
                                                        true /* We want to report errors. */,
                                                        AtomicComparator::AsGeneralComparison>
    {
    public:
        GeneralComparison(const Expression::Ptr &op1,
                          const AtomicComparator::Operator op,
                          const Expression::Ptr &op2,
                          const bool isBackwardsCompat = false);
        ~GeneralComparison();

        bool evaluateEBV(const DynamicContext::Ptr &) const override;
        Expression::Ptr typeCheck(const StaticContext::Ptr &context,
                                  const SequenceType::Ptr &reqType) override;

        SequenceType::List expectedOperandTypes() const override;
        SequenceType::Ptr staticType() const override;

        ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

        /**
         * @returns always IDGeneralComparison
         */
        ID id() const override;

        QList<QExplicitlySharedDataPointer<OptimizationPass> > optimizationPasses() const override;

        /**
         * @returns the operator that this GeneralComparison is using.
         */
        inline AtomicComparator::Operator operatorID() const
        {
            return m_operator;
        }

        /**
         * Overridden to optimize case-insensitive compares.
         */
        Expression::Ptr compress(const StaticContext::Ptr &context) override;

    private:
        static inline void updateType(ItemType::Ptr &type,
                                      const Expression::Ptr &source);
        AtomicComparator::Ptr fetchGeneralComparator(Expression::Ptr &op1,
                                                     Expression::Ptr &op2,
                                                     const ReportContext::Ptr &context) const;
        bool generalCompare(const Item &op1,
                            const Item &op2,
                            const DynamicContext::Ptr &context) const;

        const AtomicComparator::Operator m_operator;
        const bool m_isBackwardsCompat;
    };
}

QT_END_NAMESPACE

#endif
