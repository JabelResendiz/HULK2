
#include "codegen.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Colores para fprintf (si los tienes definidos en otro lugar)
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define RESET "\x1B[0m"

///--- FASE 5: codegen_type_instance (Genera el LLVM IR para la creación de una instancia de un tipo) ---

LLVMValueRef codegen_type_instance(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, RED "CODEGEN_TYPE_INSTANCE\n" RESET);

    const char *class_name = node->data.type_node.name;

    LLVMUserTypeInfo *type_info = find_user_type(v->ctx, class_name);
    if (!type_info)
    {
        fprintf(stderr, RED "ERROR: Clase '%s' no encontrada para instanciación.\n" RESET, class_name);
        exit(1);
    }
    fprintf(stderr, "Generando código para instanciar la clase '%s'...\n", class_name);

    // Obtener TargetData para alineación
    LLVMTargetDataRef target_data = LLVMCreateTargetData(LLVMGetDataLayoutStr(v->ctx->module));

    // Calcular tamaño del struct
    // Esto es un patrón LLVM para calcular el tamaño de un tipo en bytes
    // Se obtiene un puntero nulo al tipo, luego se avanza 1 elemento para obtener la dirección del siguiente objeto
    // La diferencia entre 0 y esa dirección es el tamaño del tipo.
    LLVMValueRef null_class_ptr = LLVMConstNull(LLVMPointerType(type_info->struct_type, 0));
    LLVMValueRef size_gep_indices[] = {LLVMConstInt(v->ctx->i32_type, 1, 0)}; // Avanza 1 elemento
    LLVMValueRef size_ptr = LLVMBuildGEP2(v->ctx->builder, type_info->struct_type, null_class_ptr, size_gep_indices, 1, "size_ptr");
    LLVMValueRef size = LLVMBuildPtrToInt(v->ctx->builder, size_ptr, v->ctx->i64_type, "size");

    fprintf(stderr, YELLOW "1-DEBUG\n" RESET);

    // %raw_ptr = call ptr @malloc(i64 %size)
    LLVMValueRef raw_ptr = LLVMBuildCall2(v->ctx->builder, v->ctx->malloc_func_type, v->ctx->malloc_func, (LLVMValueRef[]){size}, 1, "raw_ptr");
    // %instance = bitcast ptr %raw_ptr to %A*
    LLVMValueRef instance = LLVMBuildBitCast(v->ctx->builder, raw_ptr, LLVMPointerType(type_info->struct_type, 0), "instance");

    fprintf(stderr, YELLOW "2-DEBUG\n" RESET);

    // set type id (campo 0)
    // El campo 0 de la estructura de la clase es el ID del tipo
    LLVMValueRef id_ptr = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, instance, 0, "id_ptr");
    LLVMValueRef store_id = LLVMBuildStore(v->ctx->builder, LLVMConstInt(v->ctx->i32_type, type_info->id, 0), id_ptr);
    LLVMSetAlignment(store_id, 4); // Alineación típica para i32

    fprintf(stderr, YELLOW "3-DEBUG\n" RESET);

    // set vtable (campo 1)
    // El campo 1 de la estructura de la clase es el puntero a la vtable
    LLVMValueRef vt_ptr_field = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, instance, 1, "vt_ptr_field");
    LLVMValueRef store_vtable = LLVMBuildStore(v->ctx->builder, type_info->vtable_global, vt_ptr_field);
    LLVMSetAlignment(store_vtable, 8); // Alineación típica para punteros

    // --- Inicializar miembros de datos basados en argumentos del constructor ---
    // Los miembros de datos en la estructura LLVM comienzan después del ID (campo 0) y el puntero a la vtable (campo 1).
    // Es decir, el primer miembro de datos está en el índice 2.
    LLVMTypeMemberInfo **current_member_info_init = type_info->members;

    fprintf(stderr, "El nombre de mi type info es %s con %d \n", type_info->name, type_info->num_data_members);

    int arg_idx = 0; // Índice para recorrer node->data.type_node.args

    fprintf(stderr, YELLOW "4-DEBUG\n" RESET);

    for (int i = 0; i < type_info->num_data_members ; i++)
    {
        LLVMTypeMemberInfo *member_info = type_info->members[i];

        fprintf(stderr, "  Procesando miembro '%s' (indice LLVM: %d).\n", member_info->name, member_info->index);

        if (!member_info->default_value_node)
        {
            fprintf(stderr, RED "ERROR FATAL: member_info->default_value_node es NULL para miembro '%s'!\n" RESET, member_info->name);
            exit(1);
        }

        if(member_info->default_value_node->type == NODE_VARIABLE)
        {
            continue;
        }

        LLVMValueRef initial_val_llvm = codegen_accept(v, member_info->default_value_node);

        if (!initial_val_llvm)
        {
            fprintf(stderr, RED "ERROR: Fallo al generar LLVMValue para el valor inicial del miembro '%s'. codegen_accept devolvió NULL.\n" RESET, member_info->name);
            exit(1);
        }

        LLVMValueRef member_field_ptr = LLVMBuildStructGEP2(
            v->ctx->builder,
            type_info->struct_type,
            instance,
            member_info->index,
            member_info->name);

        LLVMValueRef store_inst = LLVMBuildStore(v->ctx->builder, initial_val_llvm, member_field_ptr);
        LLVMSetAlignment(store_inst, LLVMABISizeOfType(target_data, member_info->llvm_type));

        char *val_str = LLVMPrintValueToString(initial_val_llvm);
        fprintf(stderr, "  Miembro '%s' inicializado con éxito. Valor LLVM: %s\n", member_info->name, val_str);
        LLVMDisposeMessage(val_str);
    }

    fprintf(stderr, "Inicialización de todos los miembros de datos completada para '%s'.\n", class_name);

    // deberías mapear los argumentos del constructor
    // a los miembros de la clase según la semántica de tu lenguaje.
    for (int i = 0; i < type_info->num_data_members && arg_idx < node->data.type_node.arg_count; i++)
    {
        ASTNode *constructor_arg_node = node->data.type_node.args[arg_idx];

        LLVMValueRef arg_val_llvm = codegen_accept(v, constructor_arg_node);

        if (!arg_val_llvm)
        {
            fprintf(stderr, RED "ERROR: Fallo al generar código para el argumento %d del constructor (Línea: %d).\n" RESET, arg_idx, constructor_arg_node->line);
            exit(1);
        }


        fprintf(stderr, "ESTAMOS AQUIIIII\n");

        fprintf(stderr, RED "el indice es type_info %d de nombre %s \n" RESET, current_member_info_init[i]->index, current_member_info_init[i]->name);
        LLVMValueRef member_field_ptr = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, instance, current_member_info_init[i]->index, "member_field_ptr");

        char *member_field_ptr_str = LLVMPrintValueToString(member_field_ptr); // Get string representation
        fprintf(stderr, GREEN "el member_field_ptr es %s\n" RESET, member_field_ptr_str);
        LLVMDisposeMessage(member_field_ptr_str); // Free the allocated string\


        LLVMValueRef store_member = LLVMBuildStore(v->ctx->builder, arg_val_llvm, member_field_ptr);

        char *store_field_ptr_str = LLVMPrintValueToString(store_member); // Get string representation
        fprintf(stderr, GREEN "el store_memeber es %s\n" RESET, store_field_ptr_str);
        LLVMDisposeMessage(store_field_ptr_str); // Free the allocated string\

        // La alineación es importante para el rendimiento.
        LLVMSetAlignment(store_member, LLVMABISizeOfType(target_data, current_member_info_init[i]->llvm_type));

        // current_member_info_init = current_member_info_init->next;
        arg_idx++;
        fprintf(stderr, YELLOW "5-DEBUG\n" RESET);
    }

    
    fprintf(stderr, YELLOW "6-DEBUG\n" RESET);

    LLVMValueRef vtable_ptr_loaded = LLVMBuildLoad2(v->ctx->builder, LLVMPointerType(type_info->vtable_struct_type, 0), vt_ptr_field, "vtable_ptr_loaded");
    LLVMSetAlignment(vtable_ptr_loaded, 8); // Alineación de punteros

    fprintf(stderr, YELLOW "7-DEBUG\n" RESET);

    // Itera a través de la lista enlazada de métodos almacenada en type_info
    LLVMMethodInfo **current_method_info = type_info->methods;
    fprintf(stderr, YELLOW "8-DEBUG\n" RESET);


    fprintf(stderr, YELLOW "9-DEBUG\n" RESET);

    LLVMDisposeTargetData(target_data);
    return instance;
}

LLVMValueRef codegen_type_dec(LLVMVisitor *v, ASTNode *node)
{
    return NULL; // Esta función se mantiene igual, no genera código directamente
}
