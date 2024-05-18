#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define array_len(array) (sizeof(array) / sizeof((array)[0]))

#define DEFAULT_LIST_CAP 32

#define list_alloc(list)                                                       \
  do {                                                                         \
    list.capacity = DEFAULT_LIST_CAP;                                          \
    list.count = 0;                                                            \
    list.items = malloc(list.capacity * sizeof(*list.items));                  \
  } while (0)

#define list_free(list)                                                        \
  do {                                                                         \
    if (list.items != NULL) {                                                  \
      free(list.items);                                                        \
      list.items = NULL;                                                       \
    }                                                                          \
    list.count = 0;                                                            \
    list.capacity = 0;                                                         \
  } while (0)

#define list_append(list, item)                                                \
  do {                                                                         \
    if (list.count >= list.capacity) {                                         \
      list.capacity *= 2;                                                      \
      list.items = realloc(list.items, list.capacity * sizeof(*list.items));   \
    }                                                                          \
    list.items[list.count] = item;                                             \
    list.count += 1;                                                           \
  } while (0)

#define list_pop(list)                                                         \
  do {                                                                         \
    if (list.count > 0) {                                                      \
      list.count -= 1;                                                         \
      list.items[list.count] = 0;                                              \
      if (list.count < list.capacity / 3) {                                    \
        list.capacity /= 2;                                                    \
        list.items = realloc(list.items, list.capacity * sizeof(*list.items)); \
      }                                                                        \
    }                                                                          \
  } while (0)

#define list_copy(dest, src, start, count)                                     \
  do {                                                                         \
    if (start < 0) {                                                           \
      break;                                                                   \
    }                                                                          \
    size_t i = start;                                                          \
    size_t j = 0;                                                              \
    while (i < count && i < src.count && j < dest.count) {                     \
      dest.items[j] = src.items[i];                                            \
      i += 1;                                                                  \
      j += 1;                                                                  \
    }                                                                          \
  } while (0)

#define list_transfer(dest, src)                                               \
  do {                                                                         \
    if (dest.items != NULL) {                                                  \
      free(dest.items);                                                        \
      dest.items = NULL;                                                       \
    }                                                                          \
    dest.items = src.items;                                                    \
    dest.capacity = src.capacity;                                              \
    dest.count = src.count;                                                    \
  } while (0)

#define list_print(list, format)                                               \
  do {                                                                         \
    printf("[");                                                               \
    for (size_t i = 0; i < list.count; i++) {                                  \
      printf(format, list.items[i]);                                           \
      if (i < list.count - 1) {                                                \
        printf(", ");                                                          \
      }                                                                        \
    }                                                                          \
    printf("]\n");                                                             \
  } while (0)

#define list_clear(list) (list.count = 0)

const char operators[] = "+-/*%^"; 
#define operator_count ((sizeof(operators) / sizeof(operators[0])) - 1)

typedef struct {
    double *items;
    size_t count;
    size_t capacity;
} NumList;

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} OperList; 

typedef struct {
    NumList num_list;
    OperList oper_list;
} Math;

typedef struct {
    char *expr;
    Math math;
} Parser;

typedef struct {
    double *items;
    size_t count;
    size_t capacity;
} Stack;

typedef struct {
    char *items;
    size_t count;
    size_t capacity;
} Stackc;

void stack_push(Stack *stack, double item)
{
    list_append((*stack), item);
}

double stack_pop(Stack *stack)
{
    double removed = stack->items[stack->count - 1];
    list_pop((*stack));

    return removed;
}

void stackc_push(Stackc *stack, char item)
{
    list_append((*stack), item);
}

char stackc_pop(Stackc *stack)
{
    char removed = stack->items[stack->count - 1];
    list_pop((*stack));

    return removed;
}

void print_math(const Math math)
{
    printf("Math:\n");

    for (size_t i = 0; i < math.num_list.count; i++) {
        printf("    %lf \n",math.num_list.items[i]);
        if (i < math.oper_list.count) {
            printf("    %c \n", math.oper_list.items[i]);
        }
    }
    printf("\n");
}

bool str_contains(const char* str, size_t count, char item)
{
    for (size_t i = 0; i < count; i++) {
        if (str[i] == item) return true;
    }

    return false;
}

void remove_white_spaces(char* str, size_t count)
{
    size_t i = 0;
    for (size_t j = 0; j < count - 1; j++) {
        while (j < count - 1 && isspace(str[j])) {
            j++; 
        }
        str[i] = str[j];
        if (i != j) str[j] = ' ';
        i++;
    }
    memset(str + i, 0, count - (i + 1));
}

double perform_operation(double num1, double num2, char operation)
{
    switch(operation) {
        case '+': 
            return num1 + num2;
        case '-': 
            return num1 - num2;
        case '*': 
            return num1 * num2;
        case '/': 
            return num1 / num2;
        case '%': 
            return (int64_t)(num1) % (int64_t)(num2);
        case '^': 
            return pow(num1, num2);
        default:
            fprintf(stderr, "%s:%d:1 Invalid operation: '%c'", __FILE__, __LINE__, operation);
            exit(1);
    }
}

bool parse_input(char *buffer, size_t buffer_count, Math *output)
{
    remove_white_spaces(buffer, buffer_count);

    size_t count = 0;
    char *position = buffer;

    while (*position != 0) {

        // printf("Buffer:\n%s\n", buffer);

        char current = *position;

        if (isdigit(current) || current == '.') {
            count += 1;
        }

        else if (str_contains(operators, operator_count, current)) {
            if (count == 0) {
                fprintf(stderr, "Error: Entered operator before entering any number.\n");
                exit(1);
            }

            char temp[count + 1];
            memcpy(temp, buffer, count * sizeof(char));
            temp[count] = '\0';
            char *end;
            double num = strtod(temp, &end);

            list_append(output->num_list, num);
            list_append(output->oper_list, current);

            // printf("{\n");
            // printf("    Buffer : %s\n", buffer);
            // printf("    Temp   : %s\n", temp);
            // printf("    Number : %lf\n", num);
            // printf("    Current: %c\n", current);
            // printf("    Count  : %zu\n", count);
            // printf("}\n");

            buffer += count + 1;
            count = 0;
        }

        position++;
    }

    char *end;
    double num = strtod(buffer, &end);
    list_append(output->num_list, num);

    if (output->num_list.count <= 0) {
        printf("Next time enter a number.");
    }

    if (output->oper_list.count != output->num_list.count - 1) {
        fprintf(stderr, "%s:%d:1 Invalid input:", __FILE__, __LINE__);
        exit(1);
    }

    return true;
}

void parse_operations(Parser *parser, char *ops, size_t ops_count)
{
    if (parser->math.oper_list.count != parser->math.num_list.count - 1) {
        fprintf(stderr, "%s:%d:1 Invalid input:\n", __FILE__, __LINE__);
        print_math(parser->math);
        exit(1);
    }

    Stack num_stack;
    Stackc oper_stack;
    list_alloc(num_stack);
    list_alloc(oper_stack);
    stack_push(&num_stack, parser->math.num_list.items[0]);

    size_t j = 1;
    for (size_t i = 0; i < parser->math.oper_list.count; i++) {
        char operator = parser->math.oper_list.items[i];
        if (str_contains(ops, ops_count, operator)) {
            double num = stack_pop(&num_stack);
            double result = perform_operation(num, parser->math.num_list.items[i + 1], operator);
            stack_push(&num_stack, result);
            j = i + 2;
        }
        else {
            stackc_push(&oper_stack, operator);
            stack_push(&num_stack, parser->math.num_list.items[j]);
            j++;
        }
    }

    if (num_stack.count != oper_stack.count + 1) {
        printf("Something's wrong, I can feel it\n");
        printf("new.oper_list.count: %zu\n", oper_stack.count);
        printf("num_stack.count: %zu\n", num_stack.count);
    }

    list_transfer(parser->math.num_list, num_stack);
    list_transfer(parser->math.oper_list, oper_stack);
}


void parse_expression(Parser *parser)
{
    remove_white_spaces(parser->expr, strlen(parser->expr));

    parse_operations(parser, "^", 1);
    parse_operations(parser, "*/%", 3);
}


double do_the_math(const Math math)
{
    // print_math(math);

    double result = math.num_list.items[0];

    for (size_t i = 1; i < math.num_list.count; i++) {
        result = perform_operation(result, math.num_list.items[i], math.oper_list.items[i - 1]);
    }

    return result;
}

bool expected(char* input, double expected_output)
{
    Parser parser;
    parser.expr = input;
    list_alloc(parser.math.num_list);
    list_alloc(parser.math.oper_list);
    parse_input(input, strlen(input) + 1, &parser.math);

#if 0
    for (size_t i = 0; i < parser.math.num_list.count; i++) {
        printf("%lf ",parser.math.num_list.items[i]);
        if (i < parser.math.oper_list.count) {
            printf("%c ", parser.math.oper_list.items[i]);
        }
    }
#endif

    parse_expression(&parser);
    double output = do_the_math(parser.math);

    list_free(parser.math.num_list);
    list_free(parser.math.oper_list);

    bool success;

    printf("{\n");
    printf("    Input   : %s\n", input);
    if (((int64_t)output ^ (int64_t)expected_output) != 0) {
        printf("    \033[31mOutput  : %lf\n", output);
        success= false;
    }
    else {
        printf("    Output  : %lf\n", output);
        success = true;
    }
    printf("    \033[0mExpected: %lf\n", expected_output);
    printf("}\n");

    return success; 
}

#define EXPECTED(expr, EXPECTED_OUTPUT, success)                               \
  do {                                                                         \
    char EXPR[] = expr;                                                        \
    if (! expected(EXPR, EXPECTED_OUTPUT)) {                                   \
      success = false;                                                          \
    }                                                                          \
  } while (0)

void test()
{
    bool success = true;
    EXPECTED("3 + 3", 6, success);
    EXPECTED("3 - 3", 0, success);
    EXPECTED("3 * 3", 9, success);
    EXPECTED("3 / 3", 1, success);
    EXPECTED("3 ^ 3", 27, success);
    EXPECTED("5 % 3", 2, success);
    EXPECTED("3 * 7 + 46 % 4", 23, success);
    EXPECTED("5 * 2 ^ 3 / 4 + 1 * 5", 15, success);
    EXPECTED("5 * 2 ^ 3 ^ 4 + 1 +  79", 20560, success);
    EXPECTED("1 + 3 *  9", 28, success);
    EXPECTED("64 ^ 0 / 2 + 6", 6.5f, success);
    EXPECTED("4 ^ 2 / 8 + 1", 3, success);
    EXPECTED("1 / 0.5 + 6", 8, success);
    EXPECTED("3 + 5 / 3 ^ 4 * 9 - 2 * 1", 1.55555555555556, success);
    EXPECTED("3 + 3 + 3 + 3 + 3 ^ 0 + 3", 16, success);
    EXPECTED("1 / 0.3 - 0.1 * 3 ^ 2", 2.43333333333333, success);
    EXPECTED("0 / 2 * 1", 0, success);
    EXPECTED("0/1*2", 0, success);

    if (success) {
        printf("\033[32mALL TESTS WERE SUCCESSFUL!\n");
    }
    else {
        printf("\033[31mNOT ALL TESTS WERE SUCCESSFUL!\n");
    }
}

int main(void)
{
#if 1
    test(); 
    return 0;
#endif

    char buffer[1000];
    const size_t buffer_count = array_len(buffer);

    Parser parser;
    list_alloc(parser.math.num_list);
    list_alloc(parser.math.oper_list);
    double result;

    while (true)
    {
        memset(buffer, 0, buffer_count * sizeof(char));
        fgets(buffer, buffer_count, stdin);
        parser.expr = buffer;

        parse_input(buffer, buffer_count, &parser.math); 
        parse_expression(&parser);

        result = do_the_math(parser.math);

        printf("Result: %lf\n", result);

        list_clear(parser.math.num_list);
        list_clear(parser.math.oper_list);
    }

    return 0;
}