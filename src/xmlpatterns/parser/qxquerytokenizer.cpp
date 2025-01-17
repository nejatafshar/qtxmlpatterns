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

#include <QByteArray>

#include "qparsercontext_p.h"
#include "qquerytransformparser_p.h"

#include "qxquerytokenizer_p.h"

#include "qtokenlookup.cpp"

QT_BEGIN_NAMESPACE

namespace QPatternist
{

#define handleWhitespace()                      \
{                                               \
    const TokenType t = consumeWhitespace();    \
    if (t != T_SUCCESS)                            \
        return Token(t);                        \
}

XQueryTokenizer::XQueryTokenizer(const QString &query,
                                 const QUrl &location,
                                 const State startingState) : Tokenizer(location)
                                                            , m_data(query)
                                                            , m_length(query.length())
                                                            , m_state(startingState)
                                                            , m_pos(0)
                                                            , m_line(1)
                                                            , m_columnOffset(0)
                                                            , m_scanOnly(false)
{
    Q_ASSERT(location.isValid() || location.isEmpty());
}

const QChar XQueryTokenizer::current() const
{
    if (m_pos < m_length)
        return m_data.at(m_pos);
    else
        return QChar();
}

char XQueryTokenizer::peekCurrent() const
{
    return current().toLatin1();
}

int XQueryTokenizer::peekForColonColon() const
{
    /* Note, we don't modify m_pos in this function, so we need to do offset
     * calculations. */
    int pos = m_pos;

    while(pos < m_length)
    {
        switch(m_data.at(pos).toLatin1())
        {
            /* Fallthrough these four. */
            case ' ':
            case '\t':
            case '\n':
            case '\r':
                break;
            case ':':
            {
                if (peekAhead((pos - m_pos) + 1) == ':')
                    return pos - m_pos;
                Q_FALLTHROUGH();
            }
            default:
                return -1;
        }
        ++pos;
    }

    return -1;
}

Tokenizer::Token XQueryTokenizer::tokenAndChangeState(const TokenType code,
                                                      const State s,
                                                      const int advance)
{
    Q_ASSERT(advance >= 0);
    m_pos += advance;
    setState(s);
    return Token(code);
}

Tokenizer::Token XQueryTokenizer::tokenAndChangeState(const TokenType code,
                                                      const QString &value,
                                                      const State s)
{
    setState(s);
    return Token(code, value);
}

Tokenizer::Token XQueryTokenizer::tokenAndAdvance(const TokenType code,
                                                  const int advance)
{
    Q_ASSERT(advance >= 0);
    m_pos += advance;
    return Token(code);
}

QString XQueryTokenizer::normalizeEOL(const QString &input,
                                      const CharacterSkips &characterSkips)
{
    const int len = input.length();
    QString result;

    /* The likely hood is rather high it'll be the same content. */
    result.reserve(len);

    for(int i = 0; i < len; ++i)
    {
        const QChar &at = input.at(i);

        if (characterSkips.contains(i))
        {
            result.append(at);
            continue;
        }
        switch(input.at(i).unicode())
        {
            case '\r':
            {
                if (i + 1 < len && input.at(i + 1) == QLatin1Char('\n'))
                    ++i;

                Q_FALLTHROUGH();
            }
            case '\n':
            {
                result.append(QLatin1Char('\n'));
                continue;
            }
            default:
            {
                result.append(at);
            }
        }
    }

    return result;
}

Tokenizer::TokenType XQueryTokenizer::consumeComment()
{
    /* Below, we return ERROR instead of END_OF_FILE such that the parser
     * sees an invalid comment. */
    while(m_pos < m_length)
    {
        switch(peekCurrent())
        {
            case ':':
            {
                ++m_pos; /* Consume ':' */
                if (atEnd())
                    return T_ERROR;

                if (peekCurrent() == ')')
                {
                    ++m_pos; /* Consume ')' */
                    return T_SUCCESS; /* The comment closed nicely. */
                }
                continue; /* We don't want to increment m_pos twice. */
            }
            case '(':
            { /* It looks like the start of a comment. */
                ++m_pos;

                if (atEnd())
                    return T_END_OF_FILE;
                else if (peekCurrent() == ':')
                {
                    /* And it is a nested comment -- parse it. */
                    const TokenType retval = consumeComment();
                    if (retval == T_SUCCESS)
                        continue; /* Continue with our "own" comment. */
                    else
                        return retval; /* Return the error in the nested comment. */
                }
                break;
            }
            case '\n':
            case '\r':
            {
                /* We want to count \r\n as a single line break. */
                if (peekAhead() == '\n')
                    ++m_pos;

                m_columnOffset = m_pos;
                ++m_line;

                break;
            }
        }
        ++m_pos;
    }

    return T_ERROR; /* Error: we reached the end while inside a comment. */
}

bool XQueryTokenizer::consumeRawWhitespace()
{
    while(m_pos < m_length)
    {
        switch(peekCurrent())
        {
            case ' ':
            case '\t':
                break;
            case '\n':
            case '\r':
            {
                if (peekAhead() == '\n')
                    ++m_pos;

                m_columnOffset = m_pos;
                ++m_line;

                break;
            }
            default:
                return false;
        }
        ++m_pos;
    }
    return true;
}

Tokenizer::TokenType XQueryTokenizer::consumeWhitespace()
{
    while(m_pos < m_length)
    {
        switch(peekCurrent())
        {
            case ' ':
            case '\t':
                break;
            case '\n':
            case '\r':
            {
                /* We want to count \r\n as a single line break. */
                if (peekAhead() == '\n')
                    ++m_pos;

                m_columnOffset = m_pos;
                ++m_line;

                break;
            }
            case '(':
            {
                if (peekAhead() == ':')
                {
                    m_pos += 2; /* Consume "(:" */

                    const TokenType comment = consumeComment();
                    if (comment == T_SUCCESS)
                        continue;
                    else
                        return comment;
                }
                Q_FALLTHROUGH();
            }
            default:
                return T_SUCCESS;
        }
        ++m_pos;
    }

    return T_END_OF_FILE;
}

char XQueryTokenizer::peekAhead(const int length) const
{
    if (m_pos + length < m_length)
        return m_data.at(m_pos + length).toLatin1();
    else
        return 0;
}

Tokenizer::Token XQueryTokenizer::error()
{
    return Token(T_ERROR);
}

bool XQueryTokenizer::isDigit(const char ch)
{
    return ch >= '0' && ch <= '9';
}

/* Replace with function in QXmlUtils. Write test cases for this. */
bool XQueryTokenizer::isNCNameStart(const QChar ch)
{
    if (ch == QLatin1Char('_'))
        return true;

    switch(ch.category())
    {
        case QChar::Letter_Lowercase:
        case QChar::Letter_Uppercase:
        case QChar::Letter_Other:
        case QChar::Letter_Titlecase:
        case QChar::Number_Letter:
            return true;
        default:
            return false;
    }
}

bool XQueryTokenizer::isNCNameBody(const QChar ch)
{
    switch(ch.unicode())
    {
        case '.':
        case '_':
        case '-':
            return true;
    }

    switch(ch.category())
    {
        case QChar::Letter_Lowercase:
        case QChar::Letter_Uppercase:
        case QChar::Letter_Other:
        case QChar::Letter_Titlecase:
        case QChar::Number_Letter:
        case QChar::Mark_SpacingCombining:
        case QChar::Mark_Enclosing:
        case QChar::Mark_NonSpacing:
        case QChar::Letter_Modifier:
        case QChar::Number_DecimalDigit:
            return true;
        default:
            return false;
    }
}

bool XQueryTokenizer::isPhraseKeyword(const TokenType code)
{
    switch(code)
    {
        /* Fallthrough all these. */
        case T_CASTABLE:
        case T_CAST:
        case T_COPY_NAMESPACES:
        case T_DECLARE:
        case T_EMPTY:
        case T_MODULE:
        case T_IMPORT:
        case T_INSTANCE:
        case T_ORDER:
        case T_ORDERING:
        case T_XQUERY:
        case T_STABLE:
        case T_TREAT:
            return true;
        default:
            return false;
    }
}

bool XQueryTokenizer::isOperatorKeyword(const TokenType code)
{
    switch(code)
    {
        /* Fallthrough all these. */
        case T_AS:
        case T_ASCENDING:
        case T_AT:
        case T_CASE:
        case T_CAST:
        case T_CASTABLE:
        case T_EQ:
        case T_EXTERNAL:
        case T_GE:
        case T_G_EQ:
        case T_G_GT:
        case T_G_LT:
        case T_G_NE:
        case T_GT:
        case T_IN:
        case T_INHERIT:
        case T_INSTANCE:
        case T_IS:
        case T_ITEM:
        case T_LE:
        case T_LT:
        case T_NE:
        case T_NO_INHERIT:
        case T_NO_PRESERVE:
        case T_OF:
        case T_PRESERVE:
        case T_RETURN:
        case T_STABLE:
        case T_TO:
        case T_TREAT:
            return true;
        default:
            return false;
    };
}

bool XQueryTokenizer::isTypeToken(const TokenType t)
{
    switch(t)
    {
        /* Fallthrough all these. */
        case T_ATTRIBUTE:
        case T_COMMENT:
        case T_DOCUMENT:
        case T_DOCUMENT_NODE:
        case T_ELEMENT:
        case T_ITEM:
        case T_NODE:
        case T_PROCESSING_INSTRUCTION:
        case T_SCHEMA_ATTRIBUTE:
        case T_SCHEMA_ELEMENT:
        case T_TEXT:
            return true;
        default:
            return false;
    }
}

Tokenizer::Token XQueryTokenizer::tokenizeNCNameOrQName()
{
    const int start = m_pos;

    const Token t1 = tokenizeNCName();
    if (t1.hasError())
        return t1;

    if (peekCurrent() != ':' || peekAhead() == '=')
        return t1;

    ++m_pos;

    const Token t2 = tokenizeNCName();
    if (t2.hasError())
        return t2;
    else
        return Token(T_QNAME, m_data.mid(start, m_pos - start));
}

Tokenizer::Token XQueryTokenizer::tokenizeNumberLiteral()
{
    setState(Operator);
    const int startPos = m_pos;
    bool hasDot = false;
    bool isXPath20 = false;

    for(; m_pos < m_length; ++m_pos)
    {
        QChar ch(current());

        char cell = ch.cell();

        if (cell == 'e' || cell == 'E')
        {
            isXPath20 = true;
            ++m_pos;
            ch = current();

            if (ch.row() != 0)
                break;

            cell = ch.cell();

            if (cell == '+' || cell == '-')
                continue;
        }

        if (isNCNameStart(ch))
            return error();

        if (cell < '0' || cell > '9')
        {
            if (cell == '.' && !hasDot)
                hasDot = true;
            else
                break;
        }
    }

    return Token(isXPath20 ? T_XPATH2_NUMBER : T_NUMBER, m_data.mid(startPos, m_pos - startPos));
}

QString XQueryTokenizer::tokenizeCharacterReference()
{
    Q_ASSERT(peekCurrent() == '&');

    const int theEnd = m_data.indexOf(QLatin1Char(';'), m_pos + 1);

    if (theEnd == -1) /* No ';' found, a syntax error. i18n. */
        return QString();

    QString content(m_data.mid(m_pos + 1, (theEnd - m_pos) - 1));
    m_pos = theEnd;

    const QChar charRef(charForReference(content));

    if (!charRef.isNull())
        return charRef;
    else if (content.startsWith(QLatin1Char('#')))
    {
        int base;

        /* It is only '#' or '#x'. */
        if (content.length() < 2)
            return QString();

        /* We got a hex number if it starts with 'x', otherwise it's a decimal. */
        if (content.at(1) == QLatin1Char('x'))
        {
            base = 16;
            content = content.mid(2); /* Remove "#x". */
        }
        else
        {
            base = 10;
            content = content.mid(1); /* Remove "#". */
        }

        bool conversionOK = false;
        const int codepoint = content.toInt(&conversionOK, base);

        if (conversionOK)
        {
            const QChar ch(codepoint);

            if (ch.isNull())
            {
                /* We likely have something which require surrogate pairs. */
                QString result;
                result += QChar(QChar::highSurrogate(codepoint));
                result += QChar(QChar::lowSurrogate(codepoint));
                return result;
            }
            else
                return ch;
        }
        else
            return QString();
    }
    else
        return QString();
}

int XQueryTokenizer::scanUntil(const char *const content)
{
    const int end = m_data.indexOf(QString::fromLatin1(content), m_pos);

    if (end == -1)
        return -1;
    else
    {
        const int len = end - m_pos;
        m_pos += len;
        return len;
    }
}

QChar XQueryTokenizer::charForReference(const QString &reference)
{
    if (m_charRefs.isEmpty())
    {
        /* Initialize. */
        m_charRefs.reserve(5);
        m_charRefs.insert(QLatin1StringView("lt"),     QLatin1Char('<'));
        m_charRefs.insert(QLatin1StringView("gt"),     QLatin1Char('>'));
        m_charRefs.insert(QLatin1StringView("amp"),    QLatin1Char('&'));
        m_charRefs.insert(QLatin1StringView("quot"),   QLatin1Char('"'));
        m_charRefs.insert(QLatin1StringView("apos"),   QLatin1Char('\''));
    }

    return m_charRefs.value(reference);
}

Tokenizer::Token XQueryTokenizer::tokenizeStringLiteral()
{
    const QChar delimiter(current());
    /* We cannot unfortunately just scan and then do mid(),
     * since we can encounter character references. */
    QString result;

    /* This is more likely than QString's default allocation. */
    result.reserve(8);

    CharacterSkips skipEOLNormalization;

    /* Advance over the initial quote character. */
    ++m_pos;

    for(; m_pos < m_length; ++m_pos)
    {
        const QChar c(current());

        if (c == QLatin1Char('&'))
        {
            const QString charRef(tokenizeCharacterReference());

            if (charRef.isNull())
                return error();
            else
            {
                skipEOLNormalization.insert(result.length());
                result.append(charRef);
            }

        }
        else if (c == delimiter)
        {
            /* Maybe the escaping mechanism is used. For instance, "s""s"
             * has the value `s"s'. */
            ++m_pos;

            if (current() == delimiter) /* Double quote. */
                result += delimiter;
            else
                return Token(T_STRING_LITERAL, normalizeEOL(result, skipEOLNormalization));
        }
        else
            result += c;
    }

    return error();
}

Tokenizer::Token XQueryTokenizer::tokenizeNCName()
{
    const int startPos = m_pos;

    if (m_pos < m_length && isNCNameStart(current()))
    {
        ++m_pos;

        for(; m_pos < m_length; ++m_pos)
        {
            if (!isNCNameBody(current()))
                break;
        }

        return Token(T_NCNAME, m_data.mid(startPos, m_pos - startPos));
    }
    else
        return error();
}

bool XQueryTokenizer::aheadEquals(const char *const chs,
                                  const int len,
                                  const int offset) const
{
    Q_ASSERT(len > 0);
    Q_ASSERT(qstrlen(chs) == uint(len));

    if (m_pos + len >= m_length)
        return false;

    for(int i = offset; i < (len + offset); ++i)
    {
        if (m_data.at(m_pos + i).toLatin1() != chs[i - offset])
            return false;
    }

    return true;
}

const TokenMap *XQueryTokenizer::lookupKeyword(const QString &keyword)
{
    return TokenLookup::value(keyword.toLatin1().constData(), keyword.length());
}

XQueryTokenizer::State XQueryTokenizer::state() const
{
    return m_state;
}

void XQueryTokenizer::setState(const State s)
{
    m_state = s;
}

void XQueryTokenizer::pushState(const State s)
{
    m_stateStack.push(s);
}

void XQueryTokenizer::pushState()
{
    m_stateStack.push(m_state);
}

void XQueryTokenizer::popState()
{
    /* QStack::pop() asserts if it's empty, so we need to check
     * it, since we might receive unbalanced curlies. */
    if (!m_stateStack.isEmpty())
        m_state = m_stateStack.pop();
}

Tokenizer::Token XQueryTokenizer::nextToken()
{
    switch(state())
    {
        /* We want to skip or do special whitespace handling for these
         * states. So fallthrough all of the following. */
        case AposAttributeContent:
        case Axis:
        case ElementContent:
        case EndTag:
        case Pragma:
        case PragmaContent:
        case ProcessingInstructionName:
        case QuotAttributeContent:
        case StartTag:
        case XMLComment:
            break;
        default:
            handleWhitespace();
    }

    switch(state())
    {
        case XMLSpaceDecl:
        case NamespaceKeyword:
        {
            switch(peekCurrent())
            {
                case ',':
                    return tokenAndAdvance(T_COMMA);
                case '"':
                case '\'':
                {
                    setState(NamespaceDecl);
                    return tokenizeStringLiteral();
                }
            }

            const Token id(tokenizeNCName());

            if (id.type != T_NCNAME)
                return id;

            const TokenMap *const keyword = lookupKeyword(id.value);
            if (keyword)
            {
                switch(keyword->token)
                {
                    case T_INHERIT:
                    case T_NO_INHERIT:
                    {
                        setState(Default);
                        break;
                    }
                    case T_NAMESPACE:
                    {
                        setState(NamespaceDecl);
                        break;
                    }
                    case T_ORDERED:
                    case T_UNORDERED:
                    case T_STRIP:
                    {
                        setState(Default);
                        break;
                    }
                    case T_PRESERVE:
                    {
                        if (state() != NamespaceKeyword)
                            setState(Default);
                        break;
                    }
                    default:
                        break;
                }

                return Token(keyword->token);
            }
            else
                return id;
        }
        case NamespaceDecl:
        {
            switch(peekCurrent())
            {
                case '=':
                    return tokenAndAdvance(T_G_EQ);
                case ';':
                    return tokenAndChangeState(T_SEMI_COLON, Default);
                case '\'':
                case '\"':
                    return tokenizeStringLiteral();
            }

            const Token nc(tokenizeNCName());

            handleWhitespace();

            const char pc = peekCurrent();
            const TokenMap* const t = lookupKeyword(nc.value);

            if (pc == '\'' || (pc == '"' && t))
                return tokenAndChangeState(t->token, Default, 0);
            else
                return nc;
        }
        case Axis:
        {
            if (peekCurrent() == ':')
            {
                Q_ASSERT(peekAhead() == ':');
                m_pos += 2;
                setState(AfterAxisSeparator);
                return Token(T_COLONCOLON);
            }
            Q_FALLTHROUGH();
        }
        case AfterAxisSeparator:
        case Default:
           /* State Operator and state Default have a lot of tokens in common except
            * for minor differences. So we treat them the same way, and sprinkles logic
            * here and there to handle the small differences. */
            Q_FALLTHROUGH();
        case Operator:
        {
            switch(peekCurrent())
            {
                case '=':
                    return tokenAndChangeState(T_G_EQ, Default);
                case '-':
                    return tokenAndChangeState(T_MINUS, Default);
                case '+':
                    return tokenAndChangeState(T_PLUS, Default);
                case '[':
                    return tokenAndChangeState(T_LBRACKET, Default);
                case ']':
                    return tokenAndChangeState(T_RBRACKET, Operator);
                case ',':
                    return tokenAndChangeState(T_COMMA, Default);
                case ';':
                    return tokenAndChangeState(T_SEMI_COLON, Default);
                case '$':
                    return tokenAndChangeState(T_DOLLAR, VarName);
                case '|':
                    return tokenAndChangeState(T_BAR, Default);
                case '?':
                    return tokenAndChangeState(T_QUESTION, Operator);
                case ')':
                    return tokenAndChangeState(T_RPAREN, Operator);
                case '@':
                    return tokenAndChangeState(T_AT_SIGN, Default);
                /* Fallthrough all these. */
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                case '0':
                    return tokenizeNumberLiteral();
                case '.':
                {
                    const char next = peekAhead();
                    if (next == '.')
                        return tokenAndChangeState(T_DOTDOT, Operator, 2);
                    /* .5 is allowed, as short form for 0.5:
                     * <tt>[142]     DecimalLiteral     ::=     ("." Digits) | (Digits "." [0-9]*)</tt>
                     */
                    else if (isDigit(next))
                        return tokenizeNumberLiteral();
                    else
                        return tokenAndChangeState(T_DOT, Operator);
                }
                case '\'':
                case '"':
                {
                    setState(Operator);
                    return tokenizeStringLiteral();

                }
                case '(':
                {
                    if (peekAhead() == '#')
                        return tokenAndChangeState(T_PRAGMA_START, Pragma, 2);
                    else
                        return tokenAndChangeState(T_LPAREN, Default);
                }
                case '*':
                {
                    if (peekAhead() == ':')
                    {
                        m_pos += 2; /* Consume *:. */
                        const Token nc = tokenizeNCName();

                        if (nc.hasError())
                            return error();
                        else
                            return tokenAndChangeState(T_ANY_PREFIX, nc.value, Operator);
                    }
                    else
                        return tokenAndChangeState(T_STAR, state() == Default ? Operator : Default);
                }
                case ':':
                {
                    switch(peekAhead())
                    {
                        case '=':
                            return tokenAndChangeState(T_ASSIGN, Default, 2);
                        case ':':
                            return tokenAndChangeState(T_COLONCOLON, Default, 2);
                        default:
                            return error();
                    }
                }
                case '!':
                {
                    if (peekAhead() == '=')
                        return tokenAndChangeState(T_G_NE, Default, 2);
                    else
                        return error();
                }
                case '<':
                {
                    switch(peekAhead())
                    {
                        case '=':
                            return tokenAndChangeState(T_G_LE, Default, 2);
                        case '<':
                            return tokenAndChangeState(T_PRECEDES, Default, 2);
                        case '?':
                        {
                            pushState(Operator);
                            return tokenAndChangeState(T_PI_START, ProcessingInstructionName, 2);
                        }
                        case '!':
                        {
                            if (aheadEquals("!--", 3))
                            {
                                m_pos += 3; /* Consume "!--". */
                                pushState(Operator);
                                return tokenAndChangeState(T_COMMENT_START, XMLComment);
                            }
                            /* Fallthrough. It's a syntax error, and this is a good way to report it. */
                            Q_FALLTHROUGH();
                        }
                        default:
                        {
                            if ((m_pos + 1) < m_length && isNCNameStart(m_data.at(m_pos + 1)))
                            {
                                /* We assume it's an element constructor. */
                                pushState(Operator);
                            }

                            return tokenAndChangeState(T_G_LT, state() == Operator ? Default : StartTag);
                        }
                    }
                }
                case '>':
                {
                    switch(peekAhead())
                    {
                        case '=':
                            return tokenAndChangeState(T_G_GE, Default, 2);
                        case '>':
                            return tokenAndChangeState(T_FOLLOWS, Default, 2);
                        default:
                            return tokenAndChangeState(T_G_GT, Default);
                    }
                }
                case '/':
                {
                    if (peekAhead() == '/')
                        return tokenAndChangeState(T_SLASHSLASH, Default, 2);
                    else
                        return tokenAndChangeState(T_SLASH, Default);
                }
                case '{':
                {
                    pushState(Operator);
                    return tokenAndChangeState(T_CURLY_LBRACE, Default);
                }
                case '}':
                {
                    popState();

                    return tokenAndAdvance(T_CURLY_RBRACE);
                }
            }

            /* Ok. We're in state Default or Operator, and it wasn't a simple
             * character. */

            const Token id(tokenizeNCName());

            if (id.type != T_NCNAME)
                return id;

            const TokenMap *const keyword = lookupKeyword(id.value);

            if (state() == Operator)
            {
                if (keyword)
                {
                    if (keyword->token == T_DEFAULT || keyword->token == T_ASCENDING || keyword->token == T_DESCENDING)
                        setState(Operator);
                    else if (keyword->token == T_RETURN)
                        setState(Default);
                    else if (isPhraseKeyword(keyword->token))
                    {
                        const TokenType ws = consumeWhitespace();
                        if (ws == T_ERROR)
                            return error();

                        const Token id2(tokenizeNCName());
                        const TokenMap *const keyword2 = lookupKeyword(id2.value);

                        if (keyword2)
                        {
                            if (keyword->token == T_TREAT && keyword2->token == T_AS)
                                setState(ItemType);
                            else if (keyword->token == T_CAST || (keyword->token == T_CASTABLE && keyword2->token == T_AS) || keyword2->token == T_BY)
                                setState(Default);

                            m_tokenStack.push(Token(keyword2->token));
                        }
                        else
                            m_tokenStack.push(id2);

                        return Token(keyword->token);
                    }
                    else
                    {
                        /* Such that we tokenize the second token in "empty greatest". */
                        if (keyword->token != T_EMPTY)
                            setState(Default);
                    }

                    if (keyword->token == T_AS || keyword->token == T_CASE)
                        setState(ItemType);

                    return Token(keyword->token);
                }
                else
                    return id;
            }

            Q_ASSERT(state() == Default || state() == Axis || state() == AfterAxisSeparator);

            /*
             * This is hard. Consider this:
             *
             * Valid:           child       ::nameTest
             * Valid:           child::     nameTest
             * Syntax Error:    child       :localName
             * Syntax Error:    child:      localName
             *
             * Consider "child ::name". Right now, we're here:
             *                ^
             * We don't know whether "child" is a prefix and hence the whitespace is invalid,
             * or whether it's an axis and hence skippable. */
            {
                const int wsLength = peekForColonColon();
                /* We cannot call handleWhitespace() because it returns on
                 * END_OF_FILE, and we have parsed up keyword, and we need to
                 * deal with that.
                 *
                 * If we have a colon colon, which means the whitespace is
                 * allowed, we skip it. */
                if (wsLength != -1)
                    m_pos += wsLength;
            }

            /* Handle name tests. */
            if (peekCurrent() == ':')
            {
                switch(peekAhead())
                {
                    case '=':
                        return id;
                    case '*':
                    {
                        m_pos += 2;
                        return tokenAndChangeState(T_ANY_LOCAL_NAME, id.value, Operator);
                    }
                    case ':':
                    {
                        /* We have an axis. */
                        setState(Axis);
                        return keyword ? Token(keyword->token) : id;
                    }
                    default:
                    {
                        /* It's a QName. */
                        ++m_pos; /* Consume the colon. */

                        const Token id2(tokenizeNCName());

                        if (id2.type != T_NCNAME)
                        {
                            --m_pos;
                            return id;
                        }

                        setState(Operator);
                        const int qNameLen = id.value.length() + id2.value.length() + 1;
                        return Token(T_QNAME, m_data.mid(m_pos - qNameLen, qNameLen));
                    }
                }
            }

            if (!keyword || isOperatorKeyword(keyword->token))
            {
                setState(Operator);
                return id;
            }

            const TokenType ws = consumeWhitespace();
            if (ws == T_ERROR) // TODO this should test for success. Write test.
                return Token(T_ERROR);

            if (atEnd())
            {
                setState(Operator);
                return id;
            }

            /* Let the if-body apply for constructors, and node type tests. */
            if (isTypeToken(keyword->token) ||
               keyword->token == T_TYPESWITCH ||
               keyword->token == T_ORDERED ||
               keyword->token == T_UNORDERED ||
               keyword->token == T_IF)
            {
                switch(peekCurrent())
                {
                    case '(':
                    {
                        // TODO See if we can remove DOCUMENT from isTypeToken.
                        if (isTypeToken(keyword->token) && keyword->token != T_DOCUMENT)
                        {
                            m_tokenStack.push(Token(T_LPAREN));
                            ++m_pos; /* Consume '('. */
                            pushState(Operator);

                            if (keyword->token == T_PROCESSING_INSTRUCTION)
                                setState(KindTestForPI);
                            else
                                setState(KindTest);

                            return Token(keyword->token);
                        }
                        else if (keyword->token == T_TYPESWITCH || keyword->token == T_IF)
                            return Token(keyword->token);
                        else /* It's a function call. */
                            return id;
                    }
                    case '{':
                    {
                        m_tokenStack.push(Token(T_CURLY_LBRACE));
                        ++m_pos; /* Consume '{'. */
                        pushState(Operator);
                        /* Stay in state Default. */
                        return Token(keyword->token);
                    }
                    default:
                    {
                        /* We have read in a token which is for instance
                         * "return", and now it can be an element
                         * test("element") a node kind test("element()"), or a
                         * computed element constructor("element name {...").
                         * We need to do a two-token lookahead here, because
                         * "element return" can be an element test followed by
                         * the return keyword, but it can also be an element
                         * constructor("element return {"). */
                        if (isNCNameStart(current()))
                        {
                            const int currentPos = m_pos;
                            const Token token2 = tokenizeNCNameOrQName();

                            if (token2.hasError())
                                return token2;

                            handleWhitespace();

                            if (peekCurrent() == '{')
                            {
                                /* An element constructor. */
                                m_tokenStack.push(token2);
                                return Token(keyword->token);
                            }

                            /* We jump back in the stream, we need to tokenize token2 according
                             * to the state. */
                            m_pos = currentPos;
                            setState(Operator);
                            return Token(T_NCNAME, QLatin1StringView(keyword->name));
                        }
                    }
                }
            }

            if (peekCurrent() == '$')
            {
                setState(VarName);
                return Token(keyword->token);
            }

            /* It's not a node type, it's not the typeswitch expression, but it is a function callsite. */
            if (peekCurrent() == '(')
                return id;
            else if (peekCurrent() == '{' && keyword->token == T_VALIDATE)
                return Token(keyword->token);

            if (!isNCNameStart(current()))
            {
                setState(Operator);
                return id;
            }

            const Token id2(tokenizeNCName());
            const TokenMap *const keyword2 = lookupKeyword(id2.value);

            if (!keyword2)
            {
                /* It's a syntax error. All cases of two subsequent ncnames are keywords(e.g, declarations). */
                setState(Operator);
                return id;
            }

            switch(keyword->token)
            {
                case T_DECLARE:
                {
                    switch(keyword2->token)
                    {
                        case T_VARIABLE:
                        case T_FUNCTION:
                        {
                            m_tokenStack.push(Token(keyword2->token));
                            setState(Default);
                            return Token(keyword->token);
                        }
                        case T_OPTION:
                        {
                            m_tokenStack.push(Token(keyword2->token));
                            setState(Default);
                            return Token(keyword->token);
                        }
                        case T_COPY_NAMESPACES:
                        case T_ORDERING:
                        {
                            m_tokenStack.push(Token(keyword2->token));
                            setState(NamespaceKeyword);
                            return Token(keyword->token);
                        }
                        case T_CONSTRUCTION:
                        {
                            // TODO identical to CONSTRUCTION?
                            m_tokenStack.push(Token(keyword2->token));
                            setState(Operator);
                            return Token(keyword->token);
                        }
                        case T_NAMESPACE:
                        case T_BASEURI:
                        {
                            m_tokenStack.push(Token(keyword2->token));
                            setState(NamespaceDecl);
                            return Token(keyword->token);
                        }
                        case T_BOUNDARY_SPACE:
                        {
                            m_tokenStack.push(Token(keyword2->token));
                            setState(XMLSpaceDecl);
                            return Token(keyword->token);
                        }
                        case T_DEFAULT:
                        {
                            m_tokenStack.push(Token(keyword2->token));

                            const TokenType ws2 = consumeWhitespace();
                            if (ws2 != T_SUCCESS)
                            {
                                m_tokenStack.prepend(Token(ws2));
                                return Token(keyword->token);
                            }

                            const Token id3(tokenizeNCName());

                            if (id3.type != T_NCNAME)
                            {
                                m_tokenStack.prepend(id3);
                                return Token(keyword->token);
                            }

                            const TokenMap *const keyword3 = lookupKeyword(id3.value);
                            if (!keyword3)
                            {
                                m_tokenStack.prepend(id3);
                                return Token(keyword->token);
                            }
                            else
                            {
                                m_tokenStack.prepend(Token(keyword3->token));

                                if (keyword3->token == T_ORDER)
                                    setState(Operator);
                                else
                                    setState(NamespaceDecl);
                            }

                            return Token(keyword->token);
                        }
                        default:
                        {
                            m_tokenStack.push(Token(keyword2->token));
                            setState(Default);
                            return id;
                        }
                    }
                }
                case T_XQUERY:
                {
                    m_tokenStack.push(Token(keyword2->token));

                    if (keyword2->token == T_VERSION)
                    {
                        setState(NamespaceDecl);
                        return Token(keyword->token);
                    }
                    else
                    {
                        setState(Operator);
                        return id;
                    }
                }
                case T_IMPORT:
                {
                    m_tokenStack.push(Token(keyword2->token));

                    switch(keyword2->token)
                    {
                        case T_SCHEMA:
                        case T_MODULE:
                        {
                            setState(NamespaceKeyword);
                            return Token(keyword->token);
                        }
                        default:
                        {
                            setState(Operator);
                            return id;
                        }
                    }
                }
                case T_VALIDATE:
                {
                    m_tokenStack.push(Token(keyword2->token));

                    switch(keyword2->token)
                    {
                        case T_LAX:
                        case T_STRICT:
                        {
                            pushState(Operator);
                            return Token(keyword->token);
                        }
                        default:
                        {
                            setState(Operator);
                            return id;
                        }
                    }
                }
                default:
                {
                    m_tokenStack.push(Token(keyword2->token));
                    setState(Operator);
                    return id;
                }
            }
        }
        case VarName:
        {
            if (peekCurrent() == '$')
                return tokenAndAdvance(T_DOLLAR);

            setState(Operator);
            return tokenizeNCNameOrQName();
        }
        case ItemType:
        {
            switch(peekCurrent())
            {
                case '(':
                    return tokenAndChangeState(T_LPAREN, KindTest);
                case '$':
                    return tokenAndChangeState(T_DOLLAR, VarName);
            }

            const Token name(tokenizeNCNameOrQName());

            if (name.hasError())
                return error();

            else if (name.type == T_QNAME)
            {
                setState(OccurrenceIndicator);
                return name;
            }
            else
            {
                const TokenMap *const keyword = lookupKeyword(name.value);

                if (keyword)
                {
                    pushState(OccurrenceIndicator);
                    return Token(keyword->token);
                }
                else
                {
                    setState(Default);
                    return name;
                }
            }
        }
        case KindTest:
        {
            switch(peekCurrent())
            {
                case ')':
                {
                    popState();
                    return tokenAndAdvance(T_RPAREN);
                }
                case '(':
                    return tokenAndAdvance(T_LPAREN);
                case ',':
                    return tokenAndAdvance(T_COMMA);
                case '*':
                    return tokenAndAdvance(T_STAR);
                case '?':
                    return tokenAndAdvance(T_QUESTION);
                case '\'':
                case '"':
                    return tokenizeStringLiteral();
            }

            const Token nc(tokenizeNCNameOrQName());
            if (nc.hasError())
                return nc;

            const TokenType ws = consumeWhitespace();
            if (ws == T_ERROR)
                return error();

            if (peekCurrent() == '(')
            {
                const TokenMap *const keyword = lookupKeyword(nc.value);
                if (keyword)
                {
                    pushState(KindTest);
                    return Token(keyword->token);
                }
                else
                    return nc;
            }
            else
                return nc;
        }
        case KindTestForPI:
        {
            switch(peekCurrent())
            {
                case ')':
                {
                    popState();
                    return tokenAndAdvance(T_RPAREN);
                }
                case '\'':
                case '"':
                    return tokenizeStringLiteral();
                default:
                    return tokenizeNCName();
            }
        }
        case OccurrenceIndicator:
        {
            switch(peekCurrent())
            {
                case '?':
                    return tokenAndChangeState(T_QUESTION, Operator);
                case '*':
                    return tokenAndChangeState(T_STAR, Operator);
                case '+':
                    return tokenAndChangeState(T_PLUS, Operator);
                default:
                {
                    setState(Operator);
                    return nextToken();
                }
            }
        }
        case XQueryVersion:
        {
            switch(peekCurrent())
            {
                case '\'':
                case '"':
                    return tokenizeStringLiteral();
                case ';':
                    return tokenAndChangeState(T_SEMI_COLON, Default);
            }

            const Token id(tokenizeNCName());

            if (id.type != T_NCNAME)
                return id;

            const TokenMap *const keyword = lookupKeyword(id.value);
            if (keyword)
                return tokenAndChangeState(keyword->token, Default);
            else
                return id;
        }
        case StartTag:
        {
            if (peekAhead(-1) == '<')
            {
                if (current().isSpace())
                    return Token(T_ERROR);
            }
            else
            {
                if (consumeRawWhitespace())
                    return Token(T_END_OF_FILE);
            }

            switch(peekCurrent())
            {
                case '/':
                {
                    if (peekAhead() == '>')
                    {
                        m_pos += 2;

                        if (m_scanOnly)
                            return Token(T_POSITION_SET);
                        else
                        {
                            popState();
                            return Token(T_QUICK_TAG_END);
                        }
                    }
                    else
                        return error();
                }
                case '>':
                {
                    if (m_scanOnly)
                        return tokenAndChangeState(T_POSITION_SET, StartTag);
                    else
                        return tokenAndChangeState(T_G_GT, ElementContent);
                }
                case '=':
                    return tokenAndAdvance(T_G_EQ);
                case '\'':
                    return tokenAndChangeState(T_APOS, AposAttributeContent);
                case '"':
                    return tokenAndChangeState(T_QUOTE, QuotAttributeContent);
                default:
                    return tokenizeNCNameOrQName();
            }
        }
        case AposAttributeContent:
        case QuotAttributeContent:
        {
            const QChar sep(state() == AposAttributeContent ? QLatin1Char('\'') : QLatin1Char('"'));
            QString result;
            result.reserve(20);

            if (m_scanOnly)
            {
                int stack = 0;
                return attributeAsRaw(sep, stack, m_pos, true, result);
            }

            Q_ASSERT(!m_scanOnly);
            while(true)
            {
                if (atEnd())
                {
                    /* In the case that the XSL-T tokenizer invokes us with
                     * default state QuotAttributeContent, we need to be able
                     * to return a single string, in case that is all we have
                     * accumulated. */
                    if (result.isEmpty())
                        return Token(T_END_OF_FILE);
                    else
                        return Token(T_STRING_LITERAL, result);
                }

                const QChar curr(current());

                if (curr == sep)
                {
                    if (m_pos + 1 == m_length)
                        return Token(T_END_OF_FILE);

                    if (m_data.at(m_pos + 1) == sep)
                    {
                        /* The quoting mechanism was used. */
                        m_pos += 2;
                        result.append(sep);
                        continue;
                    }

                    const QChar next(m_data.at(m_pos + 1));
                    if (!next.isSpace() && next != QLatin1Char('/') && next != QLatin1Char('>'))
                        return Token(T_ERROR); // i18n Space must separate attributes

                    if (result.isEmpty())
                    {
                        return tokenAndChangeState(state() == AposAttributeContent ? T_APOS : T_QUOTE,
                                                   StartTag, 1);
                    }

                    /* Don't consume the sep, but leave it so we next time return a token for it. */
                    return Token(T_STRING_LITERAL, result);
                }
                else if (curr == QLatin1Char('{'))
                {
                    if (m_pos + 1 == m_length)
                        return Token(T_END_OF_FILE);
                    else if (peekAhead() == '{')
                    {
                        ++m_pos;
                        result.append(QLatin1Char('{'));
                    }
                    else
                    {
                        if (result.isEmpty())
                        {
                            /* The Attribute Value Template appeared directly in the attribute. */
                            pushState();
                            return tokenAndChangeState(T_CURLY_LBRACE, Default);
                        }
                        else
                        {
                            /* We don't advance, keep '{' as next token. */
                            return Token(T_STRING_LITERAL, result);
                        }
                    }
                }
                else if (curr == QLatin1Char('}'))
                {
                    if (m_pos + 1 == m_length)
                        return Token(T_END_OF_FILE);
                    else if (peekAhead() == '}')
                    {
                        ++m_pos;
                        result.append(QLatin1Char('}'));
                    }
                    else
                        return Token(T_ERROR);
                }
                else if (curr == QLatin1Char('&'))
                {
                    const QString ret(tokenizeCharacterReference());
                    if (ret.isNull())
                        return Token(T_ERROR);
                    else
                        result.append(ret);
                }
                else if (curr == QLatin1Char('<'))
                    return Token(T_STRING_LITERAL, result);
                else
                {
                    /* See Extensible Markup Language (XML) 1.0 (Fourth Edition),
                     * 3.3.3 Attribute-Value Normalization.
                     *
                     * However, it is complicated a bit by that AVN is defined on top of
                     * EOL normalization and we do those two in one go here. */
                    switch(curr.unicode())
                    {
                        case 0xD:
                        {
                            if (peekAhead() == '\n')
                            {
                                result.append(QLatin1Char(' '));
                                ++m_pos;
                                break;
                            }
                            Q_FALLTHROUGH();
                        }
                        case 0xA:
                        case 0x9:
                        {
                            result.append(QLatin1Char(' '));
                            break;
                        }
                        default:
                            result.append(curr);
                    }
                }

                ++m_pos;
            }
        }
        case ElementContent:
        {
            QString result;
            result.reserve(20);

            /* Whether the text node, result, may be whitespace only. Character references
             * and CDATA sections disables that. */
            bool mayBeWS = true;

            CharacterSkips skipEOLNormalization;

            while(true)
            {
                if (atEnd())
                    return Token(T_END_OF_FILE);

                switch(peekCurrent())
                {
                    case '<':
                    {
                        if (!result.isEmpty() && peekAhead(2) != '[')
                        {
                            /* We encountered the end, and it was not a CDATA section. */
                            /* We don't advance. Next time we'll handle the <... stuff. */
                            return Token(mayBeWS ? T_STRING_LITERAL : T_NON_BOUNDARY_WS, normalizeEOL(result, skipEOLNormalization));
                        }

                        ++m_pos;
                        if (atEnd())
                            return Token(T_END_OF_FILE);

                        const QChar ahead(current());
                        if (ahead.isSpace())
                            return error();
                        else if (ahead == QLatin1Char('/'))
                        {
                            if (m_pos + 1 == m_length)
                                return Token(T_END_OF_FILE);
                            else if (m_data.at(m_pos + 1).isSpace())
                                return error();
                            else
                                return tokenAndChangeState(T_BEGIN_END_TAG, EndTag);
                        }
                        else if (isNCNameStart(ahead))
                        {
                            pushState();
                            return tokenAndChangeState(T_G_LT, StartTag, 0);
                        }
                        else if (aheadEquals("!--", 3, 0))
                        {
                            pushState();
                            m_pos += 3;
                            return tokenAndChangeState(T_COMMENT_START, XMLComment, 0);
                        }
                        else if (aheadEquals("![CDATA[", 8, 0))
                        {
                            mayBeWS = false;
                            m_pos += 8;
                            const int start = m_pos;
                            const int len = scanUntil("]]>");

                            if (len == -1)
                                return Token(T_END_OF_FILE);

                            m_pos += 2; /* Consume "]]>". Note that m_pos is on '!'. */
                            result.append(m_data.mid(start, len));
                            break;
                        }
                        else if (ahead == QLatin1Char('?'))
                        {
                            pushState();
                            return tokenAndChangeState(T_PI_START, ProcessingInstructionName);
                        }
                        else
                            return Token(T_G_LT);
                    }
                    case '&':
                    {
                        const QString ret(tokenizeCharacterReference());
                        if (ret.isNull())
                            return Token(T_ERROR);
                        else
                        {
                            skipEOLNormalization.insert(result.length());
                            result.append(ret);
                            mayBeWS = false;
                            break;
                        }
                    }
                    case '{':
                    {
                        // TODO remove this check, also below.
                        if (m_pos + 1 == m_length)
                            return Token(T_END_OF_FILE);
                        else if (peekAhead() == '{')
                        {
                            ++m_pos;
                            result.append(QLatin1Char('{'));
                        }
                        else
                        {
                            if (result.isEmpty())
                            {
                                pushState();
                                return tokenAndChangeState(T_CURLY_LBRACE, Default);
                            }
                            else
                            {
                                /* We don't advance here. */
                                return Token(mayBeWS ? T_STRING_LITERAL : T_NON_BOUNDARY_WS, normalizeEOL(result, skipEOLNormalization));
                            }
                        }
                        break;
                    }
                    case '}':
                    {
                        if (m_pos + 1 == m_length)
                            return Token(T_END_OF_FILE);
                        else if (peekAhead() == '}')
                        {
                            ++m_pos;
                            result.append(QLatin1Char('}'));
                        }
                        else
                        {
                            /* This is a parse error, and the grammar won't be able
                             * to reduce this CURLY_RBRACE. */
                            return tokenAndChangeState(T_CURLY_RBRACE, Default);
                        }
                        break;
                    }
                    case '\n':
                    {
                        /* We want to translate \r\n into \n. */
                        if (peekAhead(-1) == '\r')
                            break;
                        Q_FALLTHROUGH();
                    }
                    case '\r':
                    {
                        result.append(QLatin1Char('\n'));
                        break;
                    }
                    default:
                    {
                        result.append(current());
                        break;
                    }
                }
                ++m_pos;
            }
        }
        case ProcessingInstructionName:
        {
            const int start = m_pos;

            while(true)
            {
                ++m_pos;
                if (m_pos >= m_length)
                    return Token(T_END_OF_FILE);

                const QChar next(current());
                if (next.isSpace() || next == QLatin1Char('?'))
                {
                    return tokenAndChangeState(T_PI_TARGET, m_data.mid(start, m_pos - start),
                                               ProcessingInstructionContent);
                }
            }
        }
        case ProcessingInstructionContent:
        {
            /* Consume whitespace between the name and the content. */
            if (consumeRawWhitespace())
                return Token(T_END_OF_FILE);

            const int start = m_pos;
            const int len = scanUntil("?>");

            if (len == -1)
                return Token(T_END_OF_FILE);
            else
            {
                m_pos += 2; /* Consume "?>" */
                popState();
                return Token(T_PI_CONTENT, normalizeEOL(m_data.mid(start, len), CharacterSkips()));
            }
        }
        case EndTag:
        {
            if (consumeRawWhitespace())
                return T_END_OF_FILE;

            if (peekCurrent() == '>')
            {
                popState();
                return tokenAndAdvance(T_G_GT);
            }
            else
                return tokenizeNCNameOrQName();
        }
        case XMLComment:
        {
            const int start = m_pos;
            const int len = scanUntil("--");

            if (len == -1)
                return T_END_OF_FILE;
            else
            {
                m_pos += 2; /* Consume "--". */
                popState();

                if (peekCurrent() == '>')
                {
                    ++m_pos;
                    return Token(T_COMMENT_CONTENT, normalizeEOL(m_data.mid(start, len), CharacterSkips()));
                }
                else
                    return error();
            }
        }
        case Pragma:
        {
            /* Consume whitespace. */
            if (consumeRawWhitespace())
                return Token(T_END_OF_FILE);

            setState(PragmaContent);
            return tokenizeNCNameOrQName();
        }
        case PragmaContent:
        {
            QString result;
            result.reserve(20);

            const bool hasWS = m_pos < m_length && current().isSpace();

            /* Consume all whitespace up to the pragma content(if any). */
            if (consumeRawWhitespace())
                return Token(T_END_OF_FILE);

            if (peekCurrent() == '#' && peekAhead() == ')')
            {
                /* We reached the end, and there's no pragma content. */
                return tokenAndChangeState(T_PRAGMA_END, Default, 2);
            }
            else if (!hasWS)
            {
                /* A separating space is required if there's pragma content. */
                return error(); /* i18n */
            }

            const int start = m_pos;
            const int len = scanUntil("#)");
            if (len == -1)
                return Token(T_END_OF_FILE);

            return Token(T_STRING_LITERAL, m_data.mid(start, len));
            Q_ASSERT(false);
        }
    }

    Q_ASSERT(false);
    return error();
}

Tokenizer::Token XQueryTokenizer::attributeAsRaw(const QChar sep,
                                                 int &sepStack,
                                                 const int startPos,
                                                 const bool aInLiteral,
                                                 QString &result)
{
    bool inLiteral = aInLiteral;
    const char otherSep = (sep == QLatin1Char('"') ? '\'' : '"');

    while(true)
    {
        if (atEnd())
            return T_END_OF_FILE;

        if (peekCurrent() == sep.unicode())
        {
            if (inLiteral)
                inLiteral = false;
            else
                inLiteral = true;

            if (peekAhead() == sep.unicode())
            {
                /* The quoting mechanism was used. */
                result.append(current());
                m_pos += 2;
                continue;
            }
            else
            {
                /* Don't consume the separator, such that we
                 * return a token for it next time. */
                if (m_pos == startPos)
                {
                    ++m_pos;
                    setState(StartTag);
                    return Token(sep == QLatin1Char('"') ? T_QUOTE : T_APOS);
                }


                if (sepStack == 0)
                {
                    return Token(T_STRING_LITERAL, result);
                }
                else
                {
                    result.append(current());
                    ++m_pos;
                    continue;
                }
            }
        }
        else if (peekCurrent() == '&')
        {
            const QString ret(tokenizeCharacterReference());
            if (ret.isNull())
                return Token(T_ERROR);
            else
            {
                result.append(ret);
                ++m_pos;
                continue;
            }
        }
        else if (peekCurrent() == otherSep)
        {
            result.append(current());
            ++m_pos;

            if (peekCurrent() == otherSep)
                ++m_pos;

            if (inLiteral)
                inLiteral = false;
            else
                inLiteral = true;

            continue;
        }
        else if (peekCurrent() == '{')
        {
            result.append(current());

            if (peekAhead() == '{')
            {
                m_pos += 2;
                continue;
            }
            else
            {
                ++m_pos;
                ++sepStack;
                const Token t(attributeAsRaw(sep, sepStack, startPos, false, result));
                if (t.type != T_SUCCESS)
                    return t;
            }

        }
        else if (peekCurrent() == '}')
        {
            if (inLiteral && peekAhead() == '}')
            {
                result.append(current());
                m_pos += 2;
                continue;
            }
            else
            {
                ++m_pos;
                --sepStack;
                return Token(T_SUCCESS); /* The return value is arbitrary. */
            }
        }
        else
        {
            result.append(current());
            ++m_pos;
        }
    }
}

Tokenizer::Token XQueryTokenizer::nextToken(XPATHLTYPE *const sourceLocator)
{
    sourceLocator->first_line = m_line;
    sourceLocator->first_column = m_pos - m_columnOffset + 1; /* Plus 1, since m_pos is 0-based. */

    if (m_tokenStack.isEmpty())
        return nextToken();
    else
    {
        const Token retval(m_tokenStack.pop());

        switch(retval.type)
        {
            case T_MODULE:
            case T_SCHEMA:
            case T_COPY_NAMESPACES:
            {
                setState(NamespaceKeyword);
                break;
            }
            case T_VERSION:
            {
                setState(XQueryVersion);
                break;
            }
            case T_AS:
            case T_OF:
            {
                setState(ItemType);
                break;
            }
            default:
            {
                if (isOperatorKeyword(retval.type))
                    setState(Default);

                break;
            }
        };

        return retval;
    }
}

int XQueryTokenizer::commenceScanOnly()
{
    m_scanOnly = true;
    return m_pos;
}

void XQueryTokenizer::resumeTokenizationFrom(const int pos)
{
    m_scanOnly = false;
    m_pos = pos;
}

void XQueryTokenizer::setParserContext(const ParserContext::Ptr &)
{
}

#undef handleWhitespace

} // namespace QPatternist

QT_END_NAMESPACE
