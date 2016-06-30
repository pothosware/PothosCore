// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/EvalEnvironment.hpp>
#include <vector>
#include <string>
#include <stack>
#include <cctype>

std::vector<std::string> Pothos::Util::EvalEnvironment::splitExpr(const std::string &expr, const char tokenizer)
{
    std::vector<std::string> tokens;
    std::stack<char> specials;
    char previousCh = '\0';
    bool inQuotes = false;
    std::string field;

    for (const char ch : expr)
    {
        if (inQuotes)
        {
            field.push_back(ch);
            if (ch == '"' and previousCh != '\\') inQuotes = false;
        }

        else if (/*ch == ')' or */ch == '}' or ch == ']')
        {
            field.push_back(ch);
            if ((specials.top() == '(' and ch == ')') or
                (specials.top() == '{' and ch == '}') or
                (specials.top() == '[' and ch == ']')) specials.pop();

            //top level container ended
            if (specials.empty() and tokenizer == '\0')
            {
                if (not field.empty()) tokens.push_back(field);
                field.clear();
            }
        }

        else if (/*ch == '(' or */ch == '{' or ch == '[')
        {
            //top level container began
            if (specials.empty() and tokenizer == '\0')
            {
                if (not field.empty()) tokens.push_back(field);
                field.clear();
            }

            field.push_back(ch);
            specials.push(ch);
        }

        else if (ch == '"')
        {
            field.push_back(ch);
            inQuotes = true;
        }

        else if (specials.empty() and ch == tokenizer)
        {
            if (not field.empty()) tokens.push_back(field);
            field.clear();
        }

        else
        {
            if (not field.empty() or not std::isspace(ch)) field.push_back(ch);
        }

        previousCh = ch;
    }

    if (not field.empty()) tokens.push_back(field);
    return tokens;
}
