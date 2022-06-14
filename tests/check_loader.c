#include <check.h>
#include <glib.h>
#include <stdlib.h>

#include "../src/loader.h"

START_TEST(test_scan_until) {
  const char *input = "this is an example";

  const char *cursor = input;
  char *result = NULL;
  cursor = scan_until(' ', cursor, &result);

  ck_assert_str_eq(result, "this");
  ck_assert_str_eq(cursor, "is an example");

  // Result is a copy!
  result[0] = 'X';
  ck_assert_str_eq(result, "Xhis");
  ck_assert_str_eq(input, "this is an example");
  g_free(result);

  // Keep scanning!
  cursor = scan_until('x', cursor, &result);
  ck_assert_str_eq(result, "is an e");
  ck_assert_str_eq(cursor, "ample");
  g_free(result);

  // Sets NULL result on no match without advancing cursor.
  cursor = scan_until('Z', cursor, &result);
  ck_assert_ptr_eq(result, NULL);
  ck_assert_str_eq(cursor, "ample");
}
END_TEST

START_TEST(test_emoji_parse_line) {
  const char *line = "😀	 Smileys & Emotion 	face-smiling     "
                     "	grinning face 	face    | grin   \n";
  Emoji *emoji = parse_emoji_from_line(line);

  ck_assert_str_eq(emoji->bytes, "😀");
  ck_assert_str_eq(emoji->group, "Smileys & Emotion");
  ck_assert_str_eq(emoji->subgroup, "Face-smiling");
  ck_assert_str_eq(emoji->name, "Grinning face");
  ck_assert_int_eq(g_strv_length(emoji->keywords), 2);
  ck_assert_str_eq(emoji->keywords[0], "Face");
  ck_assert_str_eq(emoji->keywords[1], "Grin");
  ck_assert_ptr_eq(emoji->keywords[2], NULL);

  emoji_free(emoji);
}
END_TEST

START_TEST(test_emoji_parse_skip_redundant_keywords) {
  const char *line =
      "😀	X	X	grinning face 	face|grinning face  |grin  \n";
  Emoji *emoji = parse_emoji_from_line(line);

  // The "grinning face" keyword is removed since its the same
  // as the name.
  ck_assert_int_eq(g_strv_length(emoji->keywords), 2);
  ck_assert_str_eq(emoji->keywords[0], "Face");
  ck_assert_str_eq(emoji->keywords[1], "Grin");
  ck_assert_ptr_eq(emoji->keywords[2], NULL);

  emoji_free(emoji);
}
END_TEST

Suite *loader_suite(void) {
  Suite *s;
  TCase *tc_core;

  s = suite_create("Loader");
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_scan_until);
  tcase_add_test(tc_core, test_emoji_parse_line);
  tcase_add_test(tc_core, test_emoji_parse_skip_redundant_keywords);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = loader_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
