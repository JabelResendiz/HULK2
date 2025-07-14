
//types.h

#ifndef TYPE_H
#define TYPE_H

#include <stdbool.h>
#include "operators.h"

typedef struct ASTNode ASTNode;

// Estructura para parámetros de tipo genérico
typedef struct TypeParameter {
    char* name;
    TypeValue* constraint;  // Tipo base que debe respetar
    struct TypeParameter* next;
} TypeParameter;

// aqui se define un tipo del lenguaje ya sea de los nativos o los creados
typedef struct TypeValue
{
    char* name;
    struct TypeValue* element_type;
    struct TypeValue* super_type;
    struct TypeValue** argument_types;
    struct ASTNode* def_node;
    int num_params;
    
    // Nuevos campos para tipos genéricos
    TypeParameter* type_parameters;  // Lista de parámetros de tipo
    TypeValue** concrete_types;      // Tipos concretos para instanciación
    int num_concrete_types;
    bool is_generic;                 // Indica si es un tipo genérico
    bool is_instantiated;            // Indica si es una instancia de un tipo genérico
}TypeValue;

// TIPOS NATIVOS
extern TypeValue TYPE_NUM;
extern TypeValue TYPE_STRING;
extern TypeValue TYPE_STRING;
extern TypeValue TYPE_BOOLEAN;
extern TypeValue TYPE_OBJ;
extern TypeValue TYPE_VOID;
extern TypeValue TYPE_ERROR;
extern TypeValue TYPE_GENERIC;

// Nuevos tipos para el sistema de tipos mejorado
extern TypeValue TYPE_ANY;           // Tipo superior (top type)
extern TypeValue TYPE_NULL;          // Tipo para valores nulos
extern TypeValue TYPE_FUNCTION;      // Tipo base para funciones

int compare_types (TypeValue* first, TypeValue* second);

int ancestor_type(TypeValue* ancestor,TypeValue* type_f);

TypeValue* compute_lca(TypeValue* type_1,TypeValue* type_2);

int is_builtin_type(TypeValue* type);

TypeValue* create_type(char* name, TypeValue* type, TypeValue** param_types,int count, struct ASTNode* node);

// Nuevas funciones para tipos genéricos
TypeValue* create_generic_type(char* name, TypeParameter* params, TypeValue* super_type, struct ASTNode* node);
TypeValue* instantiate_generic_type(TypeValue* generic_type, TypeValue** concrete_types, int num_concrete);
TypeParameter* create_type_parameter(char* name, TypeValue* constraint);
void add_type_parameter(TypeValue* type, TypeParameter* param);

// Funciones para inferencia de tipos
TypeValue* infer_function_type(TypeValue** param_types, int num_params, TypeValue* return_type);
TypeValue* unify_types(TypeValue* type1, TypeValue* type2);

int match_op(TypeValue *first, TypeValue *second, Operator op);

TypeValue *resolve_op_type(TypeValue *left, TypeValue *right, Operator op);

TypeValue *resolve_node_type(ASTNode *node);
TypeValue **resolve_nodes_type(ASTNode **args, int arg_count);

// Funciones para manejo de tipos genéricos
int is_generic_type(TypeValue* type);
int is_instantiated_type(TypeValue* type);
TypeValue* get_generic_base(TypeValue* type);

#endif