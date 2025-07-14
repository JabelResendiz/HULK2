
#ifndef FUNCTION_H
#define FUNCTION_H

#include "../type_value/type.h"

typedef struct Scope Scope;

// //Representa una funcion del lenguaje
typedef struct Function
{
    char* name;
    int count_args;
    TypeValue** args_types;
    TypeValue* result_types;
    struct Function* next;
}Function;

// //Tabla de funcionaes para ver todas las funciones de un entorno
typedef struct FuncTable
{
    Function* first;
    int count;
}FuncTable;

typedef struct FunctionComparisonState {
    int is_match;                // Indica si hay coincidencia total
    int name_matches;            // Coincidencia de nombres
    int parameters_matched;      // Número de parámetros que coinciden
    int first_function_arity;    // Aridad de la primera función
    int second_function_arity;   // Aridad de la segunda función
    char* first_type_name;       // Nombre del tipo principal de la primera función
    char* second_type_name;      // Nombre del tipo principal de la segunda función
    int current_parameter_index; // Posición del parámetro siendo comparado
} FunctionComparisonState;

typedef struct FuncStructure
{
    Function* function;
    FunctionComparisonState * state;
}FuncStructure;


// METODOS

Function* find_function_by_name(Scope* scope,char* name);


void free_func_table(FuncTable* table);

FuncStructure* match_function_scope(Scope *scope,Function* f,Function* second);

int compare_functions_signature(Function* f1, Function* f2);

FunctionComparisonState* compare_argument_types(TypeValue** args1, TypeValue** args2,int number_counts);


FuncStructure* find_function_in_hierarchy(TypeValue* type, Function* f, Function* dec);

FunctionComparisonState *init_state(int matched, int arg1_count, int arg2_count);


FuncStructure *find_type_data(Scope *scope, Function *f, Function *dec);

#endif