// ast.h

#ifndef AST_H
#define AST_H

#include "../type_value/type.h"
#include "../scope/scope.h"
#include "../scope/env.h"
#include "../scope/usage.h"

extern int line_num;

typedef enum {
    AST_NUM,
    AST_VAR,
    AST_STRING,
    AST_BOOLEAN,
    AST_BINARY_OP,
    AST_IF,
    AST_ASSIGNMENT,
    AST_DESTRUCTOR,
    AST_PROGRAM,
    AST_BLOCK,
    AST_LET,
    AST_CALL_FUNC,
    AST_WHILE,
    AST_DECL_FUNC,
    AST_TYPE,
    AST_GETTER,
    AST_SETTER,
    AST_INSTANCE,
    AST_RETURN
} ASTNodeType;

typedef struct  ASTNode ASTNode;

typedef struct ASTNode
{
    int line;                       // Linea del codigo fuente
    int checked;
    int param;
    ASTNodeType type;               // Tipo del nodo
    TypeValue* computed_type;       // Tipo del valor del nodo
    Scope* scope;
    Env* env;
    char* static_type;
    Usage * usages;                 // son las derivaciones de una variable y todos sus usos en le codiog, util para propagar tipos a todas las apariciones de ella
    union 
    {
        double number_value;
        char* string_value;
        char* var_name;

        // Operaciones Binarias
        // Asignacion de Variables
        // GETTER
        struct 
        {
            Operator op;
            char* name_op;
            ASTNode* left;
            ASTNode* right;
        }binary_op;

        // lista de sentencias

        struct 
        {
            ASTNode** statements;
            int count;
        }program;
        
        // IF_ElSE

        struct 
        {
            ASTNode* cond;
            ASTNode* body;
            ASTNode* else_branch;
            
        }conditional;
        
        // LET_IN, func

        struct {
            char* name;
            ASTNode** args;
            int arg_count;
            ASTNode *body;
        } func_node;
        
        struct 
        {
            char* name_type;
            char* name_parent;
            ASTNode** args;
            ASTNode** p_args;
            int args_count;
            int p_args_count;
            ASTNode** body_elements;
            int body_count;
            ASTNode* parent_instance;
            int id;
            int p_constructor;
        }typeDef;


        struct 
        {
            ASTNode* instance;
            ASTNode* property;
            ASTNode* value;
        }setter;
        
        struct 
        {
            ASTNode* value;
        }return_stmt;
        
        

    }data;

}ASTNode;





ASTNode* create_program_node(ASTNode** statements, int count,ASTNodeType type);
ASTNode* create_num_node(double value);
ASTNode* create_var_node(char* name, char* type, int param);
ASTNode* create_string_node(char* value);
ASTNode* create_boolean_node(char* value);
ASTNode* create_binary_op_node(Operator op, 
                               char* op_name, 
                               ASTNode* left, 
                               ASTNode* right, 
                               TypeValue* return_type);
ASTNode* create_unary_op_node(Operator op,
                              char* op_name,
                              ASTNode* operand,
                              TypeValue* return_type);
ASTNode* create_let_in_node(ASTNode** bindings,
                            int dec_count,
                            ASTNode* body);
ASTNode* create_assignment_node(char* name,
                                ASTNode*value,
                                char* type_name,
                                ASTNodeType type);
ASTNode*  create_call_function_node(char* name, ASTNode** args, int arg_count);

ASTNode* create_decl_function_node(char* name_func,
                                ASTNode** args,
                                int arg_count,
                                ASTNode* body,
                                char* return_type);
ASTNode* create_while_node(ASTNode* cond, ASTNode* body);

ASTNode *create_conditional_node(ASTNode *cond,
                                 ASTNode *body,
                                 ASTNode *else_branch);

ASTNode* create_return_stmt(ASTNode* value);








ASTNode* create_type_node(char * name_type,
                          char* name_parent,
                          ASTNode** args,
                          int args_count,
                          ASTNode** p_args,
                          int p_args_count,
                          ASTNode** body,
                          int body_count,
                          int p_constructor);

// crear una instancia de un type
ASTNode* create_struct_instance_node(char* name_type,
                                     ASTNode** args,
                                     int args_count);

//acceder a un atributo de un type
ASTNode* create_property_access_node(ASTNode* instance,
                                     ASTNode* property);

//para asignaciones de valor a un atributo de un type
ASTNode* create_property_assignment_node(ASTNode* instance,
                                         ASTNode* property,
                                         ASTNode* value);

// crear un nodo de llamads a funciones sin contexto adicional
ASTNode* create_anonymous_func_node();





void print_ast(ASTNode* node, int indent);


void free_ast(ASTNode* node);


void propagate_env_scope(ASTNode *node, ASTNode *child);

#endif
