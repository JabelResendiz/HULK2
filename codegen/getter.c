
#include "codegen.h"

// Colores para fprintf (si los tienes definidos en otro lugar)
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define RESET "\x1B[0m"

// IMPLEMENTACIÓN DE codegen_attr_getter
LLVMValueRef codegen_attr_getter(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, RED "ESTOY EN EL CODEGEN_ATTR_GETTER\n" RESET);

    // 1.Generate code for the object expresion (self)
    // This should result in an LLVmValueRef that is a pointer to the object instance.

    fprintf(stderr, "el nombre de mi variable es %s\n", node->data.op_node.left->return_type->name);

    // const char* object_instance_ptr = node->data.op_node.left->return_type->name;

    LLVMValueRef object_instance_ptr = codegen_accept(v, node->data.op_node.left);

    if (!object_instance_ptr)
    {
        fprintf(stderr, RED "Error: No se pudo generar el código para la instancia del objeto en el getter de atributo.\n" RESET);
        return NULL;
    }

    // Debugging: Print the type of the object instance pointer
    fprintf(stderr, "DEBUG (Attr Getter): Object instance pointer type: %s\n", LLVMPrintTypeToString(LLVMTypeOf(object_instance_ptr)));

    // 2. Get the type information for the object
    // Assuming node->data.op_node.left->return_type holds the Type* for the object
    const char *object_type_name = node->data.op_node.left->return_type->name;
    LLVMUserTypeInfo *type_info = find_user_type(v->ctx, object_type_name);
    if (!type_info)
    {
        fprintf(stderr, RED "Error: No se encontró la información de tipo para '%s' en el getter de atributo.\n" RESET, object_type_name);
        return NULL;
    }
    fprintf(stderr, "DEBUG (Attr Getter): Objeto de tipo: %s\n", type_info->name);

    // 3. Get the attribute name and find its index within the struct
    const char *attribute_name = node->data.op_node.right->data.variable_name; // Assuming 'variable_name' for attributes
    int attribute_index = find_field_index(type_info, attribute_name);

    // Remember: Your struct starts with type_id (0) and vtable_ptr (1).
    // So, actual data members start from index 2.
    // Your find_field_index must account for this offset.
    // If find_field_index returns 0 for 'x' and 1 for 'y' (relative to data members),
    // then the actual LLVM struct index will be 2 for 'x', 3 for 'y'.
    // Let's assume find_field_index already returns the correct LLVM struct index (2, 3, etc.).
    if (attribute_index == -1)
    { // Assuming -1 means not found
        fprintf(stderr, RED "Error: Atributo '%s' no encontrado en el tipo '%s'.\n" RESET, attribute_name, object_type_name);
        return NULL;
    }
    fprintf(stderr, "DEBUG (Attr Getter): Atributo '%s' encontrado en el índice LLVM: %d\n", attribute_name, attribute_index);

    // 4. Get the LLVM Type of the member we are trying to access.
    // You'd need to extend LLVMTypeMemberInfo to include the LLVMTypeRef of the member.
    // Alternatively, if your find_field_index function can also return the type.
    LLVMTypeMemberInfo *member_info = find_member_info(type_info, attribute_name);

    if (!member_info)
    {
        fprintf(stderr, RED "Error interno: LLVMTypeMemberInfo para '%s' no encontrado. Esto no debería pasar si find_field_index funcionó.\n" RESET, attribute_name);
        return NULL;
    }

    LLVMTypeRef attribute_llvm_type = member_info->llvm_type;
    if (!attribute_llvm_type)
    {
        fprintf(stderr, RED "Error interno: El tipo LLVM del atributo '%s' es nulo.\n" RESET, attribute_name);
        return NULL;
    }
    fprintf(stderr, "DEBUG (Attr Getter): Tipo LLVM del atributo: %s\n", LLVMPrintTypeToString(attribute_llvm_type));

    // 5. Generate the Get Element Pointer (GEP) instruction
    // This gives us a pointer to the specific attribute within the object instance.
    LLVMValueRef attribute_ptr = LLVMBuildStructGEP2(
        v->ctx->builder,
        type_info->struct_type, // The struct type (e.g., %A)
        object_instance_ptr,    // Pointer to the object instance (e.g., %self_ptr)
        attribute_index,        // The index of the member (e.g., 2 for 'x', 3 for 'y')
        attribute_name          // Name for the instruction
    );
    fprintf(stderr, "DEBUG (Attr Getter): GEP para '%s' generada. Puntero al atributo: %s\n", attribute_name, LLVMPrintValueToString(attribute_ptr));

    // 6. Load the value from the attribute's memory address
    LLVMValueRef loaded_value = LLVMBuildLoad2(
        v->ctx->builder,
        attribute_llvm_type, // The actual LLVMTypeRef of the attribute (e.g., i32, double)
        attribute_ptr,       // Pointer to the attribute
        "loaded_attribute_val");
    fprintf(stderr, "DEBUG (Attr Getter): Valor cargado para '%s': %s\n", attribute_name, LLVMPrintValueToString(loaded_value));

    return loaded_value;
}

// LLVMValueRef codegen_attr_getter(LLVMVisitor *v, ASTNode *node)
// {
//     fprintf(stderr, RED "CODEGEN_ATTR_GETTER\n" RESET);

//     // El 'object' es la expresión base, por ejemplo, 'x' en 'x.prop'
//     LLVMValueRef object_instance_ptr = codegen_accept(v, node->data.attr_access_node.object);
//     if (!object_instance_ptr)
//     {
//         fprintf(stderr, RED "Error: Fallo al generar código para el objeto en el getter de atributo (Línea: %d).\n" RESET, node->line);
//         exit(1);
//     }

//     const char *attr_name = node->data.attr_access_node.member_name; // 'x' en 'obj.x'

//     // Obtener el tipo de la instancia del objeto.
//     // Esto es crucial para encontrar la LLVMUserTypeInfo correcta.
//     // Asumiendo que object_instance_ptr es de tipo %ClassName*
//     LLVMTypeRef obj_ptr_type = LLVMTypeOf(object_instance_ptr);
//     if (LLVMGetTypeKind(obj_ptr_type) != LLVMPointerTypeKind) {
//         fprintf(stderr, RED "Error: El objeto para el getter de atributo no es un puntero (Línea: %d).\n" RESET, node->line);
//         exit(1);
//     }
//     LLVMTypeRef obj_struct_type = LLVMGetElementType(obj_ptr_type);
//     const char* obj_type_name = LLVMGetStructName(obj_struct_type);

//     LLVMUserTypeInfo *obj_type_info = find_user_type(v->ctx, obj_type_name);
//     if (!obj_type_info)
//     {
//         fprintf(stderr, RED "Error: Información de tipo para el objeto '%s' no encontrada para el atributo '%s' (Línea: %d).\n" RESET, obj_type_name, attr_name, node->line);
//         exit(1);
//     }

//     // Encontrar la información del miembro usando su nombre
//     LLVMTypeMemberInfo *member_info = find_member_info(obj_type_info, attr_name);
//     if (!member_info)
//     {
//         fprintf(stderr, RED "Error: Miembro de datos '%s' no encontrado en la clase '%s' (Línea: %d).\n" RESET, attr_name, obj_type_name, node->line);
//         exit(1);
//     }

//     // Obtener un puntero al campo del miembro específico
//     // El índice `member_info->index` ya debería tener en cuenta el ID y el puntero a la vtable.
//     LLVMValueRef member_ptr = LLVMBuildStructGEP2(v->ctx->builder, obj_type_info->struct_type, object_instance_ptr, member_info->index, "member_ptr");

//     // Cargar el valor del puntero del miembro
//     LLVMValueRef loaded_value = LLVMBuildLoad2(v->ctx->builder, member_info->llvm_type, member_ptr, attr_name);

//     fprintf(stderr, GREEN "Getter generado con éxito para '%s.%s'.\n" RESET, obj_type_name, attr_name);
//     return loaded_value;
// }

LLVMValueRef codegen_method_getter(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, RED "Estoy en el CODEGEN_METHOD_GETTER_90\n" RESET);

    // 1. Generar el código para la expresión del objeto (por ejemplo, cargar la variable 'miObjeto')
    // Esto debe devolver un LLVMValueRef que es un puntero a la instancia del objeto.
    LLVMValueRef object_instance_ptr = codegen_accept(v, node->data.op_node.left);
    if (!object_instance_ptr)
    {
        fprintf(stderr, RED "Error: No se pudo generar el puntero de la instancia del objeto.\n" RESET);
        return NULL;
    }

    // Debugging: Print the type of the object instance pointer
    fprintf(stderr, "\nDEBUG (Attr Getter): Object instance pointer type: %s\n", LLVMPrintTypeToString(LLVMTypeOf(object_instance_ptr)));

    // Asegúrate de que el tipo de retorno de la expresión del objeto esté establecido en el AST
    // (ej. si 'miObjeto' es de tipo 'ClaseA', entonces node->data.op_node.left->return_type->name debería ser "ClaseA")
    const char *object_type_name = node->data.op_node.left->return_type->name;

    const char *object_dynamic_name = node->data.op_node.left->dynamic_type->name;

    fprintf(stderr, GREEN "el tipo real es %s y el dinmaico es %s\n" RESET, object_type_name, object_dynamic_name);

    LLVMUserTypeInfo *type_info = find_user_type(v->ctx, object_type_name);
    if (!type_info)
    {
        fprintf(stderr, RED "Error: No se encontró la información de tipo para '%s'.\n" RESET, object_type_name);
        return NULL;
    }

    LLVMUserTypeInfo *type_dynamic_info = find_user_type(v->ctx, object_dynamic_name);
    if (!type_dynamic_info)
    {
        fprintf(stderr, RED "Error: No se encontró la información de tipo dinamico para '%s'.\n" RESET, object_dynamic_name);
        return NULL;
    }

    fprintf(stderr, "2-HASTA AQUI TODO BIEN con tipo de objeto: %s\n", type_info->name);

    // El nombre del método se extrae de la parte derecha (la llamada al método)
    const char *method_name = node->data.op_node.right->data.func_node.name;

    const char *real_name_method = NULL;

    LLVMMethodInfo *method_info = NULL;
    LLVMValueRef loaded_func_ptr = NULL;

    if (strcmp(object_type_name, object_dynamic_name))
    {
        const char *base_function = delete_underscore_from_str(method_name, type_dynamic_info->name);

        real_name_method = concat_str_with_underscore(type_info->name, base_function);

        free(base_function);

        method_info = find_method_info(type_info, real_name_method);

        char vtable_global_name[256];
        snprintf(vtable_global_name, sizeof(vtable_global_name), "%s_vtable_instance", object_type_name); // e.g., "_Dog_vtable_constant"
        
        LLVMValueRef static_type_vtable_global = LLVMGetNamedGlobal(v->ctx->module, vtable_global_name);

        
        LLVMValueRef func_ptr_addr = LLVMBuildStructGEP2(
            v->ctx->builder,
            type_info->vtable_struct_type, // Use the static type's vtable struct type
            static_type_vtable_global,     // This is the global vtable constant
            method_info->vtable_index,     // Index of the method in the vtable
            "static_func_ptr_slot_addr");

        fprintf(stderr, "2-Estamo aqui\n");

        loaded_func_ptr = LLVMBuildLoad2(v->ctx->builder, LLVMPointerType(method_info->llvm_func_type, 0), func_ptr_addr, "loaded_static_func_ptr");
        LLVMSetAlignment(loaded_func_ptr, 8);
    }

    else
    {
        method_info = find_method_info(type_dynamic_info, method_name);

        LLVMValueRef vtable_ptr_field_addr = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, object_instance_ptr, 1, "vtable_ptr_addr");

        LLVMValueRef vtable_ptr_loaded = LLVMBuildLoad2(v->ctx->builder, type_info->vtable_ptr_type, vtable_ptr_field_addr, "vtable_ptr_loaded");
        LLVMSetAlignment(vtable_ptr_loaded, 8); // Adjust alignment if necessary

        LLVMValueRef func_ptr_addr = LLVMBuildStructGEP2(v->ctx->builder, type_info->vtable_struct_type, vtable_ptr_loaded, method_info->vtable_index, "func_ptr_slot_addr");
        loaded_func_ptr = LLVMBuildLoad2(v->ctx->builder, LLVMPointerType(method_info->llvm_func_type, 0), func_ptr_addr, "loaded_dynamic_func_ptr");
        LLVMSetAlignment(loaded_func_ptr, 8);
    }

    // method_name : _chihuaha_bark
    // type_info : dog
    // type_dyanmic_info : chihuahua
    // LLVMMethodInfo *method_info = find_method_info(type_info, real_name_method);
    // if (!method_info)
    // {
    //     fprintf(stderr, RED "Error: Método '%s' no encontrado para el tipo '%s'.\n" RESET, real_name_method, object_type_name);
    //     return NULL;
    // }
    fprintf(stderr, "3-HASTA AQUI TODO BIEN, método: %s encontrado en el índice: %d\n", method_info->name, method_info->vtable_index);

    // 2. Cargar el puntero a la vtable de la instancia del objeto
    // La vtable es el segundo campo (índice 1) en tu estructura de objeto personalizada.
    // LLVMValueRef vtable_ptr_field_addr = LLVMBuildStructGEP2(v->ctx->builder, type_info->struct_type, object_instance_ptr, 1, "vtable_ptr_addr");
    // fprintf(stderr,"candela\n");
    // if(type_info->vtable_ptr_type == NULL)
    // {
    //       fprintf(stderr,"candela\n");
    // }
    // LLVMValueRef vtable_ptr_loaded = LLVMBuildLoad2(v->ctx->builder, type_info->vtable_ptr_type, vtable_ptr_field_addr, "vtable_ptr_loaded");
    // fprintf(stderr,"green");
    // LLVMSetAlignment(vtable_ptr_loaded, 8); // Ajusta la alineación si es necesario

    // fprintf(stderr,"PASAMO ESTA PRUEBA BINE\n");

    // // 3. Cargar el puntero a la función específica desde la vtable
    LLVMTypeRef method_signature_type = method_info->llvm_func_type; // El tipo de función COMPLETO
    // LLVMValueRef func_ptr_addr = LLVMBuildStructGEP2(v->ctx->builder, type_info->vtable_struct_type, vtable_ptr_loaded, method_info->vtable_index, "func_ptr_slot_addr");
    // LLVMValueRef loaded_func_ptr = LLVMBuildLoad2(v->ctx->builder, LLVMPointerType(method_signature_type, 0), func_ptr_addr, "loaded_func_ptr");
    // LLVMSetAlignment(loaded_func_ptr, 8); // Ajusta la alineación si es necesario

    // 4. Preparar los argumentos para la llamada
    // El primer argumento es siempre el puntero 'this' (la instancia del objeto)
    unsigned int num_llvm_params = LLVMCountParamTypes(method_signature_type); // Número total de parámetros LLVM (incluyendo 'this')
    LLVMValueRef *call_args = (LLVMValueRef *)malloc(num_llvm_params * sizeof(LLVMValueRef));
    if (!call_args)
    {
        perror(RED "Error en malloc para call_args" RESET);
        exit(EXIT_FAILURE);
    }

    call_args[0] = object_instance_ptr; // El puntero 'this'

    // Procesar los argumentos explícitos del método desde el AST
    unsigned int ast_arg_count = node->data.op_node.right->data.func_node.arg_count;
    // Comprobar que el número de argumentos del AST coincide con los esperados por LLVM (menos 'this')
    if (ast_arg_count != (num_llvm_params - 1))
    {
        fprintf(stderr, RED "Error: Número de argumentos para el método '%s' no coincide (esperado %d, recibido %d).\n" RESET,
                real_name_method, num_llvm_params - 1, ast_arg_count);
        free(call_args);
        return NULL;
    }

    for (unsigned int i = 0; i < ast_arg_count; i++)
    {
        // Generar código para cada argumento del AST
        call_args[i + 1] = codegen_accept(v, node->data.op_node.right->data.func_node.args[i]);
        if (!call_args[i + 1])
        {
            fprintf(stderr, RED "Error: No se pudo generar el argumento %d para el método '%s'.\n" RESET, i + 1, real_name_method);
            free(call_args);
            return NULL;
        }
    }

    // 5. Construir la instrucción de llamada LLVM
    LLVMValueRef call_result = LLVMBuildCall2(v->ctx->builder, method_signature_type, loaded_func_ptr, call_args, num_llvm_params, "method_call_result");
    free(call_args);

    char *ir_string = LLVMPrintValueToString(call_result);
    fprintf(stderr, RED "call_result as string:\n%s\n" RESET, ir_string);

    return call_result; // Devuelve el resultado de la llamada al método
}