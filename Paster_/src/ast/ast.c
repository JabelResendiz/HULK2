

// ast.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

int line_num = 1;
/// @brief Usado para propagar el entorno y el scope al hijo
/// @param node
/// @param child
void propagate_env_scope(ASTNode *node, ASTNode *child)
{
    if (!node || !child)
        return;

    child->scope->parent = node->scope;
    child->env->parent = node->env;
}

void free_ast(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case AST_VAR:
        free(node->data.var_name);
        break;
    case AST_BINARY_OP:
    case AST_ASSIGNMENT:
    case AST_DESTRUCTOR:
    case AST_WHILE:
    case AST_GETTER:
        if (node->data.binary_op.left) free_ast(node->data.binary_op.left);
        if (node->data.binary_op.right) free_ast(node->data.binary_op.right);
        break;

    case AST_PROGRAM:
    case AST_BLOCK:
        for (int i = 0; i < node->data.program.count; i++)
            if (node->data.program.statements[i]) free_ast(node->data.program.statements[i]);
        free(node->data.program.statements);
        break;

    case AST_DECL_FUNC:
    case AST_LET:
        for (int i = 0; i < node->data.func_node.arg_count; i++)
            if (node->data.func_node.args[i]) free_ast(node->data.func_node.args[i]);
        if (node->data.func_node.body) free_ast(node->data.func_node.body);
        break;
    
    case AST_IF:
        if (node->data.conditional.body) free_ast(node->data.conditional.body);
        if (node->data.conditional.else_branch) free_ast(node->data.conditional.else_branch);
        if (node->data.conditional.cond) free_ast(node->data.conditional.cond);
        break;
    case AST_CALL_FUNC:
        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            if (node->data.func_node.args[i]) free_ast(node->data.func_node.args[i]);
        }
        break;
    case AST_TYPE:
        // despues las definiciones de ti mismo
        for (int i = 0; i < node->data.typeDef.args_count; i++)
        {
            if (node->data.typeDef.args[i]) free_ast(node->data.typeDef.args[i]);
        }
        // las definiciones de tu cuerpo
        for (int i = 0; i < node->data.typeDef.body_count; i++)
        {
            if (node->data.typeDef.body_elements[i]) free_ast(node->data.typeDef.body_elements[i]);
        }
        // despues las deifniciones de tu padre
        for (int i = 0; i < node->data.typeDef.p_args_count; i++)
        {
            if (node->data.typeDef.p_args[i]) free_ast(node->data.typeDef.p_args[i]);
        }
        break;
    case AST_INSTANCE:
        for (int i = 0; i < node->data.typeDef.args_count; i++)
        {
            if (node->data.typeDef.args[i]) free_ast(node->data.typeDef.args[i]);
        }
        break;
    case AST_SETTER:
        if (node->data.setter.property) free_ast(node->data.setter.property);
        if (node->data.setter.value) free_ast(node->data.setter.value);
        if (node->data.setter.instance) free_ast(node->data.setter.instance);
        break;
    }

    free_scope(node->scope);
    free_env(node->env);
    free(node);
}

void print_ast(ASTNode *node, int indent)
{
    if (!node)
        return;

    for (int i = 0; i < indent; i++)
        printf("  ");

    switch (node->type)
    {
    case AST_PROGRAM:
        printf("Program:\n");
        for (int i = 0; i < node->data.program.count; i++)
        {
            print_ast(node->data.program.statements[i], indent + 1);
        }
        break;
    case AST_BLOCK:
        printf("Block:\n");
        for (int i = 0; i < node->data.program.count; i++)
        {
            print_ast(node->data.program.statements[i], indent + 1);
        }
        break;
    case AST_LET:
        printf("Let-in: %s\n", node->data.func_node.name);
        print_ast(node->data.func_node.body, indent + 1);
        break;
    case AST_NUM:
        printf("Number: %g\n", node->data.number_value);
        break;
    case AST_STRING:
        printf("String: %s\n", node->data.string_value);
        break;
    case AST_BOOLEAN:
        printf("Boolean: %s\n", node->data.string_value);
        break;
    case AST_VAR:
        printf("Variable: %s\n", node->data.var_name);
        break;
    case AST_BINARY_OP:
    {
        const char *op_str;
        switch (node->data.binary_op.op)
        {
        case OP_ADD:
            op_str = "+";
            break;
        case OP_SUB:
            op_str = "-";
            break;
        case OP_MULT:
            op_str = "*";
            break;
        case OP_DIV:
            op_str = "/";
            break;
        case OP_MOD:
            op_str = "%";
            break;
        case OP_POW:
            op_str = "^";
            break;
        case OP_AND:
            op_str = "&";
            break;
        case OP_OR:
            op_str = "|";
            break;
        case OP_CONCAT:
            op_str = "@";
            break;
        case OP_DCONCAT: 
            op_str = "@@"; 
            break;
        case OP_EQ:
            op_str = "==";
            break;
        case OP_NEQ:
            op_str = "!=";
            break;
        case OP_GE:
            op_str = ">=";
            break;
        case OP_GT:
            op_str = ">";
            break;
        case OP_LE:
            op_str = "<=";
            break;
        case OP_LT:
            op_str = "<";
            break;
        }
        printf("Binary Op: %s\n", op_str);
        print_ast(node->data.binary_op.left, indent + 1);
        print_ast(node->data.binary_op.right, indent + 1);
        break;
    }
    case AST_ASSIGNMENT:
    case AST_DESTRUCTOR:
        printf("Assignment:\n");
        print_ast(node->data.binary_op.left, indent + 1);
        print_ast(node->data.binary_op.right, indent + 1);
        break;
    case AST_IF:
        printf("Conditional:\n");
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("Condition:\n");
        print_ast(node->data.conditional.cond, indent + 2);
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("If part:\n");
        print_ast(node->data.conditional.body, indent + 2);
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("else part:\n");
        print_ast(node->data.conditional.else_branch, indent + 2);
        break;
    case AST_WHILE:
        printf("While:\n");
        for (int i = 0; i < indent + 1; i++)
            printf(" ");
        printf("Condition:\n");
        print_ast(node->data.binary_op.left, indent + 2);
        for (int i = 0; i < indent + 1; i++)
            printf(" ");
        printf("Body:\n");
        print_ast(node->data.binary_op.right, indent + 2);
        break;
    case AST_DECL_FUNC:
        printf("Declaration Function : %s \n", node->data.func_node.name);
        // Mostrar parámetros si existen
        if (node->data.func_node.arg_count > 0) {
            for (int i = 0; i < indent + 1; i++)
                printf("  ");
            printf("Parameters:\n");
            for (int i = 0; i < node->data.func_node.arg_count; i++) {
                for (int j = 0; j < indent + 2; j++)
                    printf("  ");
                printf("Parameter %d:\n", i + 1);
                print_ast(node->data.func_node.args[i], indent + 3);
            }
        }
        // Mostrar el cuerpo de la función
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf("Body:\n");
        print_ast(node->data.func_node.body, indent + 2);
        break;
    case AST_CALL_FUNC:
        printf("Function_call: %s\n", node->data.func_node.name);
        int arg_count = node->data.func_node.arg_count;
        for (int i = 0; i < arg_count; i++)
        {
            print_ast(node->data.func_node.args[i], indent + 1);
        }
        break;
    case AST_TYPE:
        printf("TYPE %s inherits %s\n", node->data.typeDef.name_type, node->data.typeDef.name_parent);
        for (int i = 0; i < node->data.typeDef.body_count; i++)
        {
            print_ast(node->data.typeDef.body_elements[i], indent + 1);
        }
        break;
    case AST_GETTER:
        printf("Type member: \n");
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf(" Instance:\n");
        print_ast(node->data.binary_op.left, indent + 2);
        for (int i = 0; i < indent + 1; i++)
            printf("  ");
        printf(" Member:\n");
        print_ast(node->data.binary_op.right, indent + 2);
        break;
    case AST_INSTANCE:
        printf("Type_instance: %s\n", node->data.typeDef.name_type);
        for (int i = 0; i < node->data.typeDef.args_count; i++)
        {
            print_ast(node->data.typeDef.args[i], indent + 1);
        }
        break;
    case AST_SETTER:
        printf("Assignment:\n");
        print_ast(node->data.setter.property, indent + 1);
        print_ast(node->data.setter.value, indent + 1);
        break;
    case AST_RETURN:
        printf("Return:\n");
        if (node->data.return_stmt.value) {
            print_ast(node->data.return_stmt.value, indent + 1);
        }
        break;
    }
}

