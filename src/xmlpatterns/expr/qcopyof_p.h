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

#ifndef Patternist_CopyOf_H
#define Patternist_CopyOf_H

#include <qsinglecontainer_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short Does node copying in a parameterized way, in order to deal with
     * namespace preservation/inheritance.
     *
     * If someone tries to call evaluateEBV(), evaluateSingleton() or
     * evaluateSequence() on us, we will infinitely loop. But apparently
     * that's not possible because we're always a child of ElementConstructor,
     * currently.
     *
     * @author Frans Englich <frans.englich@nokia.com>
     * @ingroup Patternist_expressions
     */
    class CopyOf : public SingleContainer
    {
    public:
        /**
         * Creats a CopyOf where it is checked that the expression @p operand conforms
         * to the type @p reqType.
         */
        CopyOf(const Expression::Ptr &operand,
               const bool inheritNSS,
               const bool preserveNSS);

        void evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const override;

        /**
         * @returns always the SequenceType passed in the constructor to this class. That is, the
         * SequenceType that the operand must conform to.
         */
        SequenceType::Ptr staticType() const override;

        /**
         * @returns a list containing one CommonSequenceTypes::ZeroOrMoreItems
         */
        SequenceType::List expectedOperandTypes() const override;

        ExpressionVisitorResult::Ptr accept(const ExpressionVisitor::Ptr &visitor) const override;

        inline Item mapToItem(const Item &source,
                              const DynamicContext::Ptr &context) const;

        Expression::Ptr compress(const StaticContext::Ptr &context) override;

        Properties properties() const override;
        ItemType::Ptr expectedContextItemType() const override;

    private:
        typedef QExplicitlySharedDataPointer<const CopyOf> ConstPtr;
        const bool                                      m_inheritNamespaces;
        const bool                                      m_preserveNamespaces;
        const QAbstractXmlNodeModel::NodeCopySettings   m_settings;
    };
}

QT_END_NAMESPACE

#endif
