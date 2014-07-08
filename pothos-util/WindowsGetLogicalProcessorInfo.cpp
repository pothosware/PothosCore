// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Plugin.hpp>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/NumberFormatter.h>
#include <Windows.h>
#include <cassert>
#include <iostream>

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
  if (GetLogicalProcessorInformationEx(Relationship,
                                       nullptr, &cb)) return;
  if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) return;

  m_pinfoBase =
   reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX *>
                                     (LocalAlloc(LMEM_FIXED, cb));
  if (!m_pinfoBase) return;

  if (!GetLogicalProcessorInformationEx(Relationship, 
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

std::string MaskToString(KAFFINITY Mask)
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
static Poco::JSON::Object::Ptr enumerateGetLogicalProcessorInformationEx(void)
{
    Poco::JSON::Object::Ptr topObject = new Poco::JSON::Object();

    {
        Poco::JSON::Array::Ptr infoArray = new Poco::JSON::Array();
        topObject->set("CPU Cache", infoArray);
        for (EnumLogicalProcessorInformation enumInfo(RelationCache); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            Poco::JSON::Object::Ptr infoObj = new Poco::JSON::Object();
            infoArray->add(infoObj);
            assert(pinfo->Relationship == RelationCache);
            infoObj->set("Level", "L"+std::to_string(pinfo->Cache.Level));
            infoObj->set("Associativity", "0x"+Poco::NumberFormatter::formatHex(pinfo->Cache.Associativity));
            infoObj->set("LineSize", std::to_string(pinfo->Cache.LineSize)+" bytes");
            infoObj->set("CacheSize", std::to_string(pinfo->Cache.CacheSize)+" bytes");
            switch(pinfo->Cache.Type)
            {
            case CacheUnified: infoObj->set("Type", "Unified"); break;
            case CacheInstruction:  infoObj->set("Type", "Instruction"); break;
            case CacheData: infoObj->set("Type", "Data"); break;
            case CacheTrace: infoObj->set("Type", "Trace"); break;
            }
            infoObj->set("GroupMask", MaskToString(pinfo->Cache.GroupMask.Mask));
            infoObj->set("Group", pinfo->Cache.GroupMask.Group);
        }
    }
    {
        Poco::JSON::Array::Ptr infoArray = new Poco::JSON::Array();
        topObject->set("CPU Group", infoArray);
        for (EnumLogicalProcessorInformation enumInfo(RelationGroup); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            Poco::JSON::Object::Ptr infoObj = new Poco::JSON::Object();
            infoArray->add(infoObj);
            assert(pinfo->Relationship == RelationGroup);
            infoObj->set("MaximumGroupCount", pinfo->Group.MaximumGroupCount);
            infoObj->set("ActiveGroupCount", pinfo->Group.ActiveGroupCount);
            Poco::JSON::Array::Ptr groupInfoArray = new Poco::JSON::Array();
            infoObj->set("GroupInfo", groupInfoArray);
            for (size_t i = 0; i < pinfo->Group.ActiveGroupCount; i++)
            {
                Poco::JSON::Object::Ptr infoInfoObj = new Poco::JSON::Object();
                groupInfoArray->add(infoInfoObj);
                infoInfoObj->set("MaximumProcessorCount", pinfo->Group.GroupInfo[i].MaximumProcessorCount);
                infoInfoObj->set("ActiveProcessorCount", pinfo->Group.GroupInfo[i].ActiveProcessorCount);
                infoInfoObj->set("ActiveProcessorMask", MaskToString(pinfo->Group.GroupInfo[i].ActiveProcessorMask));
            }
        }
    }
    {
        Poco::JSON::Array::Ptr infoArray = new Poco::JSON::Array();
        topObject->set("CPU Numa Node", infoArray);
        for (EnumLogicalProcessorInformation enumInfo(RelationNumaNode); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            Poco::JSON::Object::Ptr infoObj = new Poco::JSON::Object();
            infoArray->add(infoObj);
            assert(pinfo->Relationship == RelationNumaNode);
            infoObj->set("NodeNumber", pinfo->NumaNode.NodeNumber);
            infoObj->set("GroupMask", MaskToString(pinfo->NumaNode.GroupMask.Mask));
            infoObj->set("Group", pinfo->NumaNode.GroupMask.Group);
        }
    }
    {
        Poco::JSON::Array::Ptr infoArray = new Poco::JSON::Array();
        topObject->set("CPU Processor Core", infoArray);
        for (EnumLogicalProcessorInformation enumInfo(RelationProcessorCore); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            Poco::JSON::Object::Ptr infoObj = new Poco::JSON::Object();
            infoArray->add(infoObj);
            assert(pinfo->Relationship == RelationProcessorCore);
            infoObj->set("SMT", (pinfo->Processor.Flags & LTP_PC_SMT) != 0);
            Poco::JSON::Array::Ptr groupInfoArray = new Poco::JSON::Array();
            infoObj->set("GroupInfo", groupInfoArray);
            for (size_t i = 0; i < pinfo->Processor.GroupCount; i++)
            {
                Poco::JSON::Object::Ptr infoInfoObj = new Poco::JSON::Object();
                groupInfoArray->add(infoInfoObj);
                infoInfoObj->set("GroupMask", MaskToString(pinfo->Processor.GroupMask[i].Mask));
                infoInfoObj->set("Group", pinfo->Processor.GroupMask[i].Group);
            }
        }
    }
    {
        Poco::JSON::Array::Ptr infoArray = new Poco::JSON::Array();
        topObject->set("CPU Processor Package", infoArray);
        for (EnumLogicalProcessorInformation enumInfo(RelationProcessorPackage); auto pinfo = enumInfo.Current(); enumInfo.MoveNext())
        {
            Poco::JSON::Object::Ptr infoObj = new Poco::JSON::Object();
            infoArray->add(infoObj);
            assert(pinfo->Relationship == RelationProcessorPackage);
            Poco::JSON::Array::Ptr groupInfoArray = new Poco::JSON::Array();
            infoObj->set("GroupInfo", groupInfoArray);
            for (size_t i = 0; i < pinfo->Processor.GroupCount; i++)
            {
                Poco::JSON::Object::Ptr infoInfoObj = new Poco::JSON::Object();
                groupInfoArray->add(infoInfoObj);
                infoInfoObj->set("GroupMask", MaskToString(pinfo->Processor.GroupMask[i].Mask));
                infoInfoObj->set("Group", pinfo->Processor.GroupMask[i].Group);
            }
        }
    }

    return topObject;
}

pothos_static_block(registerWinProcInfo)
{
    Pothos::PluginRegistry::addCall(
        "/devices/windows_logical_processor/info", &enumerateGetLogicalProcessorInformationEx);
}
