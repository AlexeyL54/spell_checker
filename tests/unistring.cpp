#include "../src/back/unistring.hpp"
#include <gtest/gtest.h>
#include <string>

using std::string;
using utf8::Unistring;

TEST(UNISTRING, assignment) {
  Unistring str1("abc");
  Unistring str2 = str1;

  EXPECT_EQ(str1, str2);
}

TEST(UNISTRING, indexing) {
  Unistring str_rus("Рус");
  Unistring ch_rus0 = Unistring("Р");
  Unistring ch_rus1 = Unistring("у");
  Unistring ch_rus2 = Unistring("с");

  Unistring str_eng("Eng");
  Unistring ch_eng0 = Unistring("E");
  Unistring ch_eng1 = Unistring("n");
  Unistring ch_eng2 = Unistring("g");

  EXPECT_EQ(str_rus[0], ch_rus0);
  EXPECT_EQ(str_rus[1], ch_rus1);
  EXPECT_EQ(str_rus[2], ch_rus2);

  EXPECT_EQ(str_eng[0], ch_eng0);
  EXPECT_EQ(str_eng[1], ch_eng1);
  EXPECT_EQ(str_eng[2], ch_eng2);
}

TEST(UNISTRING, equality) {
  string str1 = "abc";
  const char *str2 = "abc";

  string str3 = "ab";
  const char *str4 = "ab";

  Unistring str5(str1);
  Unistring str6("abc");
  Unistring str7("ab");

  EXPECT_EQ((str5 == str6), true);
  EXPECT_EQ((str5 == str1), true);
  EXPECT_EQ((str5 == str2), true);
  EXPECT_EQ((str5 == str7), false);
  EXPECT_EQ((str5 == str3), false);
  EXPECT_EQ((str5 == str4), false);
}
