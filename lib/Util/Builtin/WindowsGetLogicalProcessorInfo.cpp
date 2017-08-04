// Copyright (c) 2014-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Config.hpp>
#include <Windows.h>
#include <Pothos/Plugin.hpp>
#include <Poco/NumberFormatter.h>
#include <cassert>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

//delay loaded symbols for windows backwards compatibility
BOOL DL_GetLogicalProcessorInformationEx(LOGICAL_PROCESSOR_RELATIONSHIP RelationshipType, PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX Buffer, PDWORD ReturnedLength);

/***********************************************************************
 * http://blogs.msdn.com/b/oldnewthing/archive/2013/10/28/10460793.aspx
 **********************************************************************/
template<typename T>
T *AdvanceBytes(T *p, SIZE_T cb)
{
 return reinterpret_cast<T*>(reinterpret_cast<BYTE *>(p) + cb);
}

class EnumLogicalProcessorInformation
{
public:
 EnumLogicalProcessorInformation(LOGICAL_PROCESSOR_RELATIONSHIP Relationship)
  : m_pinfoBase(nullptr), m_pinfoCurrent(nullptr), m_cbRemaining(0)
 {
  DWORD cb = 0;
  if (DL_GetLogicalProcessorInformationEx(Relationship,
                                       nullptr, &cb)) return;
  if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) return;

  m_pinfoBase =
   reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *>
                                     (LocalAlloc(LMEM_FIXED, cb));
  if (!m_pinfoBase) return;

  if (!DL_GetLogicalProcessorInformationEx(Relationship, 
                                        m_pinfoBase, &cb)) return;

  m_pinfoCurrent = m_pinfoBase;
  m_cbRemaining = cb;
 }

 ~EnumLogicalProcessorInformation() { LocalFree(m_pinfoBase); }

 void MoveNext()
 {
  if (m_pinfoCurrent) {
   m_cbRemaining -= m_pinfoCurrent->Size;
   if (m_cbRemaining) {
    m_pinfoCurrent = AdvanceBytes(m_pinfoCurrent,
                                  m_pinfoCurrent->Size);
   } else {
    m_pinfoCurrent = nullptr;
   }
  }
 }

 SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *Current()
                                         { return m_pinfoCurrent; }

private:
 SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *m_pinfoBase;
 SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *m_pinfoCurrent;
 DWORD m_cbRemaining;
};

static std::string MaskToString(KAFFINITY Mask)
{
 std::string out;
 for (int i = 0; i < sizeof(Mask) * 8; i++) {
  if (Mask & (static_cast<KAFFINITY>(1) << i)) {
    if (not out.empty()) out += ", ";
   out += std::to_string(i);
  }
 }
 return out;
}

/***********************************************************************
 * traverse the structures
 **********************************************************************/
static std::string enumerateGetLogicalProcessorInformationEx(void)
{
    json topObject(json::object());

    {
        auto &infoArray = topObject["CPU Cache"];
        for (EnumLogicalProcessorInformation enumInfo(RelationCache); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            json infoObj;
            assert(pinfo->Relationship == RelationCache);
            infoObj["Level"] = "L"+std::to_string(pinfo->Cache.Level);
            infoObj["Associativity"] = "0x"+Poco::NumberFormatter::formatHex(pinfo->Cache.Associativity);
            infoObj["LineSize"] = std::to_string(pinfo->Cache.LineSize)+" bytes";
            infoObj["CacheSize"] = std::to_string(pinfo->Cache.CacheSize)+" bytes";
            switch(pinfo->Cache.Type)
            {
            case CacheUnified: infoObj["Type"] = "Unified"; break;
            case CacheInstruction: infoObj["Type"] = "Instruction"; break;
            case CacheData: infoObj["Type"] = "Data"; break;
            case CacheTrace: infoObj["Type"] = "Trace"; break;
            }
            infoObj["GroupMask"] = MaskToString(pinfo->Cache.GroupMask.Mask);
            infoObj["Group"] = pinfo->Cache.GroupMask.Group;
            infoArray.push_back(infoObj);
        }
    }
    {
        auto &infoArray = topObject["CPU Group"];
        for (EnumLogicalProcessorInformation enumInfo(RelationGroup); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            json infoObj;
            assert(pinfo->Relationship == RelationGroup);
            infoObj["MaximumGroupCount"] = pinfo->Group.MaximumGroupCount;
            infoObj["ActiveGroupCount"] = pinfo->Group.ActiveGroupCount;
            auto &groupInfoArray = infoObj["GroupInfo"];
            for (size_t i = 0; i < pinfo->Group.ActiveGroupCount; i++)
            {
                json infoInfoObj;
                infoInfoObj["MaximumProcessorCount"] = pinfo->Group.GroupInfo[i].MaximumProcessorCount;
                infoInfoObj["ActiveProcessorCount"] = pinfo->Group.GroupInfo[i].ActiveProcessorCount;
                infoInfoObj["ActiveProcessorMask"] = MaskToString(pinfo->Group.GroupInfo[i].ActiveProcessorMask);
                groupInfoArray.push_back(infoInfoObj);
            }
            infoArray.push_back(infoObj);
        }
    }
    {
        auto &infoArray = topObject["CPU Numa Node"];
        for (EnumLogicalProcessorInformation enumInfo(RelationNumaNode); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            json infoObj;
            assert(pinfo->Relationship == RelationNumaNode);
            infoObj["NodeNumber"] = pinfo->NumaNode.NodeNumber;
            infoObj["GroupMask"] = MaskToString(pinfo->NumaNode.GroupMask.Mask);
            infoObj["Group"] = pinfo->NumaNode.GroupMask.Group;
            infoArray.push_back(infoObj);
        }
    }
    {
        auto &infoArray = topObject["CPU Processor Core"];
        for (EnumLogicalProcessorInformation enumInfo(RelationProcessorCore); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            json infoObj;
            assert(pinfo->Relationship == RelationProcessorCore);
            infoObj["SMT"] = (pinfo->Processor.Flags & LTP_PC_SMT) != 0;
            auto &groupInfoArray = infoObj["GroupInfo"];
            for (size_t i = 0; i < pinfo->Processor.GroupCount; i++)
            {
                json infoInfoObj;
                infoInfoObj["GroupMask"] = MaskToString(pinfo->Processor.GroupMask[i].Mask);
                infoInfoObj["Group"] = pinfo->Processor.GroupMask[i].Group;
                groupInfoArray.push_back(infoInfoObj);
            }
            infoArray.push_back(infoObj);
        }
    }
    {
        auto &infoArray = topObject["CPU Processor Package"];
        for (EnumLogicalProcessorInformation enumInfo(RelationProcessorPackage); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            json infoObj;
            assert(pinfo->Relationship == RelationProcessorPackage);
            auto &groupInfoArray = infoObj["GroupInfo"];
            for (size_t i = 0; i < pinfo->Processor.GroupCount; i++)
            {
                json infoInfoObj;
                infoInfoObj["GroupMask"] = MaskToString(pinfo->Processor.GroupMask[i].Mask);
                infoInfoObj["Group"] = pinfo->Processor.GroupMask[i].Group;
                groupInfoArray.push_back(infoInfoObj);
            }
            infoArray.push_back(infoObj);
        }
    }

    return topObject.dump();
}

pothos_static_block(registerWinProcInfo)
{
    Pothos::PluginRegistry::addCall(
        "/devices/windows_logical_processor/info", &enumerateGetLogicalProcessorInformationEx);
}
