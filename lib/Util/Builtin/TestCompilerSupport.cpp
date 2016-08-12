// Copyright (c) 2014-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Exception.hpp>
#include <Pothos/Util/Compiler.hpp>
#include <fstream>
#include <iostream>

POTHOS_TEST_BLOCK("/util/tests", test_compiler)
{
    const auto compiler = Pothos::Util::Compiler::make();

    POTHOS_TEST_TRUE(compiler->test());

    //write the source to file
    const auto sourcePath = compiler->createTempFile(".cpp");
    std::ofstream outFile(sourcePath);
    outFile << "int foo(void){return 42;}" << std::endl;
    outFile.close();

    //compile the source
    Pothos::Util::CompilerArgs args;
    args.sources.push_back(sourcePath);
    auto out = compiler->compileCppModule(args);
    std::cout << out.size() << std::endl;
}
