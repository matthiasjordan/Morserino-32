#include "TestSupport.h"

std::vector<const char*> failedFrameworkTests;


void logFailure(const char* msg) {
    failedFrameworkTests.push_back(msg);
    printf("%s -- %lu\n", msg, failedFrameworkTests.size());
}

void test_TestSupport_aE1()
{
    failedTests.clear();
    assertEquals("equals m1", "a", "a");
    if (failedTests.size() != 0)
    {
        logFailure("FAILED: assertEquals(m1, const char*, const char*)");
    }
}

void test_TestSupport_aE2()
{
    failedTests.clear();
    assertEquals("equals m2", "a", "b");
    if (failedTests.size() != 1)
    {
        logFailure("FAILED: assertEquals(m2, const char*, const char*)");
    }
}

void test_TestSupport_aE3()
{
    failedTests.clear();
    assertEquals("equals m3", true, true);
    if (failedTests.size() != 0)
    {
        logFailure("FAILED: assertEquals(m3, bool, bool)");
    }
}

void test_TestSupport_aE4()
{
    failedTests.clear();
    assertEquals("equals m4", true, false);
    if (failedTests.size() != 1)
    {
        logFailure("FAILED: assertEquals(m4, bool, bool)");
    }
}

void test_TestSupport_1()
{
    test_TestSupport_aE1();
    test_TestSupport_aE2();
    test_TestSupport_aE3();
    test_TestSupport_aE4();
}
