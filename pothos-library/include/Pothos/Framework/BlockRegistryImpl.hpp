//
// Framework/BlockRegistryImpl.hpp
//
// Implementation details for BlockRegistry templated functions.
//
// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0
//

#pragma once
#include <Pothos/Framework/BlockRegistry.hpp>
#include <Pothos/Callable/CallableImpl.hpp>
#include <utility> //std::forward

namespace Pothos {

    template <typename A0>
    std::shared_ptr<Block> BlockRegistry::make(const std::string &path, A0 &&arg0)
    {
        auto factory = BlockRegistry::lookup(path);
        auto block = factory.call<Block *>(std::forward<A0>(arg0));
        return std::shared_ptr<Block>(block);
    }

    template <typename A0, typename A1>
    std::shared_ptr<Block> BlockRegistry::make(const std::string &path, A0 &&arg0, A1 &&arg1)
    {
        auto factory = BlockRegistry::lookup(path);
        auto block = factory.call<Block *>(std::forward<A0>(arg0), std::forward<A1>(arg1));
        return std::shared_ptr<Block>(block);
    }

    template <typename A0, typename A1, typename A2>
    std::shared_ptr<Block> BlockRegistry::make(const std::string &path, A0 &&arg0, A1 &&arg1, A2 &&arg2)
    {
        auto factory = BlockRegistry::lookup(path);
        auto block = factory.call<Block *>(std::forward<A0>(arg0), std::forward<A1>(arg1), std::forward<A2>(arg2));
        return std::shared_ptr<Block>(block);
    }

    template <typename A0, typename A1, typename A2, typename A3>
    std::shared_ptr<Block> BlockRegistry::make(const std::string &path, A0 &&arg0, A1 &&arg1, A2 &&arg2, A3 &&arg3)
    {
        auto factory = BlockRegistry::lookup(path);
        auto block = factory.call<Block *>(std::forward<A0>(arg0), std::forward<A1>(arg1), std::forward<A2>(arg2), std::forward<A3>(arg3));
        return std::shared_ptr<Block>(block);
    }

    template <typename A0, typename A1, typename A2, typename A3, typename A4>
    std::shared_ptr<Block> BlockRegistry::make(const std::string &path, A0 &&arg0, A1 &&arg1, A2 &&arg2, A3 &&arg3, A4 &&arg4)
    {
        auto factory = BlockRegistry::lookup(path);
        auto block = factory.call<Block *>(std::forward<A0>(arg0), std::forward<A1>(arg1), std::forward<A2>(arg2), std::forward<A3>(arg3), std::forward<A4>(arg4));
        return std::shared_ptr<Block>(block);
    }

    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5>
    std::shared_ptr<Block> BlockRegistry::make(const std::string &path, A0 &&arg0, A1 &&arg1, A2 &&arg2, A3 &&arg3, A4 &&arg4, A5 &&arg5)
    {
        auto factory = BlockRegistry::lookup(path);
        auto block = factory.call<Block *>(std::forward<A0>(arg0), std::forward<A1>(arg1), std::forward<A2>(arg2), std::forward<A3>(arg3), std::forward<A4>(arg4), std::forward<A5>(arg5));
        return std::shared_ptr<Block>(block);
    }

    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
    std::shared_ptr<Block> BlockRegistry::make(const std::string &path, A0 &&arg0, A1 &&arg1, A2 &&arg2, A3 &&arg3, A4 &&arg4, A5 &&arg5, A6 &&arg6)
    {
        auto factory = BlockRegistry::lookup(path);
        auto block = factory.call<Block *>(std::forward<A0>(arg0), std::forward<A1>(arg1), std::forward<A2>(arg2), std::forward<A3>(arg3), std::forward<A4>(arg4), std::forward<A5>(arg5), std::forward<A6>(arg6));
        return std::shared_ptr<Block>(block);
    }

    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
    std::shared_ptr<Block> BlockRegistry::make(const std::string &path, A0 &&arg0, A1 &&arg1, A2 &&arg2, A3 &&arg3, A4 &&arg4, A5 &&arg5, A6 &&arg6, A7 &&arg7)
    {
        auto factory = BlockRegistry::lookup(path);
        auto block = factory.call<Block *>(std::forward<A0>(arg0), std::forward<A1>(arg1), std::forward<A2>(arg2), std::forward<A3>(arg3), std::forward<A4>(arg4), std::forward<A5>(arg5), std::forward<A6>(arg6), std::forward<A7>(arg7));
        return std::shared_ptr<Block>(block);
    }

    template <typename A0, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
    std::shared_ptr<Block> BlockRegistry::make(const std::string &path, A0 &&arg0, A1 &&arg1, A2 &&arg2, A3 &&arg3, A4 &&arg4, A5 &&arg5, A6 &&arg6, A7 &&arg7, A8 &&arg8)
    {
        auto factory = BlockRegistry::lookup(path);
        auto block = factory.call<Block *>(std::forward<A0>(arg0), std::forward<A1>(arg1), std::forward<A2>(arg2), std::forward<A3>(arg3), std::forward<A4>(arg4), std::forward<A5>(arg5), std::forward<A6>(arg6), std::forward<A7>(arg7), std::forward<A8>(arg8));
        return std::shared_ptr<Block>(block);
    }


} //namespace Pothos
