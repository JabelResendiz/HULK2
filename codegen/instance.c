
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


// --- FASE 5: codegen_type_instance (Genera el LLVM IR para la creación de una instancia de un tipo) ---
LLVMValueRef codegen_type_instance(LLVMVisitor* v, ASTNode* node) {
    fprintf(stderr, RED "CODEGEN_TYPE_INSTANCE\n" RESET);

    const char *class_name = node->data.type_node.name;

    LLVMUserTypeInfo *type_info = find_user_type(v->ctx, class_name);
    if (!type_info) {
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
    LLVMValueRef size_gep_indices[] = { LLVMConstInt(v->ctx->i32_type, 1, 0) }; // Avanza 1 elemento
    LLVMValueRef size_ptr = LLVMBuildGEP2(v->ctx->builder, type_info->struct_type, null_class_ptr, size_gep_indices, 1, "size_ptr");
    LLVMValueRef size = LLVMBuildPtrToInt(v->ctx->builder, size_ptr, v->ctx->i64_type, "size");

    fprintf(stderr,YELLOW "1-DEBUG\n" RESET);

    // %raw_ptr = call ptr @malloc(i64 %size)
    LLVMValueRef raw_ptr = LLVMBuildCall2(v->ctx->builder, v->ctx->malloc_func_type, v->ctx->malloc_func, (LLVMValueRef[]){size}, 1, "raw_ptr");
    // %instance = bitcast ptr %raw_ptr to %A*
    LLVMValueRef instance = LLVMBuildBitCast(v->ctx->builder, raw_ptr, LLVMPointerType(type_info->struct_type, 0), "instance");

    fprintf(stderr,YELLOW "2-DEBUG\n" RESET);
    
    // set type id (campo 0)
    // El campo 0 de la estructura de la clase es el ID del tipo
    LLVMValueRef id_ptr = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, instance, 0, "id_ptr");
    LLVMValueRef store_id = LLVMBuildStore(v->ctx->builder, LLVMConstInt(v->ctx->i32_type, type_info->id, 0), id_ptr);
    LLVMSetAlignment(store_id, 4); // Alineación típica para i32

    fprintf(stderr,YELLOW "3-DEBUG\n" RESET);
    
    // set vtable (campo 1)
    // El campo 1 de la estructura de la clase es el puntero a la vtable
    LLVMValueRef vt_ptr_field = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, instance, 1, "vt_ptr_field");
    LLVMValueRef store_vtable = LLVMBuildStore(v->ctx->builder, type_info->vtable_global, vt_ptr_field);
    LLVMSetAlignment(store_vtable, 8); // Alineación típica para punteros

    // --- Inicializar miembros de datos basados en argumentos del constructor ---
    // Los miembros de datos en la estructura LLVM comienzan después del ID (campo 0) y el puntero a la vtable (campo 1).
    // Es decir, el primer miembro de datos está en el índice 2.
    // Asumo que 'type_info->members' es una lista enlazada ORDENADA según los índices de los campos.
    LLVMTypeMemberInfo *current_member_info_init = type_info->members;
    int arg_idx = 0; // Índice para recorrer node->data.type_node.args

    fprintf(stderr,YELLOW "4-DEBUG\n" RESET);
    
    // Asegúrate de que los argumentos del constructor coincidan con los miembros de datos.
    // Esto es una simplificación; en un compilador real, deberías mapear los argumentos del constructor
    // a los miembros de la clase según la semántica de tu lenguaje.
    while (current_member_info_init != NULL && arg_idx < node->data.type_node.arg_count) {
        ASTNode* constructor_arg_node = node->data.type_node.args[arg_idx];

        LLVMValueRef arg_val_llvm = NULL;

        if (constructor_arg_node->return_type) {
            if(type_equals(constructor_arg_node->return_type, &TYPE_NUMBER)) {
                // Asumo que TYPE_NUMBER mapea a double
                arg_val_llvm = LLVMConstReal(v->ctx->double_type, constructor_arg_node->data.number_value);
            } else if(type_equals(constructor_arg_node->return_type, &TYPE_STRING)) {
                // Si es un string, crea una cadena global
                arg_val_llvm = createGlobalString(v->ctx, constructor_arg_node->data.string_value, "ctor_str_arg");
                // Asegúrate de que el tipo sea ptr_type (i8*) si tu struct almacena strings como i8*
                arg_val_llvm = LLVMBuildPointerCast(v->ctx->builder, arg_val_llvm, v->ctx->ptr_type, "str_arg_ptr_cast");
            } else if(type_equals(constructor_arg_node->return_type, &TYPE_BOOLEAN)) { // CORREGIDO: TYPE_BOOLEAN
                // Asumo que TYPE_BOOLEAN mapea a i1
                arg_val_llvm = LLVMConstInt(v->ctx->i1_type, constructor_arg_node->data.string_value ? 1 : 0, 0); // Asumiendo boolean_value en ASTNode
            } else {
                // Aquí podrías manejar otros tipos (ej. instancias de otras clases)
                // Esto requeriría una llamada recursiva a codegen_type_instance o codegen_accept
                fprintf(stderr, RED "ERROR: Tipo de argumento de constructor no soportado para inicialización directa: %s (Línea: %d).\n" RESET, constructor_arg_node->return_type->name, constructor_arg_node->line);
                exit(1);
            }
        } else {
            fprintf(stderr, RED "ERROR: Argumento de constructor sin tipo computado (Línea: %d).\n" RESET, constructor_arg_node->line);
            exit(1);
        }

        // Generar la instrucción STORE para el miembro de la clase
        // current_member_info_init->index debe ser el índice del campo LLVM,
        // que debería ser 2 para el primer miembro de datos, 3 para el segundo, etc.
        // Asumo que find_struct_fields (o el proceso que llena type_info->members)
        // asigna los índices correctamente (0 para id, 1 para vtable, luego 2, 3, ... para miembros).
        LLVMValueRef member_field_ptr = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, instance, current_member_info_init->index, "member_field_ptr");
        LLVMValueRef store_member = LLVMBuildStore(v->ctx->builder, arg_val_llvm, member_field_ptr);
        // La alineación es importante para el rendimiento.
        LLVMSetAlignment(store_member, LLVMABISizeOfType(target_data, current_member_info_init->llvm_type));

        current_member_info_init = current_member_info_init->next;
        arg_idx++;
        fprintf(stderr,YELLOW "5-DEBUG\n" RESET);
    
    }

    // --- LLAMADAS A MÉTODOS DE LA VTABLE (iterando dinámicamente) ---
    // Esto simula la llamada al constructor o métodos inicializadores.
    // Esto es muy específico; en un lenguaje OO típico, el constructor
    // sería una función explícita que se llamaría aquí, o los métodos
    // serían llamados explícitamente en el código del usuario.
    // Si esta sección es solo para depuración/demostración, está bien.
    fprintf(stderr,YELLOW "6-DEBUG\n" RESET);
    
    LLVMValueRef vtable_ptr_loaded = LLVMBuildLoad2(v->ctx->builder, LLVMPointerType(type_info->vtable_struct_type, 0), vt_ptr_field, "vtable_ptr_loaded");
    LLVMSetAlignment(vtable_ptr_loaded, 8); // Alineación de punteros

    fprintf(stderr,YELLOW "7-DEBUG\n" RESET);
    
    // Itera a través de la lista enlazada de métodos almacenada en type_info
    LLVMMethodInfo *current_method_info = type_info->methods;
    fprintf(stderr,YELLOW "8-DEBUG\n" RESET);
    
    while (current_method_info != NULL) {
        const char* method_name = current_method_info->name;
        // El vtable_index debería coincidir con el orden en que se definió la vtable.
        int vtable_index = current_method_info->vtable_index;

        // Obtener el puntero de la función del slot de la vtable
        LLVMValueRef func_ptr_addr = LLVMBuildStructGEP2(v->ctx->builder, type_info->vtable_struct_type, vtable_ptr_loaded, vtable_index, "func_ptr_addr");
        
        fprintf(stderr,YELLOW "10-DEBUG\n" RESET);
    
        // El method_signature_type viene directamente de la información pre-calculada
        LLVMTypeRef method_signature_type = current_method_info->llvm_func_type;
        
        if(method_signature_type == NULL)
        {
            fprintf(stderr, RED "ESTA CRITICO ESTO\n" RESET);
        }

        LLVMValueRef loaded_func_ptr = LLVMBuildLoad2(v->ctx->builder, LLVMPointerType(method_signature_type, 0), func_ptr_addr, "loaded_func_ptr");
        LLVMSetAlignment(loaded_func_ptr, 8);

        // Obtener el número de parámetros esperado por la función LLVM
        unsigned int num_llvm_params = LLVMCountParamTypes(method_signature_type);
        fprintf(stderr,YELLOW "12-DEBUG: el numero de parametros es %d\n" RESET, num_llvm_params);
    
        LLVMValueRef *call_args = (LLVMValueRef*)malloc(num_llvm_params * sizeof(LLVMValueRef));
        fprintf(stderr,YELLOW "11-DEBUG: el numero de parametros es %d\n \n" RESET, num_llvm_params);
    
        if (!call_args) { perror(RED "Error en malloc para call_args" RESET); exit(EXIT_FAILURE); }
        
        fprintf(stderr,YELLOW "12-DEBUG\n" RESET);
    
        // El primer argumento es siempre el puntero 'this' (la instancia actual)
        call_args[0] = instance;

        // --- Manejo de argumentos para la llamada al método ---
        // ESTA SECCIÓN NECESITA SER DINÁMICA y basada en la lógica de tu lenguaje.
        // El ejemplo de 'f' es una excepción codificada.
        // Si tu constructor llama a métodos con argumentos, esos argumentos deben
        // generarse dinámicamente aquí.
        if (strcmp(method_name, "f") == 0) {
            // Ejemplo para 'f' si toma 2 argumentos 'double' después de 'this'
            if (num_llvm_params >= 3) { // 'this' (1) + arg1 (1) + arg2 (1) = 3
                call_args[1] = LLVMConstReal(v->ctx->double_type, 10.0);
                call_args[2] = LLVMConstReal(v->ctx->double_type, 20.0);
            } else {
                fprintf(stderr, YELLOW "WARNING: El método 'f' en la clase '%s' esperaba 2 argumentos después de 'this', pero su firma LLVM tiene solo %u parámetros.\n" RESET, class_name, num_llvm_params - 1);
            }
        }
        // Para otros métodos, si tienen parámetros, ¿de dónde vienen sus valores?
        // Esto es un punto clave de diseño de tu compilador.
        // Si `num_llvm_params > 1` y no tienes argumentos definidos, podrías pasar `LLVM_UndefValue` o `LLVMConstNull`.


        LLVMValueRef call_result = LLVMBuildCall2(v->ctx->builder, method_signature_type, loaded_func_ptr, call_args, num_llvm_params, "call_result");
        free(call_args);

        // --- Impresión del resultado (para depuración) ---
        // Esto es solo para depuración; en un compilador real, no siempre imprimirías el resultado de una llamada a método.
        LLVMTypeKind return_kind = LLVMGetTypeKind(LLVMGetReturnType(method_signature_type));
        if (return_kind == LLVMDoubleTypeKind) {
            LLVMValueRef format_str_double = createGlobalString(v->ctx, "%lf\n", "format_double_str");
            LLVMValueRef format_ptr_indices[] = { LLVMConstInt(v->ctx->i64_type, 0, 0), LLVMConstInt(v->ctx->i64_type, 0, 0) };
            LLVMValueRef format_ptr = LLVMBuildInBoundsGEP2(v->ctx->builder, LLVMGlobalGetValueType(format_str_double), format_str_double, format_ptr_indices, 2, "format_ptr_double");
            LLVMValueRef printf_args[] = { format_ptr, call_result };
            LLVMBuildCall2(v->ctx->builder, v->ctx->printf_func_type, v->ctx->printf_func, printf_args, 2, "");
            fprintf(stderr, "DEBUG: Llamada a método '%s' e impresión de resultado (double).\n", method_name);
        } else if (return_kind == LLVMIntegerTypeKind) {
            LLVMValueRef format_ptr_indices[] = { LLVMConstInt(v->ctx->i64_type, 0, 0), LLVMConstInt(v->ctx->i64_type, 0, 0) };
            LLVMValueRef format_ptr = LLVMBuildInBoundsGEP2(v->ctx->builder, LLVMGlobalGetValueType(v->ctx->format_string_global), v->ctx->format_string_global, format_ptr_indices, 2, "format_ptr_int");
            LLVMValueRef printf_args[] = { format_ptr, call_result };
            LLVMBuildCall2(v->ctx->builder, v->ctx->printf_func_type, v->ctx->printf_func, printf_args, 2, "");
            fprintf(stderr, "DEBUG: Llamada a método '%s' e impresión de resultado (int).\n", method_name);
        } else if (return_kind == LLVMVoidTypeKind) {
            fprintf(stderr, "DEBUG: Llamada a método '%s' (void).\n", method_name);
        }

        // Avanza al siguiente método en la lista
        current_method_info = current_method_info->next;
    }
    
    fprintf(stderr,YELLOW "9-DEBUG\n" RESET);
    
    LLVMDisposeTargetData(target_data);
    return instance;
}

LLVMValueRef codegen_type_dec(LLVMVisitor* v,ASTNode*node)
{
    return NULL; // Esta función se mantiene igual, no genera código directamente
}

// // FASE 5: codegen_type_instance (Genera el LLVM IR para la creación de una instancia de un tipo)
// LLVMValueRef codegen_type_instance(LLVMVisitor* v, ASTNode* node) {

//     fprintf(stderr,"ESTOY EN LA SECCION DE CODEGEN-TYPE-INSTANCE\n");

//     const char *class_name = node->data.type_node.name;

//     LLVMUserTypeInfo *type_info = find_user_type(v->ctx, class_name);
//     if (!type_info) {
//         fprintf(stderr, RED "ERROR: Clase '%s' no encontrada para instanciación.\n" RESET, class_name);
//         exit(1);
//     }

//     fprintf(stderr, "Generando código para instanciar la clase '%s'...\n", class_name);

//     // Obtener TargetData para alineación
//     LLVMTargetDataRef target_data = LLVMCreateTargetData(LLVMGetDataLayoutStr(v->ctx->module)); // CORREGIDO: Obtener TargetData

//     // Calcular tamaño del struct
//     LLVMValueRef null_class_ptr = LLVMConstNull(LLVMPointerType(type_info->struct_type, 0));
//     LLVMValueRef size_gep_indices[] = { LLVMConstInt(v->ctx->i32_type, 1, 0) };
//     LLVMValueRef size_ptr = LLVMBuildGEP2(v->ctx->builder, type_info->struct_type, null_class_ptr, size_gep_indices, 1, "size_ptr");
//     LLVMValueRef size = LLVMBuildPtrToInt(v->ctx->builder, size_ptr, v->ctx->i64_type, "size");

//     // %raw_ptr = call ptr @malloc(i64 %size)
//     LLVMValueRef raw_ptr = LLVMBuildCall2(v->ctx->builder, v->ctx->malloc_func_type, v->ctx->malloc_func, (LLVMValueRef[]){size}, 1, "raw_ptr");
//     // %instance = bitcast ptr %raw_ptr to %A*
//     LLVMValueRef instance = LLVMBuildBitCast(v->ctx->builder, raw_ptr, LLVMPointerType(type_info->struct_type, 0), "instance");

//     // set type id (campo 0)
//     LLVMValueRef id_ptr = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, instance, 0, "id_ptr");
//     LLVMValueRef store_id = LLVMBuildStore(v->ctx->builder, LLVMConstInt(v->ctx->i32_type, type_info->id, 0), id_ptr);
//     LLVMSetAlignment(store_id, 4);

//     // set vtable (campo 1)
//     LLVMValueRef vt_ptr_field = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, instance, 1, "vt_ptr_field");
//     LLVMValueRef store_vtable = LLVMBuildStore(v->ctx->builder, type_info->vtable_global, vt_ptr_field);
//     LLVMSetAlignment(store_vtable, 8);

//     // --- Inicializar miembros de datos basados en argumentos del constructor ---
//     LLVMTypeMemberInfo *current_member_info_init = type_info->members;
//     int arg_idx = 0; // Índice para recorrer node->data.type_node.args

//     while (current_member_info_init != NULL && arg_idx < node->data.type_node.arg_count) {
//         ASTNode* constructor_arg_node = node->data.type_node.args[arg_idx];

//         LLVMValueRef arg_val_llvm = NULL;

//         if (constructor_arg_node->return_type) {
//             if(type_equals(constructor_arg_node->return_type,&TYPE_NUMBER))
//             {
//                 arg_val_llvm = LLVMConstReal(v->ctx->double_type, constructor_arg_node->data.number_value);
                
//             }
//               if(type_equals(constructor_arg_node->return_type,&TYPE_STRING))
//             {      
//                     arg_val_llvm = createGlobalString(v->ctx, constructor_arg_node->data.string_value, "ctor_str_arg"); // CORREGIDO: createGlobalString
//                     arg_val_llvm = LLVMBuildPointerCast(v->ctx->builder, arg_val_llvm, v->ctx->ptr_type, "str_arg_ptr_cast");
                    
//             }
//                if(type_equals(constructor_arg_node->return_type,&TYPE_STRING))
//             {  
                
//                     arg_val_llvm = LLVMConstInt(v->ctx->i1_type, constructor_arg_node->data.string_value ? 1 : 0, 0); // CORREGIDO: i1_type y boolean_value
                 
//             }
//         } else {
//             fprintf(stderr, RED "ERROR: Argumento de constructor sin tipo computado (Línea: %d).\n" RESET, constructor_arg_node->line);
//             exit(1);
//         }

//         // Generar la instrucción STORE para el miembro de la clase
//         LLVMValueRef member_field_ptr = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, instance, current_member_info_init->index, "member_field_ptr");
//         LLVMValueRef store_member = LLVMBuildStore(v->ctx->builder, arg_val_llvm, member_field_ptr);
//         LLVMSetAlignment(store_member, LLVMABISizeOfType(target_data, current_member_info_init->llvm_type)); // CORREGIDO: Uso de target_data

//         current_member_info_init = current_member_info_init->next;
//         arg_idx++;
//     }

//     // --- LLAMADAS A MÉTODOS DE LA VTABLE (iterando dinámicamente) ---
//     LLVMValueRef vtable_ptr_loaded = LLVMBuildLoad2(v->ctx->builder, type_info->vtable_ptr_type, vt_ptr_field, "vtable_ptr_loaded");
//     LLVMSetAlignment(vtable_ptr_loaded, 8);

//     for (int i = 0; i < type_info->num_methods_virtual; i++) {
//         LLVMMethodInfo *method_info = type_info->methods[i]
        
//         const char* method_name = method_info->name;
//         int num_params_ast = method_node_ast->data.func_node.arg_count;

//         LLVMValueRef func_ptr_addr = LLVMBuildStructGEP2(v->ctx->builder, type_info->vtable_struct_type, vtable_ptr_loaded, i, "func_ptr_addr");
        
//         LLVMTypeRef method_signature_type = get_llvm_method_func_type(v->ctx, method_node_ast); // CORREGIDO: get_llvm_method_func_type declarada
        
//         LLVMValueRef loaded_func_ptr = LLVMBuildLoad2(v->ctx->builder, LLVMPointerType(method_signature_type, 0), func_ptr_addr, "loaded_func_ptr");
//         LLVMSetAlignment(loaded_func_ptr, 8);

//         LLVMValueRef *call_args = (LLVMValueRef*)malloc((1 + num_params_ast) * sizeof(LLVMValueRef));
//         if (!call_args) { perror(RED "Error en malloc para call_args" RESET); exit(EXIT_FAILURE); }
//         call_args[0] = instance;

//         // Llenar los argumentos adicionales si el método los tiene
//         if (strcmp(method_name, "f") == 0) {
//             if (num_params_ast >= 2) {
//                 call_args[1] = LLVMConstReal(v->ctx->double_type, 10.0);
//                 call_args[2] = LLVMConstReal(v->ctx->double_type, 20.0);
//             } else {
//                 fprintf(stderr, YELLOW "WARNING: Método 'f' esperado con 2 argumentos, pero el AST indica %d. (Línea: %d)\n" RESET, num_params_ast, method_node_ast->line);
//             }
//         }

//         LLVMValueRef call_result = LLVMBuildCall2(v->ctx->builder, method_signature_type, loaded_func_ptr, call_args, 1 + num_params_ast, "call_result");
//         free(call_args);

//         LLVMTypeKind return_kind = LLVMGetTypeKind(LLVMGetReturnType(method_signature_type));
//         if (return_kind == LLVMDoubleTypeKind) {
//             LLVMValueRef format_str_double = createGlobalString(v->ctx, "%lf\n", "format_double_str"); // CORREGIDO: createGlobalString
//             LLVMValueRef format_ptr_indices[] = { LLVMConstInt(v->ctx->i64_type, 0, 0), LLVMConstInt(v->ctx->i64_type, 0, 0) };
//             LLVMValueRef format_ptr = LLVMBuildInBoundsGEP2(v->ctx->builder, LLVMGlobalGetValueType(format_str_double), format_str_double, format_ptr_indices, 2, "format_ptr_double");
//             LLVMValueRef printf_args[] = { format_ptr, call_result };
//             LLVMBuildCall2(v->ctx->builder, v->ctx->printf_func_type, v->ctx->printf_func, printf_args, 2, "");
//             fprintf(stderr, "DEBUG: Llamada a método '%s' e impresión de resultado (double).\n", method_name);
//         } else if (return_kind == LLVMIntegerTypeKind) {
//             LLVMValueRef format_ptr_indices[] = { LLVMConstInt(v->ctx->i64_type, 0, 0), LLVMConstInt(v->ctx->i64_type, 0, 0) };
//             LLVMValueRef format_ptr = LLVMBuildInBoundsGEP2(v->ctx->builder, LLVMGlobalGetValueType(v->ctx->format_string_global), v->ctx->format_string_global, format_ptr_indices, 2, "format_ptr_int");
//             LLVMValueRef printf_args[] = { format_ptr, call_result };
//             LLVMBuildCall2(v->ctx->builder, v->ctx->printf_func_type, v->ctx->printf_func, printf_args, 2, "");
//             fprintf(stderr, "DEBUG: Llamada a método '%s' e impresión de resultado (int).\n", method_name);
//         } else if (return_kind == LLVMVoidTypeKind) {
//              fprintf(stderr, "DEBUG: Llamada a método '%s' (void).\n", method_name);
//         }
//     }
//     LLVMDisposeTargetData(target_data); // ¡Importante liberar!
//     return instance;
// }


// LLVMValueRef codegen_type_dec(LLVMVisitor* v,ASTNode*node)
// {
//     return NULL;
// }


