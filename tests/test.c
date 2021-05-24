#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <subhook.h>
#include <stdlib.h>

typedef void (*foo_func_t)(void);

subhook_t foo_hook;

#ifdef SUBHOOK_X86
  #if defined SUBHOOK_WINDOWS
    #define FOO_CALL __cdecl
  #elif defined SUBHOOK_UNIX
    #define FOO_CALL __attribute__((cdecl))
  #endif
#endif
#ifndef FOO_CALL
  #define FOO_CALL
#endif

int subhook_remove_w_check() {
  if (subhook_remove(foo_hook) < 0) { // assume global foo_hook is present
    puts("subhook_remove_w_check: Remove failed");
    return EXIT_FAILURE;
  }
  else {
    puts("subhook_remove_w_check: SUCCESSFUL");
    return 0;
  }
}

extern void FOO_CALL foo(void);
foo_func_t foo_tr = NULL;

void foo_hooked(void) {
  puts("foo_hooked() called");
  subhook_remove_w_check();

  printf("foo_hooked: before calling foo()\n");
  foo();
  printf("foo_hooked: after foo() called\n");

  /* Install the hook back to intercept further calls. */
  subhook_install(foo_hook);
  puts("foo_hooked: after subhook_install(). returning.");
}

void foo(void) {
  int a, b, c;
  puts("foo() called");
  a = random();
  b = (random() + a) % (random() & 0xff);
  c = a + b;
  printf("foo: value of c = %d\n", c);
}

void foo_hooked_tr(void) {
  puts("foo_hooked_tr() called. will next call foo_tr() [should actually call 'foo'] ");
  foo_tr();
}

int main() {
#if 0
  puts("Testing initial install");

  foo_hook = subhook_new((void *)foo,
                                   (void *)foo_hooked,
                                   SUBHOOK_64BIT_OFFSET);
  if (foo_hook == NULL || subhook_install(foo_hook) < 0) {
    puts("Install failed");
    return EXIT_FAILURE;
  }
  printf("before calling 'foo' (should call 'foo_hook')\n");
  foo();
  if (subhook_remove(foo_hook) < 0) {
    puts("Remove failed");
    return EXIT_FAILURE;
  }
  printf("before calling 'foo' (should call 'foo')\n");
  foo();

  puts("\nTesting re-install");

  if (subhook_install(foo_hook) < 0) {
    puts("Install failed");
    return EXIT_FAILURE;
  }
  foo();
  if (subhook_remove(foo_hook) < 0) {
    puts("Remove failed");
    return EXIT_FAILURE;
  }
  foo();

  subhook_free(foo_hook);
#else

  puts("Testing trampoline");

  subhook_t foo_hook_tr = subhook_new((void *)foo,
                                      (void *)foo_hooked_tr,
                                      SUBHOOK_64BIT_OFFSET);
  printf("foo_hook_tr %p\n", foo_hook_tr);
  if (subhook_install(foo_hook_tr) < 0) {
    puts("Install failed");
    return EXIT_FAILURE;
  }

  // foo_tr, the trampoline function, is created on the fly
  foo_tr = (foo_func_t)subhook_get_trampoline(foo_hook_tr);
  if (foo_tr == NULL) {
    puts("Failed to build trampoline");
    return EXIT_FAILURE;
  }
  foo();

  subhook_remove(foo_hook_tr);
  subhook_free(foo_hook_tr);
  foo();

#endif

  return EXIT_SUCCESS;
}
