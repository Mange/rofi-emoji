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

START_TEST(test_capitalize_v) {
  GStrvBuilder *builder = g_strv_builder_new();
  g_strv_builder_add(builder, "hello");
  g_strv_builder_add(builder, "nice to meet you");
  g_strv_builder_add(builder, "DEAR FRIEND");
  char **strv = g_strv_builder_end(builder);

  capitalize_v(strv);

  ck_assert_str_eq(strv[0], "Hello");
  ck_assert_str_eq(strv[1], "Nice to meet you");
  ck_assert_str_eq(strv[2], "DEAR FRIEND");

  g_strfreev(strv);
  g_strv_builder_unref(builder);

  // Handles empty list
  char **empty = malloc(sizeof(char *) * 1);
  empty[0] = NULL;
  capitalize_v(empty);
  ck_assert_ptr_eq(empty[0], NULL);
  free(empty);

  // Handles null
  char **null = NULL;
  capitalize_v(null);
  ck_assert_ptr_eq(null, NULL);
}
END_TEST

START_TEST(test_strip_strv) {
  GStrvBuilder *builder = g_strv_builder_new();
  g_strv_builder_add(builder, "  wow   ");
  g_strv_builder_add(builder, "such    space   ");
  g_strv_builder_add(builder, "\nvery cool\t  \n");
  char **strv = g_strv_builder_end(builder);

  strip_strv(strv);

  ck_assert_str_eq(strv[0], "wow");
  ck_assert_str_eq(strv[1], "such    space");
  ck_assert_str_eq(strv[2], "very cool");

  g_strfreev(strv);
  g_strv_builder_unref(builder);

  // Handles empty list
  char **empty = malloc(sizeof(char *) * 1);
  empty[0] = NULL;
  strip_strv(empty);
  ck_assert_ptr_eq(empty[0], NULL);
  free(empty);

  // Handles null
  char **null = NULL;
  strip_strv(null);
  ck_assert_ptr_eq(null, NULL);
}
END_TEST

Suite *utils_suite(void) {
  Suite *s;
  TCase *tc_core;

  s = suite_create("Utils");
  tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_capitalize);
  tcase_add_test(tc_core, test_capitalize_v);
  tcase_add_test(tc_core, test_strip_strv);
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
