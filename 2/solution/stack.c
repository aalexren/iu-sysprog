#include <stdlib.h>
#include <stdio.h>

typedef struct char_node {
    char data;
    struct char_node* next;
} char_node;

struct char_stack {
    char_node* top;
};

struct char_stack*
cs_init()
{
    return (struct char_stack*)malloc(sizeof(struct char_stack));
}

int
cs_push(struct char_stack *stack, char data)
{
    struct char_node* node = malloc(sizeof(struct char_node));
    node->data = data;
    node->next = stack->top;

    stack->top = node;
    
    return data;
}

int 
cs_isempty(struct char_stack *stack)
{
    return stack->top == NULL;
}

char
cs_peek(struct char_stack *stack)
{
    /* skipped NULL check because user should do it himself. */
    return stack->top->data;
}

void
cs_pop(struct char_stack *stack)
{
    /* skipped NULL check because user should do it himself. */
    struct char_node *node = stack->top;
    stack->top = stack->top->next;
    free(node);
}

void
cs_print(struct char_stack *stack)
{
    struct char_node *node = stack->top;
    while (node != NULL)
    {
        printf("%c", node->data);
        node = node->next;
        if (node != NULL)
        {
            printf(" -> ");
        }
    }
}