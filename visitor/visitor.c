#include "visitor.h"

// method to select the visit method of a given node
void accept(Visitor *visitor, ASTNode *node)
{
    fprintf(stderr,"esto es magia mi loco");

    if (!node)
    {
        return;
    }

    fprintf(stderr,"esto es magia mi loco");

    switch (node->type)
    {
    case NODE_PROGRAM:
        fprintf(stderr, "ESTAMOS EN EL VISIT DE PROGRAM\n");
        visitor->visit_program(visitor, node);
        break;
    case NODE_NUMBER:
        fprintf(stderr, "ESTAMOS EN EL VISIT DE NUMBER\n");
        visitor->visit_number(visitor, node);
        break;
    case NODE_STRING:
        fprintf(stderr, "ESTAMOS EN EL VISIT DE STRING\n");
        visitor->visit_string(visitor, node);
        break;
    case NODE_BOOLEAN:
        visitor->visit_boolean(visitor, node);
        break;
    case NODE_UNARY_OP:
        visitor->visit_unary_op(visitor, node);
        break;
    case NODE_BINARY_OP:
        fprintf(stderr, "ESTAMOS EN EL VISIT DE binary op\n");
        visitor->visit_binary_op(visitor, node);
        break;
    case NODE_ASSIGNMENT:
    case NODE_D_ASSIGNMENT:
        fprintf(stderr, "ESTAMOS EN EL VISIT DE ASSIGNEMTNS\n");
        visitor->visit_assignment(visitor, node);
        break;
    case NODE_VARIABLE:
        fprintf(stderr, "ESTAMOS EN EL VISIT DE VARAIBLE\n");
        visitor->visit_variable(visitor, node);
        break;
    case NODE_FUNC_CALL:
        visitor->visit_function_call(visitor, node);
        break;
    case NODE_BLOCK:
        fprintf(stderr, "ESTAMOS EN EL VISIT DE block\n");
        visitor->visit_block(visitor, node);
        break;
    case NODE_FUNC_DEC:
        fprintf(stderr,"ESTAMOS EN EL VISIT DE NODE_DECL_FUNC\n");
        visitor->visit_function_dec(visitor, node);
        break;
    case NODE_LET_IN:
            fprintf(stderr, "ESTAMOS EN EL VISIT DE let in\n");
        visitor->visit_let_in(visitor, node);
        break;
    case NODE_CONDITIONAL:
        visitor->visit_conditional(visitor, node);
        break;
    case NODE_Q_CONDITIONAL:
        visitor->visit_q_conditional(visitor, node);
        break;
    case NODE_LOOP:
        visitor->visit_loop(visitor, node);
        break;
    case NODE_FOR_LOOP:
        visitor->visit_for_loop(visitor, node);
        break;
    case NODE_TYPE_DEC:
                fprintf(stderr, "ESTAMOS EN EL VISIT DE type declaration\n");
        visitor->visit_type_dec(visitor, node);
        break;
    case NODE_TYPE_INST:
                fprintf(stderr, "ESTAMOS EN EL VISIT DE type instance\n");
        visitor->visit_type_instance(visitor, node);
        break;
    case NODE_TEST_TYPE:
                fprintf(stderr, "ESTAMOS EN EL VISIT DE test type\n");
        visitor->visit_test_type(visitor, node);
        break;
    case NODE_CAST_TYPE:
                fprintf(stderr, "ESTAMOS EN EL VISIT DE cast type\n");
        visitor->visit_casting_type(visitor, node);
        break;
    case NODE_TYPE_GET_ATTR:
        fprintf(stderr, "ESTAMOS EN EL VISIT DE GETTER\n");
        visitor->visit_attr_getter(visitor, node);
        break;
    case NODE_TYPE_SET_ATTR:
        fprintf(stderr, "ESTAMOS EN EL VISIT DE SETTER \n");
        visitor->visit_attr_setter(visitor, node);
        break;
    case NODE_BASE_FUNC:
                fprintf(stderr, "ESTAMOS EN EL VISIT DE base function\n");
        visitor->visit_base_func(visitor, node);
        break;
    }
}

// method to get the context of a node
void get_context(Visitor *visitor, ASTNode *node)
{
    for (int i = 0; i < node->data.program_node.count; i++)
    {
        ASTNode *child = node->data.program_node.statements[i];
        if (child->type == NODE_FUNC_DEC || child->type == NODE_TYPE_DEC)
        {
            child->context->parent = node->context;
            child->scope->parent = node->scope;
            if (!save_context_item(node->context, child))
            {
                char *func_or_type = child->type == NODE_FUNC_DEC ? "Function" : "Type";
                char *name = child->type == NODE_FUNC_DEC ? child->data.func_node.name : child->data.type_node.name;
                report_error(
                    visitor, "%s '%s' already exists. Line: %d.",
                    func_or_type, name, child->line);
            }
        }
    }
}

// method to add an error to the error array of the visitor
void add_error(char ***array, int *count, const char *str)
{
    *array = realloc(*array, (*count + 1) * sizeof(char *));
    (*array)[*count] = strdup(str);
    (*count)++;
}

// method to build an error mssg
void report_error(Visitor *v, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *str = NULL;
    vasprintf(&str, fmt, args);
    va_end(args);
    add_error(&(v->errors), &(v->error_count), str);
}

// Free memory
void free_error(char **array, int count)
{
    for (int i = 0; i < count; i++)
    {
        free(array[i]);
    }
    free(array);
}