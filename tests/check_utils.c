#include <check.h>
#include <glib.h>
#include <stdlib.h>

#include "../src/utils.h"

START_TEST(test_capitalize) {
  char *str = g_strdup("hello world");
  char *upper = g_strdup("HELLO WORLD");

  capitalize(str);
  capitalize(upper);

  ck_assert_str_eq(str, "Hello world");
  ck_assert_str_eq(upper, "HELLO WORLD");

  g_free(str);
  g_free(upper);
}
END_TEST

Suite *utils_suite(void) {
  Suite *s;
  TCase *tc_core;

  s = suite_create("Utils");
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_capitalize);
  suite_add_tcase(s, tc_core);

  return s;
}

int main(void) {
  int number_failed;
  Suite *s;
  SRunner *sr;

  s = utils_suite();
  sr = srunner_create(s);

  srunner_run_all(sr, CK_VERBOSE);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
