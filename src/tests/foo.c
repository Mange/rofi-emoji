#include <glib.h>
#include <glib/gi18n.h>

/*
 * This entire file is a placeholder to get started.
 */

static void test_example() {
  g_assert(TRUE);
}

int main(int argc, char *argv[]) {
  g_test_init(&argc, &argv, NULL);

  g_test_add_func("/foo/example", test_example);

  return g_test_run();
}
