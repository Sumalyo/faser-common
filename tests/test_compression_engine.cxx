#include <gtest/gtest.h>
#include "CompressionEngine/compressionlib.hpp"
/*
TODO : Replace place holder code
 - This is a sample code for implementing tests like this with Google Test
 - Add sanity testa and functional tests for the Compression Engine Code
 - Find a way to link them properly
*/
class MyClass{
public:
int Add(int a, int b)
{
    return a+b;
}
};

class MyClassTest : public ::testing::Test {
 protected:
  MyClassTest() {}
  ~MyClassTest() {}

  virtual void SetUp() {}
  virtual void TearDown() {}
};

TEST_F(MyClassTest, CanAddTwoNumbers) {
  MyClass my_class;
  int expected = 5;
  int actual = my_class.Add(2, 3);
  EXPECT_EQ(expected, actual);
    std::vector<uint8_t> testInput= {1,2,3,4,5};
    std::vector<uint8_t> testOutput;
    bool reality = CompressionUtility::zstdCompressorEvent(&testInput,testOutput,true);
    EXPECT_EQ(reality, true);
    }