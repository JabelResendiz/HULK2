

#include "codegen.h"

LLVMValueRef codegen_attr_setter(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, RED "ESTOY EN EL CODEGEN_ATTR_SETTER\n" RESET);

    ASTNode *instance_node = node->data.cond_node.cond;

    LLVMValueRef object_instance_ptr = codegen_accept(v, instance_node);

    if (!object_instance_ptr)
    {
        fprintf(stderr, RED "Error: No se pudo generar el código para la instancia del objeto en el setter de atributo.\n" RESET);
        return NULL;
    }

    fprintf(stderr, "DEBUG (Attr Setter): Puntero a la instancia del objeto: %s\n", LLVMPrintValueToString(object_instance_ptr));

    // 2. Obtener la infomacion de tipo para objectos
    const char *object_type_name = instance_node->return_type->name;
    LLVMUserTypeInfo *type_info = find_user_type(v->ctx, object_type_name);

    if (!type_info)
    {
        fprintf(stderr, RED "Error: No se encontró la información de tipo para '%s' en el setter de atributo.\n" RESET, object_type_name);
        return NULL;
    }
    fprintf(stderr, "DEBUG (Attr Setter): Objeto de tipo: %s\n", type_info->name);

    // 3. Obtener el nombre del atributo y encontrar su indice dentro de la estructura
    // el nombre del atributo est en data.cond_node.body_true (el node del miembro)
    ASTNode *member_node = node->data.cond_node.body_true;

    if (member_node->type != NODE_VARIABLE)
    { // Asume que el miembro es un acceso a variable o un ID
        fprintf(stderr, RED "Error: El nodo del miembro en el setter no es un acceso a variable válido.\n" RESET);
        return NULL;
    }
    const char *attribute_name = member_node->data.variable_name; // Asumiendo que el nombre de la variable es el nombre del atributo

    int attribute_index = find_field_index(type_info, attribute_name);

    if (attribute_index == -1)
    {
        fprintf(stderr, RED "Error: Atributo '%s' no encontrado en el tipo '%s' para la asignación.\n" RESET, attribute_name, object_type_name);
        return NULL;
    }
    fprintf(stderr, "DEBUG (Attr Setter): Atributo '%s' encontrado en el índice LLVM: %d\n", attribute_name, attribute_index);

    // 4. Obtener la información completa del miembro (incluyendo su tipo LLVM)
    LLVMTypeMemberInfo *member_info = find_member_info(type_info, attribute_name);
    if (!member_info || !member_info->llvm_type)
    {
        fprintf(stderr, RED "Error interno: Información de tipo LLVM para el atributo '%s' es nula o no encontrada.\n" RESET, attribute_name);
        return NULL;
    }
    LLVMTypeRef attribute_llvm_type = member_info->llvm_type;
    fprintf(stderr, "DEBUG (Attr Setter): Tipo LLVM del atributo: %s\n", LLVMPrintTypeToString(attribute_llvm_type));

    // 5. Generar la instrucción Get Element Pointer (GEP)
    // Esto nos da un PUNTERO a la ubicación de memoria del atributo.
    LLVMValueRef attribute_ptr = LLVMBuildStructGEP2(
        v->ctx->builder,
        type_info->struct_type, // El tipo de la estructura (ej. %A)
        object_instance_ptr,    // Puntero a la instancia del objeto (ej. %self_ptr)
        attribute_index,        // El índice del miembro
        attribute_name          // Nombre para la instrucción
    );
    fprintf(stderr, "DEBUG (Attr Setter): GEP para '%s' generada. Puntero al atributo: %s\n", attribute_name, LLVMPrintValueToString(attribute_ptr));

    // 6. Generar código para el valor que se va a asignar (el lado derecho de la asignación)
    // El valor a asignar está en data.cond_node.body_false
    ASTNode *value_node = node->data.cond_node.body_false;
    LLVMValueRef value_to_store = codegen_accept(v, value_node);

    if (!value_to_store)
    {
        fprintf(stderr, RED "Error: No se pudo generar el código para el valor a asignar en el setter de atributo.\n" RESET);
        return NULL;
    }
    fprintf(stderr, "DEBUG (Attr Setter): Valor a almacenar: %s\n", LLVMPrintValueToString(value_to_store));

    // Opcional pero recomendado: Verificar que el tipo del valor a almacenar coincida con el tipo del atributo
    if (LLVMTypeOf(value_to_store) != attribute_llvm_type)
    {
        fprintf(stderr, YELLOW "Advertencia: Asignación de tipos no coincidentes para '%s'. Se intentará conversión implícita.\n" RESET, attribute_name);
        // Aquí podrías insertar una instrucción LLVM_BITCAST, LLVM_SEXT, etc., si tu lenguaje lo permite
        // antes de LLVMBuildStore para asegurar la compatibilidad de tipos.
        // Por ahora, asumimos que LLVM maneja conversiones básicas o que los tipos son compatibles.
    }

    // 7. Generar la instrucción Store (almacenar)
    // Esto guarda el 'value_to_store' en la dirección de memoria apuntada por 'attribute_ptr'.
    LLVMValueRef store_inst = LLVMBuildStore(
        v->ctx->builder,
        value_to_store, // El valor que se va a almacenar
        attribute_ptr   // La dirección donde se va a almacenar
    );
    fprintf(stderr, "DEBUG (Attr Setter): Instrucción Store generada: %s\n", LLVMPrintValueToString(store_inst));

    // Los setters generalmente no "retornan" un valor en LLVM IR puro,
    // pero si tu lenguaje permite encadenar asignaciones (como 'a = b = 5'),
    // podrías retornar 'value_to_store'. Si no, NULL es una opción válida.
    return value_to_store;
}