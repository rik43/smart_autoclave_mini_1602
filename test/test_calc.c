#include <unity.h>

void setUp(void)
{
  // set stuff up here
}

void tearDown(void)
{
  // clean stuff up here
}

void test_function_calculator_addition(void)
{
  TEST_ASSERT_EQUAL(32, 25 + 7);
}


void app_main()
{
  UNITY_BEGIN();

  RUN_TEST(test_function_calculator_addition);

  UNITY_END();
}