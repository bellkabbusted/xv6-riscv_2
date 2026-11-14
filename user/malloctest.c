#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define LIST_NODES 16

struct node {
  int value;
  struct node *next;
};

static int failures;

static void
check(int cond, const char *msg)
{
  if (cond) {
    printf("  PASS: %s\n", msg);
  } else {
    printf("  FAIL: %s\n", msg);
    failures++;
  }
}

static int
list_count(struct node *head) // считаем кол-во листов
{
  int c = 0;
  while (head) {
    c++;
    head = head->next;
  }
  return c;
}

static int
list_values_ok(struct node *head, int start, int step) // проверяем, что ап
{
  while (head) {
    if (head->value != start)
      return 0;
    start += step;
    head = head->next;
  }
  return 1;
}

static void
test_list(void)
{
  struct node *head = 0;
  struct node *prev;
  struct node *n;
  struct node *next;
  int i;
  int ok;

  ok = 1;
  for (i = 0; i < LIST_NODES; i++) { // добавляем узлы
    n = (struct node *)malloc(sizeof(struct node));
    if (n == 0) {
      ok = 0;
      break;
    }
    n->value = i;
    n->next = head;
    head = n;
  }
  check(ok, "allocate list nodes");
  check(list_count(head) == i, "initial node count");
  check(list_values_ok(head, LIST_NODES - 1, -1), "initial list values");

  prev = 0;
  n = head;
  while (n) { // удаляем узлы с четными значениями
    next = n->next;
    if ((n->value % 2) == 0) {
      if (prev)
        prev->next = next;
      else
        head = next;
      free(n);
    } else {
      prev = n;
    }
    n = next;
  }
  check(list_count(head) == LIST_NODES / 2, "count after removing even values");
  check(list_values_ok(head, LIST_NODES - 1, -2), "values after removal");

  ok = 1;
  for (i = 0; i < LIST_NODES / 2; i++) {
    n = (struct node *)malloc(sizeof(struct node));
    if (n == 0) {
      ok = 0;
      break;
    }
    n->value = 1000 + i;
    n->next = head;
    head = n;
  }
  check(ok, "allocate replacement nodes");
  check(list_count(head) == LIST_NODES, "count after adding nodes");

  while (head) {
    n = head->next;
    free(head);
    head = n;
  }
  check(head == 0, "list fully freed");
  printf("  list freed, break=%p\n", sbrk(0));
}

int
main(int argc, char *argv[])
{
  failures = 0;

  printf("malloctest: start, break=%p\n", sbrk(0));

  test_list();

  if (failures == 0)
    printf("malloctest: all pass\n");
  else
    printf("malloctest: %d failures\n", failures);

  exit(failures ? 1 : 0);
}
