// Copyright (C) by Ashton Mason. See LICENSE.txt for licensing information.


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <Theron/Defines.h>

#include "TestFramework/TestManager.h"

#include "TestSuites/FeatureTestSuite.h"

#if THERON_XS
#include "TestSuites/NetworkTestSuite.h"
#endif // THERON_XS


/// Static instantiations of the test suites.
Tests::FeatureTestSuite featureTestSuite;

#if THERON_XS
Tests::NetworkTestSuite networkTestSuite;
#endif // THERON_XS


int main(int argc, char *argv[])
{
    using namespace TestFramework;

    int count(1);
    bool verbose(false);

    int index(1);
    while (index < argc)
    {
        const char *const flag(argv[index]);

        if (strcmp(flag, "-count") == 0)
        {
            if (index + 1 < argc)
            {
                count = 0;
                count = atoi(argv[index + 1]);

                if (count)
                {
                    ++index;
                }
                else
                {
                    printf("ERROR: Unrecognized integer value after flag '%s'\n", flag);
                    return 1;
                }
            }
            else
            {
                printf("ERROR: Expected integer value after flag '%s'\n", flag);
                return 1;
            }
        }
        else if (strcmp(flag, "-verbose") == 0)
        {
            verbose = true;
        }
        else
        {
            printf("ERROR: Unrecognized flag '%s'\n", flag);
            return 1;
        }

        ++index;
    }

    // Run all the tests, report the result, print any errors.
    bool allPassed(true);
    printf("Running tests %d time(s). Use -count to set count, -verbose to turn on output.\n", count);

    for (int iteration(0); iteration < count; ++iteration)
    {
        if (count > 1)
        {
            printf("Iteration %d ...\n", iteration);
        }

        allPassed = allPassed && TestManager::Instance()->RunTests(verbose);
    }

    if (!allPassed)
    {
        const TestManager::ErrorList &errors(TestManager::Instance()->GetErrors());
        printf("Tests FAILED with %d error%s\n", static_cast<int>(errors.size()), errors.size() == 1 ? "" : "s");
        return 1;
    }

    printf("PASSED\n");
    return 0;
}


