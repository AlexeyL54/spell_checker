#include "../src/back/unistring.hpp"
#include <gtest/gtest.h>
#include <string>

using std::string;
using utf8::Unistring;

TEST(UNISTRING, assignment) {
  Unistring str1("abc");
  string str2 = "abc";
  const char *str3 = "abc";

  Unistring str_1 = str1;
  Unistring str_2 = str2;
  Unistring str_3 = str3;
  Unistring str_4 = "abc";

  EXPECT_EQ(str_1, str1);
  EXPECT_EQ(str_2, str2);
  EXPECT_EQ(str_3, str2);
  EXPECT_EQ(str_4, "abc");
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

  Unistring str_mix("жñ中😀!");
  Unistring ch_mix0 = Unistring("ж");
  Unistring ch_mix1 = Unistring("ñ");
  Unistring ch_mix2 = Unistring("中");
  Unistring ch_mix3 = Unistring("😀");
  Unistring ch_mix4 = Unistring("!");

  EXPECT_EQ(str_rus[0], ch_rus0);
  EXPECT_EQ(str_rus[1], ch_rus1);
  EXPECT_EQ(str_rus[2], ch_rus2);

  EXPECT_EQ(str_eng[0], ch_eng0);
  EXPECT_EQ(str_eng[1], ch_eng1);
  EXPECT_EQ(str_eng[2], ch_eng2);

  EXPECT_EQ(str_mix[0], ch_mix0);
  EXPECT_EQ(str_mix[1], ch_mix1);
  EXPECT_EQ(str_mix[2], ch_mix2);
  EXPECT_EQ(str_mix[3], ch_mix3);
  EXPECT_EQ(str_mix[4], ch_mix4);
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

TEST(UNISTRING, length) {
  Unistring str1 = "Рус";
  Unistring str2 = "Рус!";
  Unistring str3 = "Eng";
  Unistring str4 = "Eng!";
  Unistring str5("жñ中😀!");

  EXPECT_EQ(str1.length(), 3);
  EXPECT_EQ(str2.length(), 4);
  EXPECT_EQ(str3.length(), 3);
  EXPECT_EQ(str4.length(), 4);
  EXPECT_EQ(str5.length(), 5);
}

TEST(UNISTRING, to_lower) {
  Unistring str_h = "Рус";
  Unistring str_l = "рус";

  EXPECT_EQ(str_h.to_lower(), str_l);
}

TEST(UNISTRING, bytes_to_encode_symbol) {
  // 1 байт (ASCII)
  EXPECT_EQ(utf8::bytes_to_encode_symbol("A"), 1);
  EXPECT_EQ(utf8::bytes_to_encode_symbol("!"), 1);

  // 2 байта (Кириллица, Латиница расширенная)
  EXPECT_EQ(utf8::bytes_to_encode_symbol("ж"), 2);
  EXPECT_EQ(utf8::bytes_to_encode_symbol("ñ"), 2);

  // 3 байта (Иероглифы, Спецсимволы)
  EXPECT_EQ(utf8::bytes_to_encode_symbol("€"), 3);
  EXPECT_EQ(utf8::bytes_to_encode_symbol("中"), 3);

  // 4 байта (Эмодзи)
  EXPECT_EQ(utf8::bytes_to_encode_symbol("😀"), 4);

  // Ошибочные ситуации
  std::string invalid(1, static_cast<char>(0xFF));
  EXPECT_EQ(utf8::bytes_to_encode_symbol(invalid), 0);
}

TEST(UNISTRING, find) {
  Unistring str = "жñ中😀!";

  EXPECT_EQ(str.find("ж"), 0);
  EXPECT_EQ(str.find("ñ"), 1);
  EXPECT_EQ(str.find("中"), 2);
  EXPECT_EQ(str.find("😀"), 3);
  EXPECT_EQ(str.find("!"), 4);
}
