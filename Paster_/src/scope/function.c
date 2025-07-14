
#include "scope.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../ast/ast.h"

/// @brief Crea una estructura de function
/// @param matched 1 si hay match 0 si no
/// @param arg1_count la cantidad de argumentos del 1
/// @param arg2_count la cantidad de argumentos del 2
/// @return
FunctionComparisonState *init_state(int matched, int arg1_count, int arg2_count)
{
    FunctionComparisonState *state = malloc(sizeof(*state));
    *state = (FunctionComparisonState){
        .name_matches = 1,
        .first_function_arity = arg1_count,
        .second_function_arity = arg2_count,
        .is_match = matched,
        .parameters_matched = matched};

    return state;
}

/// @brief Buscar una funcion por su nombre en el scope (recursivo)
/// @param scope El scope por donde empezar
/// @param name EL nombre de la funcion a buscar
/// @return NULL si no encuntra la funcion y FUnction* si la encuentra
Function *find_function_by_name(Scope *scope, char *name)
{
    if (!scope)
        return NULL;

    Scope *curr = scope;

    while (curr)
    {
        Function *actual = scope->functions->first;

        for (; actual != NULL; actual = actual->next)
        {
            if (!strcmp(actual->name, name))
                return actual;
        }

        curr = curr->parent;
    }

    return NULL;
}

/// @brief liberar la tabla de funciones
/// @param table la tabla de funciones a liberar
void free_func_table(FuncTable *table)
{
    if (!table)
        return;

    Function *current = table->first;
    int i = 0;
    while (i < table->count)
    {
        Function *next = current->next;
        free(current);
        current = next;
        i++;
    }

    free(table);
}

// devuelve una estructura de funcion si f1 y f2 son iguales(nombre, #args, y comparacion de tipos de args)
FunctionComparisonState *func_equals(Function *f1, Function *f2)
{
    fprintf(stderr, "los nombre son %s y %s\n", f1->name, f2->name);
    //fprintf(stderr,"los tipos del arguemtno de %s es \n\n\n",f2->args_types[0]->name);
    
    if (strcmp(f1->name, f2->name))
    {
        FunctionComparisonState *state = init_state(0, -1, -1);
        state->name_matches = 0;
        return state;
    }

    // si le numero de argumentos es diferntes, error
    if (f1->count_args != f2->count_args)
        return init_state(0, f1->count_args, f2->count_args);

    fprintf(stderr, "Podria ser aqui\n");

    
    // comprobar que los tipos de los argumentos sean iguales
    FunctionComparisonState *s = compare_argument_types(f1->args_types, f2->args_types, f1->count_args);

    return s;
}

// Busca una funcion f dentro de un Scope jerarquico, tratando de encontrar una coincidencia con una funcion existente en ese ambito o alguno de sus ancestros
FuncStructure *match_function_scope(Scope *scope, Function *f, Function *second)
{
    fprintf(stderr, "estoy en el match de fucntion scope\n");

    if (!second)
        fprintf(stderr, "Sceond es null\n");

    if (!scope)
        return NULL;

    int x = 1;

    FuncStructure *result = malloc(sizeof(FuncStructure));

    Function *current = scope->functions->first;

    //fprintf(stderr,"el tipo de curent es %s\n",current->args_types[0]->name);
    fprintf(stderr,"\n\n\n\nel tipo de f es %s\n\n\n\n\n", f->args_types[0]->name);

    for (int i = 0; i < scope->functions->count; i++)
    {
        fprintf(stderr,"la funcion current es %s\n\n", current->name);
        fprintf(stderr,"los tipos del arguemtno de %s es \n\n\n",f->args_types[0]->name);
    

        FunctionComparisonState *aux = func_equals(current, f);

        fprintf(stderr,"el valor de aux es %d\n",aux->is_match);
        //exit(1);
        // se encontro una funcion que matcheaba bien
        if (aux->is_match)
        {
            result->function = current;
            result->state = aux;

  
            return result;
        }

        // si tiene el mismo nombre
        if (aux->name_matches)
        {
            x = 0;
            if ((!result->state && !aux->parameters_matched) ||
                aux->parameters_matched)
            {
                // si no coinciden los argumentos
                // marca que encontro al menos una y guarda ka mejor coincidencia
                result->state = aux;
                result->function = current;
            }
        }

        current = current->next;
    }

    // si no fue en el ambito actual se busca en el padre
    if (scope->parent)
    {
        FuncStructure *data = match_function_scope(scope->parent, f, second);

        // si encontro una coincidencia exacta arriba o not_found , prefiere el ersutlado del padre
        if (x || data->state->is_match)
            return data;

        return result;
    }

    // si no encontro nada , pero hay una declaracion de dec
    if (x && second)
    {
        // se compara con f
        result->state = func_equals(second, f);
        result->function = second;
    }

    else if (x)
    {
        // si no hay dec , devuelve una tupla error
        result->state = init_state(0, -1, -1);
        result->state->name_matches = 0;
    }

    fprintf(stderr, "no candeal\n");
    return result;
}

// Compara los arguemntos de arg1 y de arg2 para ver si son de tipos compatibles
FunctionComparisonState *compare_argument_types(TypeValue **args1, TypeValue **args2, int number_counts)
{
    FunctionComparisonState *state = malloc(sizeof(FunctionComparisonState));

    for (int i = 0; i < number_counts; ++i)
    {
        fprintf(stderr, "el tipo de arg2[%d] es %s\n", i, args2[i]->name);
        fprintf(stderr, "el tipo de arg1[%d] es %s\n", i, args1[i]->name);

        if (!compare_types(args2[i], &TYPE_ERROR) &&
            !compare_types(args2[i], &TYPE_GENERIC) &&
            !ancestor_type(args1[i], args2[i]) &&
            !compare_types(args1[i], &TYPE_GENERIC))
        {
            *state = (FunctionComparisonState){
                .is_match = 0,
                .name_matches = 1,
                .parameters_matched = 1,
                .current_parameter_index = i + 1,
                .first_type_name = args1[i]->name,
                .second_type_name = args2[i]->name};

            return state;
        }
    }

    // exito
    fprintf(stderr, "444444444444444444444\n");
    *state = (FunctionComparisonState){
        .is_match = 1,
        .name_matches = 1,
        .parameters_matched = 1,
        .current_parameter_index = -1,
        .first_type_name = "",
        .second_type_name = ""};

    return state;
}

// Busca una funcion en una jeraquia de tipos, con manejo especial de nombres y herencia
FuncStructure *find_function_in_hierarchy(TypeValue *type, Function *f, Function *dec)
{

    if (!type->def_node)
    {
        fprintf(stderr, "es aqui mi compa\n");
        FuncStructure *structure = malloc(sizeof(FuncStructure));
        FunctionComparisonState *tuple = init_state(0, -1, -1);
        *structure = (FuncStructure){
            .function = NULL,
            .state = tuple};

        structure->state->name_matches = 0;
        return structure;
    }
    fprintf(stderr, "NO ES AQUIIII\n");
    FuncStructure *structure = match_function_scope(type->def_node->scope, f, dec);

    if (structure && (structure->state->is_match) || structure->state->name_matches)
    {
        fprintf(stderr, "AQUIIII\n");
        fprintf(stderr, "el valor de function es %d\n", structure->state->is_match);

        return structure;
    }

    if (type->super_type)
    {
        f->name = remove_type_prefix(f->name, type->name);
        f->name = generate_underscored_name(type->super_type->name, f->name);

        return find_function_in_hierarchy(type->super_type, f, dec);
    }

    return structure;
}

FuncStructure *find_type_data(Scope *scope, Function *f, Function *dec)
{
    if (!scope)
    {
        return NULL;
    }

    fprintf(stderr,"ESTOY EN TYPE DATA Y LOS NOMBRES DE LAS FUNCIONES SON : %s y %s\n", f->name,dec->name);
    
    FuncStructure *result = malloc(sizeof(FuncStructure));
    int not_found = 1;

    if (scope->types)
    {
        Symbol *current_sym = scope->types;

        
        int i = 0;
        while (i < scope->t_count)
        {
            Function *current = malloc(sizeof(Function));
            current->name = current_sym->name;
            current->count_args = current_sym->type->num_params;
            current->args_types = current_sym->type->argument_types;
            current->result_types = current_sym->type;
            FunctionComparisonState *tuple = func_equals(current, f);
            if (tuple->is_match)
            {
                fprintf(stderr,"el nombre de curr_sym es %s\n",current_sym->name);

                result->state = tuple;
                result->function = current;
                return result;
            }
            if (tuple->name_matches)
            {
                not_found = 0;
                if ((!result->state && !tuple->parameters_matched) || tuple->parameters_matched)
                {
                    result->state = tuple;
                    result->function = &current;
                }
            }

            current_sym = current_sym->next;
            i++;
        }
    }

    if (scope->parent)
    {
        FuncStructure *data = find_type_data(scope->parent, f, dec);

        if (not_found || data->state->is_match)
            return data;

        return result;
    }

    if (not_found && dec)
    {
        result->state = func_equals(dec, f);
        result->function = dec;
    }
    else if (not_found && !dec)
    {
        result->state = init_state(0, -1, -1);
        result->state->name_matches = 0;
    }

    return result;
}