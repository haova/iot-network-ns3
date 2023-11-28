
// Include a header file from your module to test.
#include "ns3/iotnet.h"

// An essential include is test.h
#include "ns3/test.h"

// Do not put your test classes in namespace ns3.  You may find it useful
// to use the using directive to access the ns3 namespace directly
using namespace ns3;

// Add a doxygen group for tests.
// If you have more than one test, this should be in only one of them.
/**
 * \defgroup iotnet-tests Tests for iotnet
 * \ingroup iotnet
 * \ingroup tests
 */

// This is an example TestCase.
/**
 * \ingroup iotnet-tests
 * Test case for feature 1
 */
class IotnetTestCase1 : public TestCase
{
  public:
    IotnetTestCase1();
    virtual ~IotnetTestCase1();

  private:
    void DoRun() override;
};

// Add some help text to this case to describe what it is intended to test
IotnetTestCase1::IotnetTestCase1()
    : TestCase("Iotnet test case (does nothing)")
{
}

// This destructor does nothing but we include it as a reminder that
// the test case should clean up after itself
IotnetTestCase1::~IotnetTestCase1()
{
}

//
// This method is the pure virtual method from class TestCase that every
// TestCase must implement
//
void
IotnetTestCase1::DoRun()
{
    // A wide variety of test macros are available in src/core/test.h
    NS_TEST_ASSERT_MSG_EQ(true, true, "true doesn't equal true for some reason");
    // Use this one for floating point comparisons
    NS_TEST_ASSERT_MSG_EQ_TOL(0.01, 0.01, 0.001, "Numbers are not equal within tolerance");
}

// The TestSuite class names the TestSuite, identifies what type of TestSuite,
// and enables the TestCases to be run.  Typically, only the constructor for
// this class must be defined

/**
 * \ingroup iotnet-tests
 * TestSuite for module iotnet
 */
class IotnetTestSuite : public TestSuite
{
  public:
    IotnetTestSuite();
};

IotnetTestSuite::IotnetTestSuite()
    : TestSuite("iotnet", UNIT)
{
    // TestDuration for TestCase can be QUICK, EXTENSIVE or TAKES_FOREVER
    AddTestCase(new IotnetTestCase1, TestCase::QUICK);
}

// Do not forget to allocate an instance of this TestSuite
/**
 * \ingroup iotnet-tests
 * Static variable for test initialization
 */
static IotnetTestSuite siotnetTestSuite;
