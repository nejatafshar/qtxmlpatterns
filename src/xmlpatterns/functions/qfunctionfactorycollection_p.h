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


#ifndef Patternist_FunctionFactoryCollection_H
#define Patternist_FunctionFactoryCollection_H

#include <qfunctionfactory_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short A FunctionFactoryCollection instance is a FunctionFactory in its own right,
     * but looks in its contained collection of factories for requested functions.
     *
     * @note the order of adding function libraries is significant.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class Q_AUTOTEST_EXPORT FunctionFactoryCollection: public FunctionFactory
                                                     , public FunctionFactory::List
    {
    public:

        typedef QExplicitlySharedDataPointer<FunctionFactoryCollection> Ptr;

        /**
         * Creates a function call node.
         */
        Expression::Ptr createFunctionCall(const QXmlName,
                                           const Expression::List &arguments,
                                           const StaticContext::Ptr &context,
                                           const SourceLocationReflection *const r) override;
        bool isAvailable(const NamePool::Ptr &np, const QXmlName name, const xsInteger arity) override;

        FunctionSignature::Hash functionSignatures() const override;

        FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QXmlName name) override;

        /**
         * @return a FunctionFactory containing all core functions and constructor
         * functions required for XPath 2.. The functions specified for XQuery 1.0
         * are the same as for XPath 2.0 so this FunctionFactory work for XQuery
         * as well.
         */
        static FunctionFactory::Ptr xpath20Factory(const NamePool::Ptr &np);

        /**
         * @return a FunctionFactory containing all core functions required for XPath 1.0.
         */
        static FunctionFactory::Ptr xpath10Factory();

        /**
         * @return a FunctionFactory containing all core functions required for XSL-T 2.0
         * functions.
         */
        static FunctionFactory::Ptr xslt20Factory(const NamePool::Ptr &np);
    };
}

QT_END_NAMESPACE

#endif
