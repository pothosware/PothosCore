// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/BlockDescription.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/String.h>
#include <Poco/Format.h>
#include <Poco/Path.h>
#include <Poco/RegularExpression.h>
#include <Poco/StringTokenizer.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Types.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>

struct CodeLine
{
    CodeLine(const std::string &text, const size_t lineNo):
        text(text), lineNo(lineNo){}
    std::string text;
    size_t lineNo;
    std::string toString(void) const
    {
        return Poco::format("%z:%s", lineNo, text);
    }
};

typedef std::vector<CodeLine> CodeBlock;

/***********************************************************************
 * misc utils
 **********************************************************************/
static std::string titleCase(const std::string &input)
{
    if (input.empty()) return input;
    return Poco::toUpper(input.substr(0, 1)) + input.substr(1);
}

//! Split an args string into a vector of string at the comma separators
static std::vector<std::string> splitCommaArgs(const std::string &argsStr)
{
    std::vector<std::string> args(1);
    int bracketDepth = 0;
    for (char ch : argsStr)
    {
        if (ch == '{' or ch == '[' or ch == '(') bracketDepth++;
        else if (ch == '}' or ch == ']' or ch == ')') bracketDepth--;
        if (ch == ',' and bracketDepth == 0) args.push_back("");
        else args.back().push_back(ch);
        if (bracketDepth < 0) throw Pothos::SyntaxException(
            "bracket mismatch for comma-separated args", argsStr);
    }
    if (args.back().empty()) args.resize(args.size()-1);
    for (auto &arg : args) arg = Poco::trim(arg);
    return args;
}

//! Turn a simple expression into a type-specific container
static Poco::Dynamic::Var exprToDynVar(const std::string &expr)
{
    try
    {
        const auto result = Poco::JSON::Parser().parse("["+expr+"]");
        return result.extract<Poco::JSON::Array::Ptr>()->get(0);
    }
    catch (const Poco::Exception &){}
    return expr;
}

/*!
 * Extract an args array and kwargs object parsed from an args string
 */
static void extractArgs(const std::string &argsStr, Poco::JSON::Array::Ptr &args, Poco::JSON::Object::Ptr &kwargs)
{
    for (const auto &arg : splitCommaArgs(argsStr))
    {
        const Poco::StringTokenizer kvpTok(arg, "=", Poco::StringTokenizer::TOK_TRIM);
        const std::vector<std::string> kvp(kvpTok.begin(), kvpTok.end());
        if (kwargs and kvp.size() == 2) kwargs->set(kvp[0], exprToDynVar(kvp[1]));
        else if (args) args->add(exprToDynVar(arg));
    }
}

/*!
 * Load a JSON object with an args array and kwargs object parsed from the args string
 */
static void loadArgs(
    const CodeLine &codeLine, Poco::JSON::Object &obj, const std::string &argsStr,
    const std::string &argsKey = "args", const std::string &kwargsKey = "kwargs")
{
    Poco::JSON::Array::Ptr args(new Poco::JSON::Array());
    Poco::JSON::Object::Ptr kwargs(new Poco::JSON::Object());
    try {extractArgs(argsStr, args, kwargs);}
    catch (const Pothos::Exception &ex)
    {
        throw Pothos::SyntaxException(codeLine.toString(), ex);
    }

    if (args->size() > 0) obj.set(argsKey, args);
    if (kwargs->size() > 0) obj.set(kwargsKey, kwargs);
}

//! Encode backslash escaped brackets to save them from regex
static std::string bracketEscapeEncode(const std::string &payload)
{
    std::string out = payload;
    Poco::replaceInPlace(out, "\\[", "\\x5b");
    Poco::replaceInPlace(out, "\\]", "\\x5d");
    return out;
}

//! Decode escaped brackets and remove the backslash for output
static std::string bracketEscapeDecode(const std::string &payload)
{
    std::string out = payload;
    Poco::replaceInPlace(out, "\\x5b", "[");
    Poco::replaceInPlace(out, "\\x5d", "]");
    return out;
}

/***********************************************************************
 * Extract contiguous lines of comments
 **********************************************************************/
static std::vector<CodeBlock> extractContiguousBlocks(std::istream &is)
{
    std::vector<CodeBlock> contiguousCommentBlocks;
    CodeBlock currentCodeBlock;
    size_t lineNo = 0;
    std::string partial, line;

    bool inMultiLineComment = false;
    while (is.good() and not is.eof())
    {
        if (not partial.empty()) line = partial;
        else
        {
            lineNo++; //starts at 1
            std::getline(is, line);
        }
        partial.clear();
        if (line.empty()) continue;

        const std::string lineTrim = Poco::trimLeft(line);
        const auto openMulti = line.find("/*");
        const auto closeMulti = line.find("*/");
        if (not inMultiLineComment and lineTrim.size() >= 2 and lineTrim.substr(0, 2) == "//")
        {
            currentCodeBlock.push_back(CodeLine(line, lineNo));
        }
        else if (not inMultiLineComment and openMulti != std::string::npos)
        {
            inMultiLineComment = true;
            currentCodeBlock.push_back(CodeLine(line.substr(openMulti, std::string::npos), lineNo));
        }
        else if (inMultiLineComment and closeMulti != std::string::npos)
        {
            inMultiLineComment = false;
            currentCodeBlock.push_back(CodeLine(line.substr(0, closeMulti), lineNo));
            partial = line.substr(closeMulti+2);
        }
        else if (inMultiLineComment)
        {
            currentCodeBlock.push_back(CodeLine(line, lineNo));
        }
        else if (not currentCodeBlock.empty())
        {
            contiguousCommentBlocks.push_back(currentCodeBlock);
            currentCodeBlock.clear();
        }
    }

    if (not currentCodeBlock.empty())
    {
        contiguousCommentBlocks.push_back(currentCodeBlock);
    }

    return contiguousCommentBlocks;
}

/***********************************************************************
 * Strip top and bottom whitespace from a document array
 **********************************************************************/
static Poco::JSON::Array::Ptr stripDocArray(const Poco::JSON::Array::Ptr &in)
{
    Poco::JSON::Array::Ptr out(new Poco::JSON::Array());

    for (size_t i = 0; i < in->size(); i++)
    {
        //dont add empty lines if the last line is empty
        const auto line = in->getElement<std::string>(i);
        std::string lastLine;
        if (out->size() != 0) lastLine = out->getElement<std::string>(out->size()-1);
        if (not lastLine.empty() or not line.empty()) out->add(line);
    }

    //remove trailing empty line from docs
    if (out->size() != 0 and out->getElement<std::string>(out->size()-1).empty())
    {
        out->remove(out->size()-1);
    }

    return out;
}

/***********************************************************************
 * Parse a single documentation block for markup
 **********************************************************************/
static Poco::JSON::Object::Ptr parseCommentBlockForMarkup(const CodeBlock &commentBlock)
{
    Poco::JSON::Object::Ptr topObj(new Poco::JSON::Object());
    Poco::JSON::Array calls;
    Poco::JSON::Array keywords;
    Poco::JSON::Array::Ptr aliases(new Poco::JSON::Array());
    Poco::JSON::Array categories;
    Poco::JSON::Array params;
    Poco::JSON::Array::Ptr topDocs(new Poco::JSON::Array());
    Poco::JSON::Object::Ptr currentParam;

    std::string state;
    std::string indent;

    std::string instruction;
    std::string payload;

    //search for the markup begin tag and record the indent
    for (const auto &codeLine : commentBlock)
    {
        std::string line = codeLine.text;
        Poco::RegularExpression::MatchVec matches;

        if (not state.empty())
        {
            if (line.size() >= indent.size() and line.substr(0, indent.size()) != indent)
            {
                if (codeLine.lineNo == commentBlock.back().lineNo) line = "";
                else throw Pothos::SyntaxException("Inconsistent indentation", codeLine.toString());
            }

            if (line.size() >= indent.size()) line = line.substr(indent.size());
            else line = "";

            Poco::RegularExpression("^\\|(\\w+)\\s+(.*)$").match(line, 0, matches);
            if (not matches.empty())
            {
                assert(matches.size() == 3);
                instruction = line.substr(matches[1].offset, matches[1].length);
                payload = line.substr(matches[2].offset, matches[2].length);
            }
        }

        if (state.empty())
        {
            Poco::RegularExpression("^(.*)\\|PothosDoc\\s+(.*)$").match(line, 0, matches);
            if (matches.empty()) continue;
            assert(matches.size() == 3);
            indent = line.substr(matches[1].offset, matches[1].length);
            topObj->set("name", Poco::trim(line.substr(matches[2].offset, matches[2].length)));
            state = "DOC";
        }
        else if (matches.empty() and state == "DOC")
        {
            topDocs->add(line);
        }
        else if (matches.empty() and state == "PARAM")
        {
            auto array = currentParam->getArray("desc");
            array->add(line);
            currentParam->set("desc", stripDocArray(array));
        }
        else if (instruction == "category" and state == "DOC")
        {
            categories.add(Poco::trim(payload));
        }
        else if (instruction == "keywords" and state == "DOC")
        {
            for (const auto &keyword : Poco::StringTokenizer(
                payload, " \t", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY))
            {
                keywords.add(Poco::trim(keyword));
            }
        }
        else if (instruction == "alias" and state == "DOC")
        {
            const std::string alias(Poco::trim(payload));
            try {Pothos::PluginPath(alias);}
            catch (const Pothos::PluginPathError &)
            {
                throw Pothos::SyntaxException("Invalid alias path", codeLine.toString());
            }
            aliases->add(alias);
        }
        else if (instruction == "param" and (state == "DOC" or state == "PARAM"))
        {
            payload = bracketEscapeEncode(payload);
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^\\s*(\\w+)(\\s*\\[(.*)\\]\\s*)?(.*)$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |param key[name] description",
                codeLine.toString());

            assert(fields.size() == 5);
            const std::string key = bracketEscapeDecode(Poco::trim(payload.substr(fields[1].offset, fields[1].length)));
            std::string name = titleCase(key);
            if (fields[3].length != 0) name = bracketEscapeDecode(Poco::trim(payload.substr(fields[3].offset, fields[3].length)));
            const std::string desc = bracketEscapeDecode(Poco::trim(payload.substr(fields[4].offset, fields[4].length)));

            currentParam = Poco::JSON::Object::Ptr(new Poco::JSON::Object());
            params.add(currentParam);
            currentParam->set("key", key);
            currentParam->set("name", name);
            Poco::JSON::Array::Ptr descArr(new Poco::JSON::Array());
            descArr->add(desc);
            currentParam->set("desc", descArr);
            state = "PARAM";
        }
        else if (instruction == "default" and state == "PARAM")
        {
            if (currentParam->has("default")) throw Pothos::SyntaxException(
                "Multiple occurrence of |default for param",
                codeLine.toString());
            currentParam->set("default", payload);
        }
        else if (instruction == "units" and state == "PARAM")
        {
            if (currentParam->has("units")) throw Pothos::SyntaxException(
                "Multiple occurrence of |units for param",
                codeLine.toString());
            currentParam->set("units", payload);
        }
        else if (instruction == "widget" and state == "PARAM")
        {
            if (currentParam->has("widgetType")) throw Pothos::SyntaxException(
                "Multiple occurrence of |widget for param",
                codeLine.toString());
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^\\s*(\\w+)\\s*\\((.*)\\)$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |widget SpinBox(args, )",
                codeLine.toString());

            assert(fields.size() == 3);
            const std::string widgetType = Poco::trim(payload.substr(fields[1].offset, fields[1].length));
            const std::string argsStr = Poco::trim(payload.substr(fields[2].offset, fields[2].length));

            currentParam->set("widgetType", widgetType);
            loadArgs(codeLine, *currentParam, argsStr, "widgetArgs", "widgetKwargs");
        }
        else if (instruction == "tab" and state == "PARAM")
        {
            if (currentParam->has("tab")) throw Pothos::SyntaxException(
                "Multiple occurrence of |tab for param",
                codeLine.toString());
            currentParam->set("tab", payload);
        }
        else if (instruction == "preview" and state == "PARAM")
        {
            if (currentParam->has("preview")) throw Pothos::SyntaxException(
                "Multiple occurrence of preview for param",
                codeLine.toString());
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^\\s*(\\w+)(\\s*\\((.*)\\))?$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |preview previewType(args, )",
                codeLine.toString());

            assert(fields.size() == 2 or fields.size() == 4);
            const std::string previewType = Poco::trim(payload.substr(fields[1].offset, fields[1].length));

            if (previewType != "disable" and
                previewType != "enable" and
                previewType != "valid" and
                previewType != "invalid" and
                previewType != "when"
            ) throw Pothos::SyntaxException(
                "Only supports enable/disable/valid/invalid/when as value for preview option of param",
                codeLine.toString());

            currentParam->set("preview", previewType);
            if (fields.size() == 4)
            {
                const std::string argsStr = Poco::trim(payload.substr(fields[3].offset, fields[3].length));
                loadArgs(codeLine, *currentParam, argsStr, "previewArgs", "previewKwargs");
            }
        }
        else if (instruction == "option" and state == "PARAM")
        {
            payload = bracketEscapeEncode(payload);
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^(\\s*\\[(.*)\\]\\s*)?(.*)$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |option [name] value",
                codeLine.toString());

            assert(fields.size() == 4);
            const std::string value = bracketEscapeDecode(Poco::trim(payload.substr(fields[3].offset, fields[3].length)));
            std::string name = titleCase(value);
            if (fields[2].length != 0) name = bracketEscapeDecode(Poco::trim(payload.substr(fields[2].offset, fields[2].length)));

            Poco::JSON::Object option;
            option.set("value", value);
            option.set("name", name);
            if (not currentParam->has("options")) currentParam->set(
                "options", Poco::JSON::Array::Ptr(new Poco::JSON::Array()));
            currentParam->getArray("options")->add(option);
        }
        else if (instruction == "factory" and (state == "DOC" or state == "PARAM"))
        {
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^\\s*(/.*)\\s*\\((.*)\\)$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |factory /registry/path(args, )",
                codeLine.toString());

            assert(fields.size() == 3);
            const std::string path = Poco::trim(payload.substr(fields[1].offset, fields[1].length));
            const std::string argsStr = Poco::trim(payload.substr(fields[2].offset, fields[2].length));

            //add the path
            try {Pothos::PluginPath(path);}
            catch (const Pothos::PluginPathError &)
            {
                throw Pothos::SyntaxException("Invalid factory path", codeLine.toString());
            }
            if (topObj->has("path")) throw Pothos::SyntaxException(
                "Multiple occurrence of |factory", codeLine.toString());
            topObj->set("path", path);

            //split and extract args
            loadArgs(codeLine, *topObj, argsStr);

            state = "DOC";
        }
        else if ((instruction == "setter" or instruction == "initializer") and (state == "DOC" or state == "PARAM"))
        {
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^\\s*(\\w+)\\s*\\((.*)\\)$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |"+instruction+" setFooBar(args, )",
                codeLine.toString());

            assert(fields.size() == 3);
            const std::string callName = Poco::trim(payload.substr(fields[1].offset, fields[1].length));
            const std::string argsStr = Poco::trim(payload.substr(fields[2].offset, fields[2].length));

            //add to calls
            Poco::JSON::Object call;
            call.set("type", instruction);
            call.set("name", callName);
            loadArgs(codeLine, call, argsStr);
            calls.add(call);

            state = "DOC";
        }
        else if (instruction == "mode" and (state == "DOC" or state == "PARAM"))
        {
            if (topObj->has("mode")) throw Pothos::SyntaxException(
                "Multiple occurrence of |mode",
                codeLine.toString());
            topObj->set("mode", payload);
        }
    }

    topDocs = stripDocArray(topDocs);
    if (topDocs->size() > 0) topObj->set("docs", topDocs);
    if (categories.size() > 0) topObj->set("categories", categories);
    if (keywords.size() > 0) topObj->set("keywords", keywords);
    if (aliases->size() > 0) topObj->set("aliases", aliases);
    if (params.size() > 0) topObj->set("params", params);
    if (calls.size() > 0) topObj->set("calls", calls);

    //sanity check for required stuff
    if (not state.empty() and not topObj->has("path"))
    {
        throw Pothos::SyntaxException("missing |factory declaration");
    }

    return topObj;
}

/***********************************************************************
 * parser interface
 **********************************************************************/
struct Pothos::Util::BlockDescriptionParser::Impl
{
    std::map<std::string, Poco::JSON::Object::Ptr> objects;
    Poco::JSON::Array::Ptr array;
    std::vector<Pothos::PluginPath> factories;
};

Pothos::Util::BlockDescriptionParser::BlockDescriptionParser(void):
    _impl(new Impl())
{
    _impl->array = new Poco::JSON::Array();
}

void Pothos::Util::BlockDescriptionParser::feedStream(std::istream &is)
{
    for (const auto &contiguousBlock : extractContiguousBlocks(is))
    {
        const auto obj = parseCommentBlockForMarkup(contiguousBlock);

        //store into the array of all description objects
        if (obj->has("path")) _impl->array->add(obj);

        //get a list of all paths including aliases
        std::vector<std::string> paths;
        paths.push_back(obj->getValue<std::string>("path"));
        if (obj->has("aliases")) for (const auto &alias : *obj->getArray("aliases"))
        {
            paths.push_back(alias.toString());
        }

        //store mapping for each factory path
        for (const auto &path : paths)
        {
            _impl->factories.push_back(path);
            _impl->objects[path] = obj;
        }
    }
}

void Pothos::Util::BlockDescriptionParser::feedFilePath(const std::string &filePath)
{
    std::ifstream inputFile(filePath.c_str());
    this->feedStream(inputFile);
    inputFile.close();
}

std::vector<Pothos::PluginPath> Pothos::Util::BlockDescriptionParser::listFactories(void) const
{
    return _impl->factories;
}

std::string Pothos::Util::BlockDescriptionParser::getJSONArray(void) const
{
    std::stringstream ossJsonArr;
    _impl->array->stringify(ossJsonArr);
    return ossJsonArr.str();
}

std::string Pothos::Util::BlockDescriptionParser::getJSONObject(const Pothos::PluginPath &factoryPath) const
{
    const auto &obj = _impl->objects.at(factoryPath.toString());
    std::stringstream ossJsonObj;
    obj->stringify(ossJsonObj);
    return ossJsonObj.str();
}
