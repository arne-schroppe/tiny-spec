#include "tiny_spec.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#include <stdio.h>

typedef struct {
	int success;
	const char *description;
} example_context;

void print_test_header(example_context *context) {
	printf("it %s:\n", context->description);
}

void check(void *ctx, int is_failing, char *fail_desc, ...) {
	example_context *context = (example_context *)ctx;

	if(!is_failing) return;

	/* Only show the description of this example once */
	if(context->success != 0) {
		print_test_header(context);
		context->success = 0;
	}

	printf("  ");
	va_list arg_ptr;
	va_start(arg_ptr, fail_desc);
	vprintf(fail_desc, arg_ptr);
	va_end(arg_ptr);
	printf("\n");
}

static jmp_buf exception_env;
static int caught_sigsegv;
static int caught_sigfpe;
static int caught_sigill;
static int caught_sigbus;
void handle_signal(int sig) {
	switch(sig) {
		case SIGSEGV:
			caught_sigsegv = 1;
			break;
		case SIGFPE:
			caught_sigfpe = 1;
			break;
		case SIGILL:
			caught_sigill = 1;
			break;
    case SIGBUS:
      caught_sigbus = 1;
      break;
	}
	longjmp(exception_env, 1);
}


void process_signals(example_context *context) {

	if(!(caught_sigsegv || caught_sigill || caught_sigfpe || caught_sigbus)) {
		return;
	}

	if(context->success == 1) {
		print_test_header(context);
	}

	if(caught_sigsegv) {
		printf("  Signal: Segmentation Fault\n");
	}

	if(caught_sigfpe) {
		printf("  Signal: Floating Point Error\n");
	}

	if(caught_sigill) {
		printf("  Signal: Illegal Instruction\n");
	}

	if(caught_sigbus) {
		printf("  Signal: Bus error\n");
	}
}


int __verify_spec(char *name, spec spec) {
	
  example *this_spec = (example *)spec;
	int i = 0;
	example_context context = {0, 0};
	example_func current_example;

	while(1) {
		current_example = this_spec[i].ex;
		if(current_example == 0) break;

		signal(SIGSEGV, handle_signal);
		signal(SIGFPE, handle_signal);
		signal(SIGILL, handle_signal);
		signal(SIGBUS, handle_signal);

		context.success = 1;
		context.description = this_spec[i].name;
		caught_sigsegv = caught_sigfpe = caught_sigill = caught_sigbus = 0;

		if( setjmp(exception_env) ) {
			process_signals(&context);
			return 0;
		}
		else {
			current_example(&context);
		}

		++i;
	}

	return 0;
}



/* Built-in checks */

void __is_true(void *context, const char *desc, const void *subject) {
	check(context, !subject, "%s is not true", desc);
}

void __is_false(void *context, const char *desc, const void *subject) {
	check(context, !!subject, "%s is not false", desc);
}

void __is_null(void *context, const char *desc, const void *subject) {
	check(context, subject != NULL, "%s is not NULL", desc);
}

void __is_equal(void *context, const char *desc1, const void *subject1, const char *desc2, const void *subject2) {
	check(context, subject1 != subject2, "%s does not equal %s", desc1, desc2);
}

void __is_not_equal(void *context, const char *desc1, const void *subject1, const char *desc2, const void *subject2) {
	check(context, subject1 == subject2, "%s equals %s (but shouldn't)", desc1, desc2);
}

void __string_is_equal(void *context, const char *desc1, const char *subject1, const char *desc2, const char *subject2) {
	/* TODO only show desc if it differs from subject */
	subject1 = subject1 == NULL ? "NULL" : subject1;
	subject2 = subject2 == NULL ? "NULL" : subject2;
	/* TODO improve output. NULL should not be surrounded with quotes */
	check(context, strcmp(subject1, subject2) != 0, "\"%s\" (%s) does not equal \"%s\" (%s)", subject1, desc1, subject2, desc2);
}
