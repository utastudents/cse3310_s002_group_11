/* Example file for using µnit.
 *
 * µnit is MIT-licensed, but for this file and this file alone:
 *
 * To the extent possible under law, the author(s) of this file have
 * waived all copyright and related or neighboring rights to this
 * work.  See <https://creativecommons.org/publicdomain/zero/1.0/> for
 * details.
 *********************************************************************/

#include "munit.h"
#include "extfat.h"

#if defined(_MSC_VER)
#pragma warning(disable: 4127)
#endif

static MunitResult test_init(const MunitParameter params[], void* data)
{
  (void) params;
  (void) data;

  fileInfo inst;

  setFunction ((&inst));

  int result = initInstance(&inst);

  munit_assert(result == EXIT_SUCCESS);

  return MUNIT_OK;
}

static MunitResult test_fill_instance(const MunitParameter params[], void* data)
{
  (void) params;
  (void) data;

  char * fileName = malloc(256);
  memset(fileName, ' ', 255);
  fileName[255] = '\0';

  char* argv[] = {"extfat", "-i", "test.image", "-o", "output", "-c", fileName};
  int argc = sizeof(argv) / sizeof(char*) - 1;

  fileInfo inst;

  setFunction ((&inst));

  argv[4] = fileName;

  int result = fillInstance(&inst, argc, argv);

  free(fileName);

  munit_assert(result == EXIT_SUCCESS);

  return MUNIT_OK;
}

static MunitResult test_compare(const MunitParameter params[], void* data)
{
  (void) params;
  (void) data;

  char* argv[] = {"extfat", "-i", "test.image", "-o", "output", "-c", NULL};
  int argc = sizeof(argv) / sizeof(char*) - 1;

  fileInfo inst;

  setFunction ((&inst));

  fillInstance(&inst, argc, argv);

  int result = compareBootSec(&inst);

  munit_assert(result == EXIT_SUCCESS);

  return MUNIT_OK;
}

static MunitResult test_mmap_copy(const MunitParameter params[], void* data)
{
  (void) params;
  (void) data;

  char* argv[] = {"extfat", "-i", "test.image", "-o", "output", "-c", NULL};
  int argc = sizeof(argv) / sizeof(char*) - 1;

  fileInfo inst;

  setFunction ((&inst));

  initInstance(&inst);

  setFunction ((&inst));

  fillInstance(&inst, argc, argv);

  int result = mmapCopy(&inst);

  munit_assert(result == EXIT_SUCCESS);

  return MUNIT_OK;
}

static MunitResult test_unmap_file(const MunitParameter params[], void* data)
{
  (void) params;
  (void) data;

  fileInfo inst;

  setFunction ((&inst));

  int result = unmapFile(&inst);

  munit_assert(result == EXIT_SUCCESS);

  return MUNIT_OK;
}


static MunitTest test_suite_tests[] = {
  { (char*) "/example/init", test_init, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}, 
  { (char*) "/example/fill_instance", test_fill_instance, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}, 
  { (char*) "/example/compare", test_compare, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}, 
  { (char*) "/example/mmap_copy", test_mmap_copy, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}, 
  { (char*) "/example/unmap_file", test_unmap_file, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL}, 
  { NULL, NULL, NULL, NULL, MUNIT_TEST_OPTION_NONE, NULL }
};

static const MunitSuite test_suite = {
  (char*) "",
  test_suite_tests,
  NULL,
  1,
  MUNIT_SUITE_OPTION_NONE
};

/* This is only necessary for EXIT_SUCCESS and EXIT_FAILURE, which you
 * *should* be using but probably aren't (no, zero and non-zero don't
 * always mean success and failure).  I guess my point is that nothing
 * about µnit requires it. */
#include <stdlib.h>

int main(int argc, char* argv[MUNIT_ARRAY_PARAM(argc + 1)]) {
  return munit_suite_main(&test_suite, (void*) "µnit", argc, argv);
}
