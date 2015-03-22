// Copyright (c) 2015-2015 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Exception.hpp>
#include <iostream>

#include "mpParser.h"

POTHOS_TEST_BLOCK("/util/tests", test_parser)
{
    mup::ParserX  p(mup::pckALL_COMPLEX);
    p.DefineConst("x", 12.3);
    p.DefineConst("j", std::complex<double>(0.0, 1.0));

    mup::Value m1(3, 3, 0);
    m1.At(0, 0) = 1;
    m1.At(1, 1) = 1;
    m1.At(2, 2) = 1;
    p.DefineConst("m1", m1);

    p.EnableAutoCreateVar(true);
    p.SetExpr("a=200.0");
    p.Eval();
    p.SetExpr("b=2*a");
    p.Eval();

    try
    {
        p.SetExpr("{1, 2, -3}*b*j");
        mup::Value result = p.Eval();
        std::cout << result.GetType() << std::endl;
        std::cout << result << std::endl;
    }
    catch (const mup::ParserError &ex)
    {
        std::cout << ex.GetMsg() << std::endl;
    }
    //f double
    //c complex
    //i integer
    //b bool
    //s string
    //m matrix
}
