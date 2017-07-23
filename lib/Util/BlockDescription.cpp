// Copyright (c) 2014-2017 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Util/BlockDescription.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Plugin.hpp>
#include <Poco/String.h>
#include <Poco/Format.h>
#include <Poco/Path.h>
#include <Poco/RegularExpression.h>
#include <Poco/StringTokenizer.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Types.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <fstream>
#include <vector>
#include <string>
#include <cassert>
#include <json.hpp>

using json = nlohmann::json;

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
static json exprToJSON(const std::string &expr)
{
    try
    {
        return json::parse(expr);
    }
    catch (...)
    {
        return expr;
    }
}

/*!
 * Extract an args array and kwargs object parsed from an args string
 */
static void extractArgs(const std::string &argsStr, json &args, json &kwargs)
{
    for (const auto &arg : splitCommaArgs(argsStr))
    {
        const Poco::StringTokenizer kvpTok(arg, "=", Poco::StringTokenizer::TOK_TRIM);
        const std::vector<std::string> kvp(kvpTok.begin(), kvpTok.end());
        if (kvp.size() == 2) kwargs[kvp[0]] = exprToJSON(kvp[1]);
        else args.push_back(exprToJSON(arg));
    }
}

/*!
 * Load a JSON object with an args array and kwargs object parsed from the args string
 */
static void loadArgs(
    const CodeLine &codeLine, json &obj, const std::string &argsStr,
    const std::string &argsKey = "args", const std::string &kwargsKey = "kwargs")
{
    json args, kwargs;
    try {extractArgs(argsStr, args, kwargs);}
    catch (const Pothos::Exception &ex)
    {
        throw Pothos::SyntaxException(codeLine.toString(), ex);
    }

    if (not args.empty()) obj[argsKey] = args;
    if (not kwargs.empty()) obj[kwargsKey] = kwargs;
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

            //This strips the remaining carriage return
            //left when getline uses the unix newline locale,
            //and the file is using  windows style newlines.
            if (line.back() == '\r') line.pop_back();
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
static void stripDocArray(json &in)
{
    if (in.empty()) return;
    json out;

    for (const auto &entry : in)
    {
        //dont add empty lines if the last line is empty
        const std::string line = entry;
        std::string lastLine;
        if (not out.empty()) lastLine = out.back();
        if (not lastLine.empty() or not line.empty()) out.push_back(line);
    }

    //remove trailing empty line from docs
    if (not out.empty() and out.back().get<std::string>().empty())
    {
        out.erase(out.size()-1);
    }

    in = out;
}

/***********************************************************************
 * Parse a single documentation block for markup
 **********************************************************************/
static json parseCommentBlockForMarkup(const CodeBlock &commentBlock)
{
    json topObj;
    json &params = topObj["params"];

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
            topObj["name"] = Poco::trim(line.substr(matches[2].offset, matches[2].length));
            state = "DOC";
        }
        else if (matches.empty() and state == "DOC")
        {
            topObj["docs"].push_back(line);
        }
        else if (matches.empty() and state == "PARAM")
        {
            params.back()["desc"].push_back(line);
        }
        else if (instruction == "category" and state == "DOC")
        {
            topObj["categories"].push_back(Poco::trim(payload));
        }
        else if (instruction == "keywords" and state == "DOC")
        {
            for (const auto &keyword : Poco::StringTokenizer(
                payload, " \t", Poco::StringTokenizer::TOK_TRIM | Poco::StringTokenizer::TOK_IGNORE_EMPTY))
            {
                topObj["keywords"].push_back(Poco::trim(keyword));
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
            topObj["aliases"].push_back(alias);
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

            json param;
            param["key"] = key;
            param["name"] = name;
            param["desc"].push_back(desc);
            params.push_back(param);
            state = "PARAM";
        }
        else if (instruction == "default" and state == "PARAM")
        {
            if (params.back().count("default")) throw Pothos::SyntaxException(
                "Multiple occurrence of |default for param",
                codeLine.toString());
            params.back()["default"] = payload;
        }
        else if (instruction == "units" and state == "PARAM")
        {
            if (params.back().count("units")) throw Pothos::SyntaxException(
                "Multiple occurrence of |units for param",
                codeLine.toString());
            params.back()["units"] = payload;
        }
        else if (instruction == "widget" and state == "PARAM")
        {
            if (params.back().count("widgetType")) throw Pothos::SyntaxException(
                "Multiple occurrence of |widget for param",
                codeLine.toString());
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^\\s*(\\w+)\\s*\\((.*)\\)$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |widget SpinBox(args...)",
                codeLine.toString());

            assert(fields.size() == 3);
            const std::string widgetType = Poco::trim(payload.substr(fields[1].offset, fields[1].length));
            const std::string argsStr = Poco::trim(payload.substr(fields[2].offset, fields[2].length));

            params.back()["widgetType"] = widgetType;
            loadArgs(codeLine, params.back(), argsStr, "widgetArgs", "widgetKwargs");
        }
        else if (instruction == "tab" and state == "PARAM")
        {
            if (params.back().count("tab")) throw Pothos::SyntaxException(
                "Multiple occurrence of |tab for param",
                codeLine.toString());
            params.back()["tab"] = payload;
        }
        else if (instruction == "preview" and state == "PARAM")
        {
            if (params.back().count("preview")) throw Pothos::SyntaxException(
                "Multiple occurrence of preview for param",
                codeLine.toString());
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^\\s*(\\w+)(\\s*\\((.*)\\))?$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |preview previewType(args...)",
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

            params.back()["preview"] = previewType;
            if (fields.size() == 4)
            {
                const std::string argsStr = Poco::trim(payload.substr(fields[3].offset, fields[3].length));
                loadArgs(codeLine, params.back(), argsStr, "previewArgs", "previewKwargs");
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

            json option;
            option["value"] = value;
            option["name"] = name;
            params.back()["options"].push_back(option);
        }
        else if (instruction == "factory" and (state == "DOC" or state == "PARAM"))
        {
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^\\s*(/.*)\\s*\\((.*)\\)$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |factory /registry/path(args...)",
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
            if (topObj.count("path")) throw Pothos::SyntaxException(
                "Multiple occurrence of |factory", codeLine.toString());
            topObj["path"] = path;

            //split and extract args
            loadArgs(codeLine, topObj, argsStr);

            state = "DOC";
        }
        else if ((instruction == "setter" or instruction == "initializer") and (state == "DOC" or state == "PARAM"))
        {
            Poco::RegularExpression::MatchVec fields;
            Poco::RegularExpression("^\\s*(\\w+)\\s*\\((.*)\\)$").match(payload, 0, fields);
            if (fields.empty()) throw Pothos::SyntaxException(
                "Expected |"+instruction+" setFooBar(args...)",
                codeLine.toString());

            assert(fields.size() == 3);
            const std::string callName = Poco::trim(payload.substr(fields[1].offset, fields[1].length));
            const std::string argsStr = Poco::trim(payload.substr(fields[2].offset, fields[2].length));

            //add to calls
            json call;
            call["type"] = instruction;
            call["name"] = callName;
            loadArgs(codeLine, call, argsStr);
            topObj["calls"].push_back(call);

            state = "DOC";
        }
        else if (instruction == "mode" and (state == "DOC" or state == "PARAM"))
        {
            if (topObj.count("mode")) throw Pothos::SyntaxException(
                "Multiple occurrence of |mode",
                codeLine.toString());
            topObj["mode"] = payload;
        }
    }

    //empty state means this was a regular comment block, return null
    if (state.empty()) return json();

    //cleanup docs with trimming/stripping
    stripDocArray(topObj["docs"]);
    for (auto &param : params) stripDocArray(param["desc"]);

    //remove null entries
    if (topObj["docs"].empty()) topObj.erase("docs");
    if (params.empty()) topObj.erase("params");

    //sanity check for required stuff
    if (not topObj.count("path"))
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
    std::map<std::string, json> objects;
    json array;
    std::vector<std::string> factories;
};

Pothos::Util::BlockDescriptionParser::BlockDescriptionParser(void):
    _impl(new Impl())
{
    return;
}

void Pothos::Util::BlockDescriptionParser::feedStream(std::istream &is)
{
    for (const auto &contiguousBlock : extractContiguousBlocks(is))
    {
        const auto obj = parseCommentBlockForMarkup(contiguousBlock);
        if (obj.empty()) continue;

        //store into the array of all description objects
        _impl->array.push_back(obj);

        //get a list of all paths including aliases
        std::vector<std::string> paths;
        paths.push_back(obj["path"]);
        for (const auto &alias : obj.value("aliases", json::array()))
        {
            paths.push_back(alias);
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
    if (not Poco::File(filePath).exists())
        throw Pothos::FileExistsException(filePath);

    std::ifstream inputFile(Poco::Path::expand(filePath));

    if (not inputFile)
        throw Pothos::OpenFileException(filePath);

    try
    {
        this->feedStream(inputFile);
    }
    catch (const Pothos::Exception &ex)
    {
        throw Pothos::SyntaxException("BlockDescriptionParser("+filePath+")", ex);
    }
}

std::vector<std::string> Pothos::Util::BlockDescriptionParser::listFactories(void) const
{
    return _impl->factories;
}

std::string Pothos::Util::BlockDescriptionParser::getJSONArray(const size_t indent) const
{
    return _impl->array.dump(indent);
}

std::string Pothos::Util::BlockDescriptionParser::getJSONObject(const std::string &factoryPath, const size_t indent) const
{
    const auto &obj = _impl->objects.at(factoryPath);
    return obj.dump(indent);
}
