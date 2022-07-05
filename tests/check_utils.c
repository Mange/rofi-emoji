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

START_TEST(test_tokenize_search_simple_query) {
  char *query = NULL;
  char *group_query = NULL;
  char *subgroup_query = NULL;

  char *input = "hello  world  ";
  tokenize_search(input, &query, &group_query, &subgroup_query);

  ck_assert_str_eq(query, "hello  world");
  ck_assert_ptr_eq(group_query, NULL);
  ck_assert_ptr_eq(subgroup_query, NULL);

  // query is a copy of the input
  query[0] = 'b';
  ck_assert_str_eq(query, "bello  world");
  ck_assert_str_eq(input, "hello  world  ");

  g_free(query);
  g_free(group_query);
  g_free(subgroup_query);
}
END_TEST

START_TEST(test_tokenize_search_empty_query) {
  char *query = NULL;
  char *group_query = NULL;
  char *subgroup_query = NULL;

  char *input = "";
  tokenize_search(input, &query, &group_query, &subgroup_query);

  ck_assert_str_eq(query, "");
  ck_assert_ptr_eq(group_query, NULL);
  ck_assert_ptr_eq(subgroup_query, NULL);

  // query is a copy of the input
  ck_assert_ptr_ne(query, input);

  g_free(query);
  g_free(group_query);
  g_free(subgroup_query);
}
END_TEST

START_TEST(test_tokenize_search_group_query) {
  char *query = NULL;
  char *group_query = NULL;
  char *subgroup_query = NULL;

  char *input = "hello @group world";
  tokenize_search(input, &query, &group_query, &subgroup_query);

  ck_assert_str_eq(query, "hello world");
  ck_assert_str_eq(group_query, "group");
  ck_assert_ptr_eq(subgroup_query, NULL);

  // query is a copy of the input
  ck_assert_ptr_ne(query, input);

  g_free(query);
  g_free(group_query);
  g_free(subgroup_query);
}
END_TEST

START_TEST(test_tokenize_search_subgroup_query) {
  char *query = NULL;
  char *group_query = NULL;
  char *subgroup_query = NULL;

  char *input = "hello #sub world";
  tokenize_search(input, &query, &group_query, &subgroup_query);

  ck_assert_str_eq(query, "hello world");
  ck_assert_ptr_eq(group_query, NULL);
  ck_assert_str_eq(subgroup_query, "sub");

  // query is a copy of the input
  ck_assert_ptr_ne(query, input);

  g_free(query);
  g_free(group_query);
  g_free(subgroup_query);
}
END_TEST

START_TEST(test_tokenize_search_complex_query) {
  char *query = NULL;
  char *group_query = NULL;
  char *subgroup_query = NULL;

  char *input = "@group unicorn #animal";
  tokenize_search(input, &query, &group_query, &subgroup_query);

  ck_assert_str_eq(query, "unicorn");
  ck_assert_str_eq(group_query, "group");
  ck_assert_str_eq(subgroup_query, "animal");

  // query is a copy of the input
  ck_assert_ptr_ne(query, input);

  g_free(query);
  g_free(group_query);
  g_free(subgroup_query);
}
END_TEST

START_TEST(test_tokenize_search_empty_filters) {
  char *query = NULL;
  char *group_query = NULL;
  char *subgroup_query = NULL;

  char *input = "@ #";
  tokenize_search(input, &query, &group_query, &subgroup_query);

  ck_assert_str_eq(query, "");
  ck_assert_ptr_eq(group_query, NULL);
  ck_assert_ptr_eq(subgroup_query, NULL);

  // query is a copy of the input
  ck_assert_ptr_ne(query, input);

  g_free(query);
  g_free(group_query);
  g_free(subgroup_query);
}
END_TEST

START_TEST(test_tokenize_search_only_group) {
  char *query = NULL;
  char *group_query = NULL;
  char *subgroup_query = NULL;

  char *input = "@hello";
  tokenize_search(input, &query, &group_query, &subgroup_query);

  ck_assert_str_eq(query, "");
  ck_assert_str_eq(group_query, "hello");
  ck_assert_ptr_eq(subgroup_query, NULL);

  // query is a copy of the input
  ck_assert_ptr_ne(query, input);

  g_free(query);
  g_free(group_query);
  g_free(subgroup_query);
}
END_TEST

START_TEST(test_tokenize_search_repeated_filters) {
  char *query = NULL;
  char *group_query = NULL;
  char *subgroup_query = NULL;

  char *input = "1 @a #x 2 #y @b 3";
  tokenize_search(input, &query, &group_query, &subgroup_query);

  ck_assert_str_eq(query, "1 2 3");
  ck_assert_str_eq(group_query, "b");
  ck_assert_str_eq(subgroup_query, "y");

  // query is a copy of the input
  ck_assert_ptr_ne(query, input);

  g_free(query);
  g_free(group_query);
  g_free(subgroup_query);
}
END_TEST

START_TEST(test_codepoint) {
  ck_assert_str_eq(codepoint("A"), "U+0041");
  ck_assert_str_eq(codepoint("🙃"), "U+1F643");
  ck_assert_str_eq(codepoint("🇸🇪"), "U+1F1F8 U+1F1EA");
}
END_TEST

Suite *utils_suite(void) {
  Suite *s;
  TCase *tc_core;
  TCase *tc_tokenize;
  TCase *tc_codepoint;

  s = suite_create("Utils");

  tc_core = tcase_create("Core");
  tcase_add_test(tc_core, test_capitalize);

  tc_tokenize = tcase_create("Tokenize");
  tcase_add_test(tc_tokenize, test_tokenize_search_simple_query);
  tcase_add_test(tc_tokenize, test_tokenize_search_empty_query);
  tcase_add_test(tc_tokenize, test_tokenize_search_group_query);
  tcase_add_test(tc_tokenize, test_tokenize_search_subgroup_query);
  tcase_add_test(tc_tokenize, test_tokenize_search_complex_query);
  tcase_add_test(tc_tokenize, test_tokenize_search_empty_filters);
  tcase_add_test(tc_tokenize, test_tokenize_search_only_group);
  tcase_add_test(tc_tokenize, test_tokenize_search_repeated_filters);

  tc_codepoint = tcase_create("Codepoint");
  tcase_add_test(tc_codepoint, test_codepoint);

  suite_add_tcase(s, tc_core);
  suite_add_tcase(s, tc_tokenize);
  suite_add_tcase(s, tc_codepoint);

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
