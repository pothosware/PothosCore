// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework/Label.hpp>

Pothos::Label::Label(void):
    index(0),
    width(1)
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
    .registerConstructor<Pothos::Label, const std::string &, const Pothos::Object &, const unsigned long long>()
    .registerField(POTHOS_FCN_TUPLE(Pothos::Label, id))
    .registerField(POTHOS_FCN_TUPLE(Pothos::Label, data))
    .registerField(POTHOS_FCN_TUPLE(Pothos::Label, index))
    .registerField(POTHOS_FCN_TUPLE(Pothos::Label, width))
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

template<class Archive>
void Pothos::Label::serialize(Archive & ar, const unsigned int)
{
    ar & this->id;
    ar & this->data;
    ar & this->index;
}

template void Pothos::Label::serialize<Pothos::archive::polymorphic_iarchive>(Pothos::archive::polymorphic_iarchive &, const unsigned int);
template void Pothos::Label::serialize<Pothos::archive::polymorphic_oarchive>(Pothos::archive::polymorphic_oarchive &, const unsigned int);

POTHOS_OBJECT_SERIALIZE(Pothos::Label)
POTHOS_OBJECT_SERIALIZE(std::vector<Pothos::Label>)
