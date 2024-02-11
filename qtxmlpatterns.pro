QT       += core network xml core5compat core-private

TARGET = qtxmlpatterns

TEMPLATE = lib
CONFIG += staticlib

CONFIG += exceptions

DEFINES += QT_NO_USING_NAMESPACE QT_ENABLE_QEXPLICITLYSHAREDDATAPOINTER_STATICCAST
win32-msvc*|win32-icc:QMAKE_LFLAGS += /BASE:0x61000000

INCLUDEPATH += src/xmlpatterns/acceltree
INCLUDEPATH += src/xmlpatterns/api
INCLUDEPATH += src/xmlpatterns/data
INCLUDEPATH += src/xmlpatterns/environment
INCLUDEPATH += src/xmlpatterns/expr
INCLUDEPATH += src/xmlpatterns/functions
INCLUDEPATH += src/xmlpatterns/iterators
INCLUDEPATH += src/xmlpatterns/janitors
INCLUDEPATH += src/xmlpatterns/parser
INCLUDEPATH += src/xmlpatterns/projection
INCLUDEPATH += src/xmlpatterns/schema
INCLUDEPATH += src/xmlpatterns/type
INCLUDEPATH += src/xmlpatterns/utils
INCLUDEPATH += src/xmlpatterns/qobjectmodel

include(src/xmlpatterns/common.pri)
include(src/xmlpatterns/acceltree/acceltree.pri)
include(src/xmlpatterns/api/api.pri)
include(src/xmlpatterns/data/data.pri)
include(src/xmlpatterns/environment/environment.pri)
include(src/xmlpatterns/expr/expr.pri)
include(src/xmlpatterns/functions/functions.pri)
include(src/xmlpatterns/iterators/iterators.pri)
include(src/xmlpatterns/janitors/janitors.pri)
include(src/xmlpatterns/parser/parser.pri)
include(src/xmlpatterns/projection/projection.pri)

include(src/xmlpatterns/schema/schema.pri)
include(src/xmlpatterns/type/type.pri)
include(src/xmlpatterns/utils/utils.pri)
include(src/xmlpatterns/qobjectmodel/qobjectmodel.pri, "", true)
