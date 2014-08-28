// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Object/Serialize.hpp>
#include <Pothos/Callable.hpp>
#include <sstream>
#include <QString>

/***********************************************************************
 * conversion support for QString <-> std::string
 **********************************************************************/
pothos_static_block(QStringRegisterConvertStrings)
{
    Pothos::PluginRegistry::add("/object/convert/strings/qstring_to_string", Pothos::Callable(&QString::toStdString));
    Pothos::PluginRegistry::add("/object/convert/strings/string_to_qstring", Pothos::Callable(&QString::fromStdString));

    Pothos::PluginRegistry::add("/object/convert/strings/qstring_to_wstring", Pothos::Callable(&QString::toStdWString));
    Pothos::PluginRegistry::add("/object/convert/strings/wstring_to_qstring", Pothos::Callable(&QString::fromStdWString));
}

POTHOS_TEST_BLOCK("/widgets/tests", test_qstring_convert)
{
    Pothos::Object s0("hello");
    POTHOS_TEST_TRUE(s0.convert<QString>() == "hello");

    Pothos::Object s1(QString("world"));
    POTHOS_TEST_EQUAL(s1.convert<std::string>(), "world");
}

/***********************************************************************
 * serialization support for QString
 **********************************************************************/
namespace Pothos { namespace serialization {
template<class Archive>
void save(Archive & ar, const QString &t, const unsigned int)
{
    std::string s = t.toStdString();
    ar << s;
}

template<class Archive>
void load(Archive & ar, QString &t, const unsigned int)
{
    std::string s;
    ar >> s;
    t = QString::fromStdString(s);
}
}}

POTHOS_SERIALIZATION_SPLIT_FREE(QString)
POTHOS_OBJECT_SERIALIZE(QString)

POTHOS_TEST_BLOCK("/widgets/tests", test_qstring_serialize)
{
    QString s0("just some test string");
    Pothos::Object o0(s0);
    POTHOS_TEST_TRUE(o0);

    std::stringstream ss;
    o0.serialize(ss);
    std::cout << ss.str() << std::endl;

    Pothos::Object o1;
    o1.deserialize(ss);

    POTHOS_TEST_TRUE(o1);
    auto s1 = o1.extract<QString>();
    POTHOS_TEST_TRUE(s1 == s0);
}
