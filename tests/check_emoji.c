#include <check.h>
#include <glib.h>
#include <stdlib.h>

#include "../src/emoji.h"

START_TEST(test_new_and_free) {
  char *keywords[4] = {"kw1", "kw2 - electric buggaloo", NULL};

  // Accepts owned data
  // clang-format off
  Emoji *emoji = emoji_new(
    g_strdup("😀"),
    g_strdup("smiling"),
    g_strdup("people"),
    g_strdup("faces"),
    g_strdupv(keywords)
  );
  // clang-format on

  ck_assert_str_eq(emoji->bytes, "😀");
  ck_assert_str_eq(emoji->name, "smiling");
  ck_assert_str_eq(emoji->group, "people");
  ck_assert_str_eq(emoji->subgroup, "faces");
  ck_assert_str_eq(emoji->keywords[0], "kw1");
  ck_assert_str_eq(emoji->keywords[1], "kw2 - electric buggaloo");
  ck_assert_ptr_eq(emoji->keywords[2], NULL);

  emoji_free(emoji);
}
END_TEST

Suite *emoji_suite(void) {
  Suite *s;
  TCase *tc_model;

  s = suite_create("Emoji");
  tc_model = tcase_create("Model");

  tcase_add_test(tc_model, test_new_and_free);
  suite_add_tcase(s, tc_model);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = emoji_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
