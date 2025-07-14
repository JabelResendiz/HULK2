

// type.c

#include "../ast/ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

TypeValue TYPE_OBJ = {"Object", NULL, NULL, NULL, NULL, 0, NULL, NULL, 0, false, false};
TypeValue TYPE_NUM = {"Number", NULL, &TYPE_OBJ, NULL, NULL, 0, NULL, NULL, 0, false, false};
TypeValue TYPE_STRING = {"String", NULL, &TYPE_OBJ, NULL, NULL, 0, NULL, NULL, 0, false, false};
TypeValue TYPE_BOOLEAN = {"Boolean", NULL, &TYPE_OBJ, NULL, NULL, 0, NULL, NULL, 0, false, false};
TypeValue TYPE_VOID = {"Void", NULL, &TYPE_OBJ, NULL, NULL, 0, NULL, NULL, 0, false, false};
TypeValue TYPE_ERROR = {"Error", NULL, NULL, NULL, NULL, 0, NULL, NULL, 0, false, false};
TypeValue TYPE_GENERIC = {"Generic", NULL, NULL, NULL, NULL, 0, NULL, NULL, 0, false, false};

// Nuevos tipos
TypeValue TYPE_ANY = {"Any", NULL, &TYPE_OBJ, NULL, NULL, 0, NULL, NULL, 0, false, false};
TypeValue TYPE_NULL = {"Null", NULL, &TYPE_OBJ, NULL, NULL, 0, NULL, NULL, 0, false, false};
TypeValue TYPE_FUNCTION = {"Function", NULL, &TYPE_OBJ, NULL, NULL, 0, NULL, NULL, 0, false, false};

// Funciones para tipos genéricos
TypeParameter* create_type_parameter(char* name, TypeValue* constraint) {
    TypeParameter* param = malloc(sizeof(TypeParameter));
    param->name = strdup(name);
    param->constraint = constraint;
    param->next = NULL;
    return param;
}

void add_type_parameter(TypeValue* type, TypeParameter* param) {
    if (!type->type_parameters) {
        type->type_parameters = param;
    } else {
        TypeParameter* current = type->type_parameters;
        while (current->next) {
            current = current->next;
        }
        current->next = param;
    }
}

TypeValue* create_generic_type(char* name, TypeParameter* params, TypeValue* super_type, struct ASTNode* node) {
    TypeValue* type = malloc(sizeof(TypeValue));
    type->name = strdup(name);
    type->super_type = super_type;
    type->type_parameters = params;
    type->is_generic = true;
    type->is_instantiated = false;
    type->concrete_types = NULL;
    type->num_concrete_types = 0;
    type->def_node = node;
    type->argument_types = NULL;
    type->num_params = 0;
    return type;
}

TypeValue* instantiate_generic_type(TypeValue* generic_type, TypeValue** concrete_types, int num_concrete) {
    if (!generic_type->is_generic) {
        return generic_type;
    }
    
    TypeValue* instantiated = malloc(sizeof(TypeValue));
    instantiated->name = strdup(generic_type->name);
    instantiated->super_type = generic_type->super_type;
    instantiated->type_parameters = generic_type->type_parameters;
    instantiated->is_generic = false;
    instantiated->is_instantiated = true;
    instantiated->concrete_types = concrete_types;
    instantiated->num_concrete_types = num_concrete;
    instantiated->def_node = generic_type->def_node;
    instantiated->argument_types = NULL;
    instantiated->num_params = 0;
    return instantiated;
}

int is_generic_type(TypeValue* type) {
    return type && type->is_generic;
}

int is_instantiated_type(TypeValue* type) {
    return type && type->is_instantiated;
}

TypeValue* get_generic_base(TypeValue* type) {
    if (!type) return NULL;
    
    TypeValue* current = type;
    while (current && !current->is_generic) {
        current = current->super_type;
    }
    return current;
}

// Funciones para inferencia de tipos
TypeValue* infer_function_type(TypeValue** param_types, int num_params, TypeValue* return_type) {
    TypeValue* func_type = malloc(sizeof(TypeValue));
    func_type->name = strdup("Function");
    func_type->super_type = &TYPE_FUNCTION;
    func_type->argument_types = param_types;
    func_type->num_params = num_params;
    func_type->element_type = return_type;
    func_type->is_generic = false;
    func_type->is_instantiated = false;
    func_type->type_parameters = NULL;
    func_type->concrete_types = NULL;
    func_type->num_concrete_types = 0;
    func_type->def_node = NULL;
    return func_type;
}

TypeValue* infer_expression_type(ASTNode* node) {
    if (!node) return &TYPE_ERROR;
    
    // Si ya tiene un tipo computado, usarlo
    if (node->computed_type) {
        return node->computed_type;
    }
    
    // Inferir tipo basado en el tipo de nodo
    switch (node->type) {
        case AST_NUM:
            return &TYPE_NUM;
        case AST_STRING:
            return &TYPE_STRING;
        case AST_BOOLEAN:
            return &TYPE_BOOLEAN;
        case AST_VAR:
            // Para variables, necesitamos buscar en el scope
            return &TYPE_ANY; // Placeholder
        case AST_BINARY_OP:
            // Para operaciones binarias, inferir basado en operandos
            return &TYPE_NUM; // Placeholder
        default:
            return &TYPE_ANY;
    }
}

TypeValue* unify_types(TypeValue* type1, TypeValue* type2) {
    if (!type1 || !type2) return &TYPE_ERROR;
    
    // Si son iguales, retornar cualquiera
    if (compare_types(type1, type2)) {
        return type1;
    }
    
    // Si uno es ANY, retornar el otro
    if (compare_types(type1, &TYPE_ANY)) {
        return type2;
    }
    if (compare_types(type2, &TYPE_ANY)) {
        return type1;
    }
    
    // Buscar el tipo común más específico
    return compute_lca(type1, type2);
}

TypeValue *resolve_op_type(TypeValue *left, TypeValue *right, Operator op)
{

    for (int i = 0; i < rule_count; i++)
    {
        if (compare_types(left, operatorRules[i].left_type) &&
            compare_types(right, operatorRules[i].right_type) &&
            op == operatorRules[i].op)
        {
            return &operatorRules[i].result_type;
        }
    }

    return &TYPE_ERROR;
}

int match_op(TypeValue *first, TypeValue *second, Operator op)
{
    for (int i = 0; i < rule_count; i++)
    {
        if (compare_types(first, operatorRules[i].left_type) &&
            compare_types(second, operatorRules[i].right_type) &&
            op == operatorRules[i].op)
        {
            return 1;
        }
    }

    return 0;
}


int compare_types(TypeValue *first, TypeValue *second)
{
    // fprintf(stderr,"el tipo de first es : %s  y el de second es %s \n", first->name,second->name);
    //  ambos son NULL => son iguales
    if (first == NULL && second == NULL)
        return 1;
    if (first == NULL || second == NULL)
        return 0;

    if (!strcmp(first->name, second->name))
    {
        return compare_types(first->element_type, second->element_type);
    }

    return 0;
}

// Verifica si un tipo ancestor es un ancestro o igual del tipo type
// Devuelve 1 si ancestor es igual a type o alguno de sus ancestro (padre,abuelo)
// Devulve 0 en otro caso
int ancestor_type(TypeValue *ancestor, TypeValue *type_f)
{
    if (!type_f)
        return 0;

    if (compare_types(ancestor, type_f))
    {
        fprintf(stderr, "900000\n");
        return 1;
    }

    return ancestor_type(ancestor, type_f->super_type);
}

// // Marca todos los ancestros de un tipo en un arreglo de flags o tabla hash simple
// void mark_ancestors(TypeValue* type, int* visited, int max_types) {
//     while (type != NULL) {
//         visited[type->id] = 1;
//         type = type->super_type;
//     }
// }

// TypeValue* compute_lca(TypeValue* type1, TypeValue* type2) {
//     // Casos especiales
//     if (type_equals(type1, &TYPE_ANY) || type_equals(type2, &TYPE_ANY)) {
//         return &TYPE_ANY;
//     }
//     if (type_equals(type1, &TYPE_ERROR) || type_equals(type2, &TYPE_ERROR)) {
//         return &TYPE_ERROR;
//     }

//     // Supón que los tipos tienen IDs únicos del 0 al N-1
//     const int MAX_TYPES = 256;  // Aumentar si hay más tipos en tu sistema
//     int visited[MAX_TYPES] = {0};

//     // Marca todos los ancestros de type1
//     mark_ancestors(type1, visited, MAX_TYPES);

//     // Recorre type2 hacia arriba hasta encontrar uno ya marcado
//     while (type2 != NULL) {
//         if (visited[type2->id]) {
//             return type2;  // Primer ancestro común
//         }
//         type2 = type2->super_type;
//     }

//     // Si no se encontró ningún ancestro común, devolvemos TYPE_ANY como fallback
//     return &TYPE_ANY;
// }

// halla el menor ancestro comun de ambos tipos
TypeValue *compute_lca(TypeValue *type_1, TypeValue *type_2)
{
    // si alguno es de tipo ANY
    if (compare_types(type_1, &TYPE_GENERIC) ||
        compare_types(type_2, &TYPE_GENERIC))
    {
        return &TYPE_GENERIC;
    }

    else if (compare_types(type_1, &TYPE_ERROR) ||
             compare_types(type_2, &TYPE_ERROR))
    {
        return &TYPE_ERROR;
    }

    fprintf(stderr, "3232\n");
    if (ancestor_type(type_1, type_2))
    {
        fprintf(stderr, "00000\n");
        return type_1;
    }

    if (ancestor_type(type_2, type_1))
        return type_2;

    return compute_lca(type_1->super_type, type_2->super_type);
}

/// @brief Me dice si type es un tipo builtin
/// @param type
/// @return
int is_builtin_type(TypeValue *type)
{
    return (compare_types(type, &TYPE_BOOLEAN) ||
            compare_types(type, &TYPE_NUM) ||
            compare_types(type, &TYPE_STRING) ||
            compare_types(type, &TYPE_OBJ) ||
            compare_types(type, &TYPE_VOID) ||
            compare_types(type, &TYPE_ERROR));
}

/// @brief Crea un tipo nuevo
/// @param name
/// @param type
/// @return retorna el tipo creado
TypeValue *create_type(char *name, TypeValue *type, TypeValue **param_types, int count, struct ASTNode *node)
{
    TypeValue *new_type = malloc(sizeof(TypeValue));
    new_type->name = name;
    new_type->super_type = type;
    new_type->argument_types = param_types;
    new_type->num_params = count;
    new_type->def_node = node;
    return new_type;
}

// averigua el tipo real (si esta definido en el programa se crea)
// sino se mantiene el que tiene por defecto
TypeValue *resolve_node_type(ASTNode *node)
{
    fprintf(stderr,"ENTRE AL RESOLVE NODE TYPE\n");

    if (!node) {
        fprintf(stderr, "resolve_node_type: node is NULL\n");
        return &TYPE_ERROR;
    }

    TypeValue *type = node->computed_type;
    fprintf(stderr,"PERO BUENO\n");
    
    // Si el tipo computado es NULL, retornar TYPE_ERROR
    if (!type) {
        fprintf(stderr, "resolve_node_type: computed_type is NULL\n");
        return &TYPE_ERROR;
    }
    
    // Si el nombre del tipo es NULL, retornar el tipo directamente
    if (!type->name) {
        fprintf(stderr, "resolve_node_type: type->name is NULL\n");
        return type;
    }
    
    // cuando permita la creacion de tipos nuevos
    
    Symbol *symbol = find_type_scopes(node->scope, type->name);

    if (symbol)
        return symbol->type;

    return type;
}


// Busca todos los tipos de los argumentos pasados por argumento
TypeValue **resolve_nodes_type(ASTNode **args, int arg_count)
{
    TypeValue **types = malloc(sizeof(TypeValue *) * arg_count);

    for (int i = 0; i < arg_count; i++)
    {
        types[i] = resolve_node_type(args[i]);
    }

    return types;
}

