#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

typedef struct cons {
  int car;
  struct cons *cdr;
} cons_t;

static cons_t nil_cons_ = { 0, NULL };

cons_t * const NIL = &nil_cons_;

cons_t *cons_new(int car, cons_t *cdr) {
  cons_t *c = (cons_t *)malloc(sizeof(cons_t));
  c->car = car;
  c->cdr = cdr;
  return c;
}

cons_t *cons_delete(cons_t *c) {
  if(c == NIL) {
    return NIL;
  }
  cons_t *cdr = c->cdr;
  free(c);
  return cdr;
}

typedef struct {
  cons_t *top;
} stack_t;

stack_t stack_new(void) {
  return (stack_t){ NIL };
}

void stack_push(stack_t *s, int x) {
  s->top = cons_new(x, s->top);
}

int stack_top(stack_t *s, bool *is_err) {
  if(s->top == NIL) {
    is_err && (*is_err = true);
    return 0;
  }
  is_err && (*is_err = false);
  return s->top->car;
}

int stack_pop(stack_t *s, bool *is_err) {
  int top = stack_top(s, is_err);
  s->top = cons_delete(s->top);
  return top;
}

void list_for_each_cons(cons_t *lst, void (* visitor) (cons_t *)) {
  for(cons_t *itr = lst, *cdr; itr != NIL; itr = cdr) {
    cdr = itr->cdr;
    visitor(itr);
  }
}

void stack_delete(stack_t *s) {
  list_for_each_cons(s->top, (void (*) (cons_t *))cons_delete);
}

static void stack_repr_cons_(cons_t *c) {
  printf("%d", c->car);
  if(c->cdr != NIL) {
    printf(", ");
  }
}

void stack_repr(stack_t *s) {
  printf("[");
  list_for_each_cons(s->top, stack_repr_cons_);
  puts("]");
}
