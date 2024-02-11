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

#include "qatomicmathematicianlocators_p.h"
#include "qbuiltintypes_p.h"

#include "qbuiltinatomictypes_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

/* -------------------------------------------------------------- */
#define implAccept(className)                                                                       \
AtomicTypeVisitorResult::Ptr className##Type::accept(const AtomicTypeVisitor::Ptr &v,               \
                                                     const SourceLocationReflection *const r) const \
{                                                                                                   \
    return v->visit(this, r);                                                                       \
}                                                                                                   \
                                                                                                    \
AtomicTypeVisitorResult::Ptr                                                                        \
className##Type::accept(const ParameterizedAtomicTypeVisitor::Ptr &v,                               \
                        const qint16 op,                                                            \
                        const SourceLocationReflection *const r) const                              \
{                                                                                                   \
    return v->visit(this, op, r);                                                                   \
}

#define deployComp(className, qname, parent, comp, mather, caster)                          \
className##Type::className##Type() : BuiltinAtomicType(BuiltinTypes::parent,                \
                                                       comp,                                \
                                                       mather,                              \
                                                       caster)                              \
{                                                                                           \
}                                                                                           \
implAccept(className)

#define deployBase(className, qname, parent) deployComp(className, qname, parent,           \
                                                        AtomicComparatorLocator::Ptr(),     \
                                                        AtomicMathematicianLocator::Ptr(),  \
                                                        AtomicCasterLocator::Ptr())

#define deployFull(className, qname, parent)                                                \
deployComp(className, qname, parent,                                                        \
           AtomicComparatorLocator::Ptr(new className##ComparatorLocator()),                \
           AtomicMathematicianLocator::Ptr(),                                               \
           AtomicCasterLocator::Ptr(new To##className##CasterLocator()))

#define deployMathComp(className, qname, parent)                                            \
deployComp(className, qname, parent,                                                        \
           AtomicComparatorLocator::Ptr(new className##ComparatorLocator()),                \
           AtomicMathematicianLocator::Ptr(new className##MathematicianLocator()),          \
           AtomicCasterLocator::Ptr(new To##className##CasterLocator()))
/* -------------------------------------------------------------- */

/* -------------------------------------------------------------- */
/* xs:anyURI & xs:untypedAtomic are much treated like strings. This ensures
 * they get the correct operators and automatically takes care of type promotion. */
deployComp(UntypedAtomic,       xsUntypedAtomic,       xsAnyAtomicType,
           AtomicComparatorLocator::Ptr(new StringComparatorLocator()),
           AtomicMathematicianLocator::Ptr(),
           AtomicCasterLocator::Ptr(new ToUntypedAtomicCasterLocator()))
deployComp(AnyURI,              xsAnyURI,            xsAnyAtomicType,
           AtomicComparatorLocator::Ptr(new StringComparatorLocator()),
           AtomicMathematicianLocator::Ptr(),
           AtomicCasterLocator::Ptr(new ToAnyURICasterLocator()))

deployBase(NOTATION,            xsNOTATION,                 xsAnyAtomicType)

deployMathComp(Float,               xsFloat,                numeric)
deployMathComp(Double,              xsDouble,               numeric)
deployMathComp(Decimal,             xsDecimal,              numeric)
deployMathComp(DayTimeDuration,     xsDayTimeDuration,      xsDuration)
deployMathComp(YearMonthDuration,   xsYearMonthDuration,    xsDuration)
deployMathComp(Date,                xsDate,                 xsAnyAtomicType)
deployMathComp(DateTime,            xsDateTime,             xsAnyAtomicType)
deployMathComp(SchemaTime,          xsTime,                 xsAnyAtomicType)

deployFull(Base64Binary,        xsBase64Binary,             xsAnyAtomicType)
deployFull(Boolean,             xsBoolean,                  xsAnyAtomicType)
deployFull(Duration,            xsDuration,                 xsAnyAtomicType)
deployFull(GDay,                xsGDay,                     xsAnyAtomicType)
deployFull(GMonth,              xsGMonth,                   xsAnyAtomicType)
deployFull(GMonthDay,           xsGMonthDay,                xsAnyAtomicType)
deployFull(GYear,               xsGYear,                    xsAnyAtomicType)
deployFull(GYearMonth,          xsGYearMonth,               xsAnyAtomicType)
deployFull(HexBinary,           xsHexBinary,                xsAnyAtomicType)
deployFull(QName,               xsQName,                    xsAnyAtomicType)
/* --------------------------------------------------------------- */

/* --------------------------------------------------------------- */
StringType::StringType(const AtomicType::Ptr &pType,
                       const AtomicCasterLocator::Ptr &casterLoc)
: BuiltinAtomicType(pType,
                    AtomicComparatorLocator::Ptr(new StringComparatorLocator()),
                    AtomicMathematicianLocator::Ptr(),
                    casterLoc)
{
}
implAccept(String)
/* --------------------------------------------------------------- */

/* --------------------------------------------------------------- */
IntegerType::IntegerType(const AtomicType::Ptr &pType,
                         const AtomicCasterLocator::Ptr &casterLoc)
: BuiltinAtomicType(pType,
                    AtomicComparatorLocator::Ptr(new IntegerComparatorLocator()),
                    AtomicMathematicianLocator::Ptr(new IntegerMathematicianLocator()),
                    casterLoc)
{
}
implAccept(Integer)
/* --------------------------------------------------------------- */

/* ---------------------- Special Overrides ---------------------- */
AnyAtomicType::AnyAtomicType() : BuiltinAtomicType(AtomicType::Ptr(),
                                                   AtomicComparatorLocator::Ptr(),
                                                   AtomicMathematicianLocator::Ptr(),
                                                   AtomicCasterLocator::Ptr())
{
}
implAccept(AnyAtomic)

ItemType::Ptr AnyAtomicType::xdtSuperType() const
{
    return BuiltinTypes::item;
}

SchemaType::Ptr AnyAtomicType::wxsSuperType() const
{
    return BuiltinTypes::xsAnySimpleType;
}

bool AnyAtomicType::isAbstract() const
{
    return true;
}

bool NOTATIONType::isAbstract() const
{
    return true;
}

#define implementName(className, typeName)                          \
QXmlName className##Type::name(const NamePool::Ptr &np) const          \
{                                                                   \
    return np->allocateQName(StandardNamespaces::xs, typeName);     \
}                                                                   \
                                                                    \
QString className##Type::displayName(const NamePool::Ptr &np) const \
{                                                                   \
    return np->displayName(name(np));                               \
}

implementName(AnyAtomic,            QLatin1StringView("anyAtomicType"))
implementName(AnyURI,               QLatin1StringView("anyURI"))
implementName(Base64Binary,         QLatin1StringView("base64Binary"))
implementName(Boolean,              QLatin1StringView("boolean"))
implementName(Date,                 QLatin1StringView("date"))
implementName(DateTime,             QLatin1StringView("dateTime"))
implementName(DayTimeDuration,      QLatin1StringView("dayTimeDuration"))
implementName(Decimal,              QLatin1StringView("decimal"))
implementName(Double,               QLatin1StringView("double"))
implementName(Duration,             QLatin1StringView("duration"))
implementName(Float,                QLatin1StringView("float"))
implementName(GDay,                 QLatin1StringView("gDay"))
implementName(GMonthDay,            QLatin1StringView("gMonthDay"))
implementName(GMonth,               QLatin1StringView("gMonth"))
implementName(GYearMonth,           QLatin1StringView("gYearMonth"))
implementName(GYear,                QLatin1StringView("gYear"))
implementName(HexBinary,            QLatin1StringView("hexBinary"))
implementName(Integer,              QLatin1StringView("integer"))
implementName(NOTATION,             QLatin1StringView("NOTATION"))
implementName(QName,                QLatin1StringView("QName"))
implementName(String,               QLatin1StringView("string"))
implementName(SchemaTime,                 QLatin1StringView("time"))
implementName(UntypedAtomic,        QLatin1StringView("untypedAtomic"))
implementName(YearMonthDuration,    QLatin1StringView("yearMonthDuration"))
/* --------------------------------------------------------------- */

#undef implAccept
#undef implementName
#undef deployComp
#undef deployBase
#undef deployFull
#undef deployMathComp

QT_END_NAMESPACE
