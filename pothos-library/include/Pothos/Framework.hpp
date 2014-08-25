///
/// \file Framework.hpp
///
/// Top level include wrapper for Framework classes.
///
/// \copyright
/// Copyright (c) 2014-2014 Josh Blum
/// SPDX-License-Identifier: BSL-1.0
///

#pragma once
#include <Pothos/Config.hpp>
#include <Pothos/Framework/WorkInfo.hpp>
#include <Pothos/Framework/DType.hpp>
#include <Pothos/Framework/Label.hpp>
#include <Pothos/Framework/InputPort.hpp>
#include <Pothos/Framework/OutputPort.hpp>
#include <Pothos/Framework/Connectable.hpp>
#include <Pothos/Framework/ConnectableImpl.hpp>
#include <Pothos/Framework/ThreadPool.hpp>
#include <Pothos/Framework/Block.hpp>
#include <Pothos/Framework/BlockImpl.hpp>
#include <Pothos/Framework/Topology.hpp>
#include <Pothos/Framework/TopologyImpl.hpp>
#include <Pothos/Framework/BlockRegistry.hpp>
#include <Pothos/Framework/BufferManager.hpp>
#include <Pothos/Framework/BufferAccumulator.hpp>
#include <Pothos/Framework/BufferChunk.hpp>
#include <Pothos/Framework/SharedBuffer.hpp>
#include <Pothos/Framework/ManagedBuffer.hpp>
#include <Pothos/Framework/Exception.hpp>
