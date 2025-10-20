1. How to setup test scenario
-Split up methods -- have blink and case-changing portions of code in their own methods with input arguments.
-Blink takes in an argument of count to determine if the LED should turn on/off.
-Have a header file where these methods can be declared and accessed via test.c to be tested.

2. How to exercise system
-For each test, have a TEST_ASSERT_TRUE_MESSAGE that indicates if it was passed and TEST_FAIL_MESSAGE if the test was failed.
-To test the case-changing code, try a couple capital letters (A,C, whatever) and see if (a,c,...) ASCII values are returned.
-To test the blink code, give counts that are a multiple of 11 (11, 22, etc) and the blink method should the same value for on. If not multiple of 11 (15, 18, etc). blink method should return the opposite value passed in for on.

3. Expected behavior
-Hopefully, all of the TEST_ASSERT_TRUE_MESSAGES should appear. I think I wrote expected behavior in the section above. 