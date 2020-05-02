// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "MupOctalValReader.hpp"

#include "mpTypes.h"

#include <iostream>

MupOctalValReader::MupOctalValReader(): mup::IValueReader() {}
MupOctalValReader::~MupOctalValReader() {}

// Copied from muParserX's hex parsing.
bool MupOctalValReader::IsValue(
    const mup::char_type *a_szExpr,
    int &a_iPos,
    mup::Value &a_val)
{
    std::size_t len = std::char_traits<mup::char_type>::length(a_szExpr);
    if (a_iPos >= (int)len || a_szExpr[a_iPos + 1] != 'o' || a_szExpr[a_iPos + 1] == 0 || a_szExpr[a_iPos] != '0')
        return 0;

    unsigned iVal(0);

    mup::stringstream_type::pos_type nPos(0);
    mup::stringstream_type ss(a_szExpr + a_iPos + 2);
    ss >> std::oct >> iVal;

    if (ss.fail())
        return false;

    if (ss.eof())
    {
        // This part sucks but tellg will return -1 if eof is set,
        // so i need a special treatment for those cases.
        for (; a_szExpr[a_iPos] != 0; ++a_iPos);
    }
    else
    {
        nPos = ss.tellg();
        assert(nPos > 0);
        a_iPos += (int)(2 + nPos);
    }

    a_val = (mup::float_type)iVal;
    return true;
}

mup::IValueReader* MupOctalValReader::Clone(mup::TokenReader *pTokenReader) const
{
    mup::IValueReader *pReader = new MupOctalValReader(*this);
    pReader->SetParent(pTokenReader);
    return pReader;
}
