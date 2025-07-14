#include "regex_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char peek(RegexParser *parser)
{
    if (parser->pos >= parser->length)
        return '\0';
    return parser->input[parser->pos];
}

char get(RegexParser *parser)
{
    if (parser->pos >= parser->length)
        return '\0';
    return parser->input[parser->pos++];
}

bool eat(RegexParser *parser, char c)
{
    if (peek(parser) == c)
    {
        parser->pos++;
        return true;
    }
    return false;
}

NFA *regex_parse(const char *pattern, int is_literal)
{
    if (is_literal)
    {
        // Construir una concatenación de símbolos
        NFA *result = NULL;
        size_t len = strlen(pattern);
        for (size_t i = 0; i < len; i++)
        {
            NFA *symbol = nfa_symbol(pattern[i]);
            if (!result)
            {
                result = symbol;
            }
            else
            {
                NFA *temp = nfa_concat(result, symbol);
                nfa_destroy(result);
                nfa_destroy(symbol);
                result = temp;
            }
        }
        // El tipo de token se asignará en lexer_init
        return result ? result : nfa_epsilon();
    }
    RegexParser parser;
    parser.input = strdup(pattern);
    parser.pos = 0;
    parser.length = strlen(pattern);

    NFA *result = parse_union(&parser);

    // Si el NFA no tiene estados finales, marca el último como final
    if (result && result->num_final_states == 0 && result->num_states > 0)
    {
        nfa_add_final_state(result, result->states[result->num_states - 1], TOKEN_ERROR);
    }

    free(parser.input);
    return result;
}

NFA *parse_union(RegexParser *parser)
{
    NFA *left = parse_concat(parser);
    if (!left)
        return NULL;

    if (eat(parser, '|'))
    {
        NFA *right = parse_union(parser);
        if (!right)
        {
            nfa_destroy(left);
            return NULL;
        }

        NFA *result = nfa_union(left, right);
        nfa_destroy(left);
        nfa_destroy(right);
        return result;
    }

    return left;
}

NFA *parse_concat(RegexParser *parser)
{
    NFA *result = NULL;

    while (true)
    {
        char c = peek(parser);
        if (!c || c == '|' || c == ')')
            break;

        NFA *atom = parse_closure(parser);
        if (!atom)
        {
            if (result)
                nfa_destroy(result);
            return NULL;
        }

        if (!result)
        {
            result = atom;
        }
        else
        {
            NFA *temp = nfa_concat(result, atom);
            nfa_destroy(result);
            nfa_destroy(atom);
            result = temp;
        }
    }

    return result ? result : nfa_epsilon();
}

NFA *parse_closure(RegexParser *parser)
{
    NFA *atom = parse_atom(parser);
    if (!atom)
        return NULL;

    while (true)
    {
        char c = peek(parser);
        if (c == '*')
        {
            get(parser);
            NFA *temp = nfa_closure(atom);
            nfa_destroy(atom);
            atom = temp;
        }
        else if (c == '+')
        {
            get(parser);
            NFA *closure = nfa_closure(atom);
            NFA *temp = nfa_concat(atom, closure);
            nfa_destroy(atom);
            nfa_destroy(closure);
            atom = temp;
        }
        else if (c == '?')
        {
            get(parser);
            NFA *epsilon = nfa_epsilon();
            NFA *temp = nfa_union(atom, epsilon);
            nfa_destroy(atom);
            nfa_destroy(epsilon);
            atom = temp;
        }
        else
        {
            break;
        }
    }

    return atom;
}

NFA *parse_atom(RegexParser *parser)
{
    if (eat(parser, '('))
    {
        NFA *result = parse_union(parser);
        if (!eat(parser, ')'))
        {
            if (result)
                nfa_destroy(result);
            return NULL;
        }
        return result;
    }
    else if (eat(parser, '['))
    {
        // Parsear clase de caracteres
        int start = parser->pos;
        bool neg = false;
        if (peek(parser) == '^')
        {
            neg = true;
            get(parser);
        }
        NFA *result = NULL;
        while (peek(parser) && peek(parser) != ']')
        {
            char first = get(parser);
            if (peek(parser) == '-' && parser->pos + 1 < parser->length && parser->input[parser->pos + 1] != ']')
            {
                get(parser); // Consumir '-'
                char last = get(parser);
                if (first > last)
                {
                    if (result)
                        nfa_destroy(result);
                    return NULL;
                }
                NFA *range = nfa_range(first, last);
                if (!result)
                {
                    result = range;
                }
                else
                {
                    NFA *temp = nfa_union(result, range);
                    nfa_destroy(result);
                    nfa_destroy(range);
                    result = temp;
                }
            }
            else
            {
                NFA *symbol = nfa_symbol(first);
                if (!result)
                {
                    result = symbol;
                }
                else
                {
                    NFA *temp = nfa_union(result, symbol);
                    nfa_destroy(result);
                    nfa_destroy(symbol);
                    result = temp;
                }
            }
        }
        if (!eat(parser, ']'))
        {
            if (result)
                nfa_destroy(result);
            return NULL;
        }
        // Si la clase de caracteres es negada, no la soportamos por ahora
        if (neg)
        {
            printf("[ERROR] Clases de caracteres negadas ([^...]) no soportadas.\n");
            if (result)
                nfa_destroy(result);
            return NULL;
        }
        return result;
    }
    else if (eat(parser, '"'))
    {
        // Soporte para strings entre comillas dobles con escapes
        NFA *result = NULL;
        while (peek(parser) && peek(parser) != '"')
        {
            char c = get(parser);
            if (c == '\\')
            {
                if (parser->pos >= parser->length)
                {
                    if (result)
                        nfa_destroy(result);
                    return NULL;
                }
                char esc = get(parser);
                switch (esc)
                {
                case 'n':
                    c = '\n';
                    break;
                case 't':
                    c = '\t';
                    break;
                case 'r':
                    c = '\r';
                    break;
                case '"':
                    c = '"';
                    break;
                case '\\':
                    c = '\\';
                    break;
                default:
                    if (result)
                        nfa_destroy(result);
                    return NULL;
                }
            }
            NFA *symbol = nfa_symbol(c);
            if (!result)
            {
                result = symbol;
            }
            else
            {
                NFA *temp = nfa_concat(result, symbol);
                nfa_destroy(result);
                nfa_destroy(symbol);
                result = temp;
            }
        }
        if (!eat(parser, '"'))
        {
            if (result)
                nfa_destroy(result);
            return NULL;
        }
        return result ? result : nfa_epsilon();
    }
    else if (eat(parser, '.'))
    {
        return nfa_any();
    }
    else if (peek(parser))
    {
        char c = get(parser);
        if (c == '\\')
        {
            if (parser->pos >= parser->length)
            {
                return NULL;
            }
            char next = get(parser);
            return nfa_symbol(next);
        }
        return nfa_symbol(c);
    }
    return NULL;
}