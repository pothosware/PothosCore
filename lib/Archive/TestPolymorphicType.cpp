// Copyright (c) 2016-2016 Josh Blum
//                    2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Object.hpp>
#include <Pothos/Testing.hpp>
#include <Pothos/Archive.hpp>
#include <sstream>
#include <iostream>

namespace PothosTesting {

    struct CustomPolyBase
    {
        virtual ~CustomPolyBase(void){}
    };

    template <typename T>
    struct CustomPolyType : CustomPolyBase
    {
        CustomPolyType(void){}
        CustomPolyType(const T &value):value(value){}
        T value;
    };

} //namespace PothosTesting

namespace Pothos {
namespace serialization {

    template<class Archive>
    void serialize(Archive &, PothosTesting::CustomPolyBase &, const unsigned int){}

    template <class Archive, typename T>
    void serialize(Archive &ar, PothosTesting::CustomPolyType<T> &t, const unsigned int)
    {
        ar & t.value;
    }

} // namespace serialization
} // namespace Pothos

POTHOS_CLASS_EXPORT(PothosTesting::CustomPolyType<int>)

POTHOS_TEST_BLOCK("/archive/tests", test_polymorphic_type)
{
    PothosTesting::CustomPolyBase *x = new PothosTesting::CustomPolyType<int>(123);

    std::stringstream so;
    Pothos::Archive::OStreamArchiver ao(so);
    ao << x;

    std::stringstream si(so.str());
    PothosTesting::CustomPolyBase *y(nullptr);
    Pothos::Archive::IStreamArchiver ai(si);
    ai >> y;

    POTHOS_TEST_NOT_EQUAL(dynamic_cast<PothosTesting::CustomPolyType<int> *>(y), nullptr);

    auto xVal = dynamic_cast<PothosTesting::CustomPolyType<int> *>(x)->value;
    auto yVal = dynamic_cast<PothosTesting::CustomPolyType<int> *>(y)->value;
    POTHOS_TEST_EQUAL(xVal, yVal);

    delete x;
    delete y;
}
