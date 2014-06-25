// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Label.hpp>

Pothos::Label::Label(void):
    index(0)
{
    return;
}

Pothos::LabelIteratorRange::LabelIteratorRange(void):
    _begin(nullptr), _end(nullptr)
{
    return;
}

#include <Pothos/Managed.hpp>

static auto managedLabel = Pothos::ManagedClass()
    .registerConstructor<Pothos::Label>()
    .registerConstructor<Pothos::Label, const Pothos::Object &, const unsigned long long>()
    .registerField(POTHOS_FCN_TUPLE(Pothos::Label, data))
    .registerField(POTHOS_FCN_TUPLE(Pothos::Label, index))
    .commit("Pothos/Label");

static Pothos::LabelIteratorRange::const_iterator labelIterRangeAt(const Pothos::LabelIteratorRange &iter, const size_t index)
{
    return iter.begin() + index;
}

static auto managedLabelIteratorRange = Pothos::ManagedClass()
    .registerClass<Pothos::LabelIteratorRange>()
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::LabelIteratorRange, begin))
    .registerMethod(POTHOS_FCN_TUPLE(Pothos::LabelIteratorRange, end))
    .registerMethod("at", labelIterRangeAt)
    .commit("Pothos/LabelIteratorRange");

static const Pothos::Label &labelIterDeref(const Pothos::LabelIteratorRange::const_iterator &iter)
{
    return *iter;
}

static auto managedLabelConstIterator = Pothos::ManagedClass()
    .registerClass<Pothos::LabelIteratorRange::const_iterator>()
    .registerMethod("deref", labelIterDeref)
    .commit("Pothos/LabelConstIterator");

#include <Pothos/Plugin.hpp>
#include <Pothos/Util/CompareTo.hpp>

static int compareLabelIteratorRanges(const Pothos::LabelIteratorRange::const_iterator &lhs, const Pothos::LabelIteratorRange::const_iterator &rhs)
{
    return Pothos::Util::compareTo(lhs, rhs);
}

pothos_static_block(pothosLabelIteratorRegisterCompare)
{
    Pothos::PluginRegistry::addCall("/object/compare/labeliterator_to_labeliterator", &compareLabelIteratorRanges);
}

#include <Pothos/Object/Serialize.hpp>

namespace Pothos { namespace serialization {
template <class Archive>
void serialize(Archive &ar, Pothos::Label &t, const unsigned int)
{
    ar & t.data;
    ar & t.index;
}
}}

POTHOS_OBJECT_SERIALIZE(Pothos::Label)
POTHOS_OBJECT_SERIALIZE(std::vector<Pothos::Label>)
