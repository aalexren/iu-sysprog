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

char *
cs_splice(struct char_stack *stack)
{
    size_t len = 0;
    struct char_node *pntr = stack->top;
    while (pntr != NULL)
    {
        len++;
        pntr = pntr->next;
    }

    pntr = stack->top;
    char *ret = malloc(sizeof(char) * len);
    for (size_t i = 0; i < len; ++i)
    {
        ret[i] = pntr->data;
        pntr = pntr->next;
    }

    return ret;
}

struct char_stack*
cs_reverse(const struct char_stack *stack)
{
    struct char_stack *temp = cs_init();
    struct char_node *pntr = stack->top;
    while (pntr != NULL)
    {
        cs_push(temp, pntr->data);
        pntr = pntr->next;
    }

    return temp;
}

void 
cs_free(struct char_stack *stack)
{
    while (!cs_isempty(stack))
    {
        cs_pop(stack);
    }
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