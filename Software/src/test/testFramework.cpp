#include <stdio.h>
#include <vector>
#include <string>

#include "TestSupport.h"

#include "TestSupportTest.h"

int main()
{
    printf("Unit tests for the test framework\n\n");

    test_TestSupport_1();

    printf("Failed tests: %lu\n", failedFrameworkTests.size());
    return failedFrameworkTests.size();
}
