// Copyright (c) 2020 Nicholas Corgan
// SPDX-License-Identifier: BSL-1.0

#include "mpIValReader.h"

class MupOctalValReader: public mup::IValueReader
{
public:
    MupOctalValReader();
    virtual ~MupOctalValReader();

    bool IsValue(const mup::char_type *a_szExpr, int &a_iPos, mup::Value &a_val) override;
    mup::IValueReader* Clone(mup::TokenReader *pTokenReader) const override;
};
