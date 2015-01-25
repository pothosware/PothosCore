// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Framework.hpp>

/***********************************************************************
 * |PothosDoc Simple MAC
 *
 * This MAC is a simple implementation of a media access control layer.
 * https://en.wikipedia.org/wiki/Media_access_control
 *
 *
 *
 * |category /Packet
 * |keywords MAC PHY packet
 *
 * |factory /blocks/simple_mac()
 **********************************************************************/
class SimpleMac : public Pothos::Block
{
public:
    SimpleMac(void)
    {
        this->setupInput("phyIn");
        this->setupInput("macIn");
        this->setupOutput("phyOut");
        this->setupOutput("macOut");
    }

    static Block *make(void)
    {
        return new SimpleMac();
    }

    void work(void)
    {
        
    }
};

static Pothos::BlockRegistry registerSimpleMac(
    "/blocks/simple_mac", &SimpleMac::make);
