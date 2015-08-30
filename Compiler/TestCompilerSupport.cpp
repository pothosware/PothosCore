// Copyright (c) 2014-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Util/Compiler.hpp>
#include <iostream>

POTHOS_TEST_BLOCK("/util/tests", test_compiler)
{
    const auto compiler = Pothos::Util::Compiler::make();

    Pothos::Util::CompilerArgs args;
    args.sources.push_back("int foo(void){return 42;}");
    auto out = compiler->compileCppModule(args);
    std::cout << out.size() << std::endl;
}
