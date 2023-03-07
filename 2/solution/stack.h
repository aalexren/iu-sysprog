struct char_node;

struct char_stack;

struct char_stack* cs_init();

int cs_push(struct char_stack *stack, char data);

int cs_isempty(struct char_stack *stack);

char cs_peek(struct char_stack *stack);

void cs_pop(struct char_stack *stack);

/**
 * Simply splices chars from stack to single string.
 */
char *cs_splice(struct char_stack *stack);

struct char_stack *cs_reverse(const struct char_stack* stack);

void cs_free(struct char_stack *stack);

void cs_print(struct char_stack *stack);