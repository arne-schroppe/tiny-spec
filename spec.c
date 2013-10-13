#include "spec.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

typedef struct {
	int success;
	const char *description;
} example_context;


void check(void *ctx, int is_failing, char *fail_desc, ...) {
	example_context *context = (example_context *)ctx;

	if(!is_failing) return;

	/* Only show the description of this example once */
	if(context->success != 0) {
		printf("it %s:\n", context->description);
		context->success = 0;
	}

	printf("  ");
	va_list arg_ptr;
	va_start(arg_ptr, fail_desc);
	vprintf(fail_desc, arg_ptr);
	va_end(arg_ptr);
	printf("\n");
}


int __verify_spec(char *name, spec this_spec) {
	
	int i = 0;
	example_context context = {0, 0};
	example_func current_example;
	while(1) {
		current_example = this_spec[i].ex;
		if(current_example == 0) break;

		context.success = 1;
		context.description = this_spec[i].name;
		current_example(&context);

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
