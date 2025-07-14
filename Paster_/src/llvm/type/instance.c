
#include "../codegen.h"
#include "../typeId.h"
#include "common.h"

// busca la posicion en la estrcutra de datos , considerando la herencia y campos prefijados
static int find_field_index_in_type(TypeValue* type, const char* field_name, int base_index) {
    ASTNode* type_node = type->def_node;
    if (type_node) {
        int current_index = base_index;
        for(int i = 0; i < type_node->data.typeDef.body_count; i++) {
            ASTNode* def = type_node->data.typeDef.body_elements[i];
            if (def->type == AST_ASSIGNMENT) {
                if (strcmp(def->data.binary_op.left->data.var_name, field_name) == 0) {
                    return current_index;
                }
                current_index++;
            }
        }
    }
    return -1;
}

int find_field_index(TypeValue* type,const char * name)
{
    int curr_index = 2;

    // si tiene padre , salta al puntero del padre
    if(type->super_type && !is_builtin_type(type->super_type))
    {
        curr_index ++;
    }


    // Primero entra a buscar en un current_type

    int field_idx = find_field_index_in_type(type,name,curr_index);

    if(field_idx >=0)
    {
        return field_idx;
    }

    return -1;
}

LLVMValueRef codegen_type_instance(LLVMVisitor *v, ASTNode *node)
{
    push_scope();
    // printf("Debug: Generating instance for type %s\n", node->data.type_node.name);
    const char *type_name = node->data.typeDef.name_type;
    // printf("Debug: Type name: %s\n", type_name);
    LLVMTypeRef struct_type = LLVMGetTypeByName(v->ctx->module, type_name);
    // printf("Debug: Struct type: %s\n", LLVMPrintTypeToString(struct_type));
    LLVMValueRef instance = LLVMBuildMalloc(v->ctx->builder, struct_type, "instance");
    // printf("Debug: Allocated instance: %s\n", LLVMPrintValueToString(instance));
    TypeValue *type = node->computed_type;
    ASTNode *type_def = type->def_node;
    // printf("Debug: Type definition found for %s\n", type_name);

    // Almacenamos los parámetros del constructor en el scope antes de cualquier inicialización
    for (int i = 0; i < node->data.typeDef.args_count; i++)
    {
        LLVMValueRef arg_value = codegen_accept(v, node->data.typeDef.args[i]);
        const char *param_name = type_def->data.typeDef.args[i]->data.var_name;

        // Crear alloca para el parámetro y almacenarlo en el scope
        TypeValue *param_type = node->data.typeDef.args[i]->computed_type;
        if (compare_types(param_type, &TYPE_STRING))
        {
            // For strings, store directly the i8* value
            declare_variable(param_name, arg_value);
        }
        else
        {
            // For other types, we need to handle them differently based on whether they're class types or not
            TypeValue *param_type = node->data.typeDef.args[i]->computed_type;
            if (!is_builtin_type(param_type))
            {
                // For class types, store the pointer directly
                declare_variable(param_name, arg_value);
            }
            else
            {
                // For primitive types, create alloca and store
                LLVMTypeRef alloca_type = LLVMTypeOf(arg_value);
                LLVMValueRef param_alloca = LLVMBuildAlloca(v->ctx->builder, alloca_type, "param_alloca");
                LLVMBuildStore(v->ctx->builder, arg_value, param_alloca);
                declare_variable(param_name, param_alloca);
            }
        }
        printf("Debug: Declared constructor parameter '%s' in scope\n", param_name);
    }

    // Initialize type ID field (index 0)
    LLVMValueRef id_ptr = LLVMBuildStructGEP2(v->ctx->builder, struct_type, instance, 0, "type_id_ptr");
    printf("Debug: Type ID pointer: %s\n", LLVMPrintValueToString(id_ptr));

    int id_type = get_type_id(type_def->data.typeDef.name_type);

    LLVMBuildStore(v->ctx->builder, LLVMConstInt(LLVMInt32Type(), id_type, 0), id_ptr);
    printf("Debug: Set type ID to %d\n", id_type);
    // Initialize vtable pointer (index 1)
    char vtable_name[256];
    snprintf(vtable_name, sizeof(vtable_name), "%s_vtable_instance", type_name);
    LLVMValueRef vtable_ptr = LLVMGetNamedGlobal(v->ctx->module, vtable_name);
    if (vtable_ptr)
    {
        LLVMValueRef vtable_field_ptr = LLVMBuildStructGEP2(v->ctx->builder, struct_type, instance, 1, "vtable_ptr");
        LLVMBuildStore(v->ctx->builder, vtable_ptr, vtable_field_ptr);
    }

    // Initialize parent instance if it exists (index 2)
    if (type_def->data.typeDef.name_parent[0] != '\0' && type_def->data.typeDef.parent_instance)
    {
        printf("Debug: Creating parent instance of type %s\n", type_def->data.typeDef.name_parent);

        // Generar la instancia del padre - los parámetros ya están en el scope actual
        LLVMValueRef parent_instance = codegen_accept(v, type_def->data.typeDef.parent_instance);
        if (parent_instance)
        {
            LLVMValueRef parent_ptr = LLVMBuildStructGEP2(v->ctx->builder, struct_type, instance, 2, "parent_ptr");
            LLVMBuildStore(v->ctx->builder, parent_instance, parent_ptr);
            printf("Debug: Parent instance created and stored in child\n");
        }
    }

    // Initialize constructor parameters and fields
    for (int i = 0; i < node->data.typeDef.args_count; i++)
    {
        const char *param_name = type_def->data.typeDef.args[i]->data.var_name;

        TypeValue *param_type = type_def->data.typeDef.args[i]->computed_type;
        LLVMValueRef arg_value;

        if (compare_types(param_type, &TYPE_STRING))
        {
            // For strings, just use the value directly
            arg_value = lookup_variable(param_name);
        }
        else
        {
            // For other types, load from alloca
            LLVMValueRef param_alloca = lookup_variable(param_name);
            arg_value = LLVMBuildLoad2(v->ctx->builder,
                                       LLVMGetElementType(LLVMTypeOf(param_alloca)),
                                       param_alloca,
                                       "param_val");
        }

        // LLVMValueRef arg_value = accept_gen(v, node->data.type_node.args[i]);

        int field_index = -1;

        for (int j = 0; j < type_def->data.typeDef.body_count; j++)
        {
            ASTNode *def = type_def->data.typeDef.body_elements[j];
            if (def->type == AST_ASSIGNMENT)
            {
                if (def->data.binary_op.right->type == AST_VAR &&
                    strcmp(def->data.binary_op.right->data.var_name, param_name) == 0)
                {
                    field_index = find_field_index(type, def->data.binary_op.left->data.var_name);
                    break;
                }
            }
        }
        //printf("Debug: Parameter '%s' maps to field index %d\n", param_name, field_index);
        if (field_index >= 0)
        {
            LLVMValueRef field_ptr = LLVMBuildStructGEP2(
                v->ctx->builder, struct_type, instance, field_index, "field_ptr");
            //printf("Debug: Field pointer for '%s': %s\n", param_name, LLVMPrintValueToString(field_ptr));

            // If arg_value is a pointer to double (parameter alloca), load the actual value
            if (LLVMGetTypeKind(LLVMTypeOf(arg_value)) == LLVMPointerTypeKind &&
                LLVMGetTypeKind(LLVMGetElementType(LLVMTypeOf(arg_value))) == LLVMDoubleTypeKind)
            {
                arg_value = LLVMBuildLoad2(v->ctx->builder, LLVMDoubleTypeInContext(v->ctx->context), arg_value, "param_val");
            }

            LLVMBuildStore(v->ctx->builder, arg_value, field_ptr);
        }
        //printf("Debug: Set field '%s' to value %s\n", param_name, LLVMPrintValueToString(arg_value));
    }



    // Initialize default field values for non-parameter fields
    if (node->data.typeDef.args_count == 0) {
        for (int j = 0; j < type_def->data.typeDef.body_count; j++) {
            ASTNode* def = type_def->data.typeDef.body_elements[j];
            if (def->type == AST_ASSIGNMENT) {
                if (def->data.binary_op.right->type != AST_VAR) {
                    LLVMValueRef def_val = codegen_accept(v, def->data.binary_op.right);
                    const char* field_name = def->data.binary_op.left->data.var_name;
                    int field_index = find_field_index(type, field_name);
                    if (field_index >= 0) {
                        LLVMValueRef field_ptr = LLVMBuildStructGEP2(v->ctx->builder, struct_type, instance, field_index, "field_ptr_default");
                        LLVMBuildStore(v->ctx->builder, def_val, field_ptr);
                    }
                }
            }
        }
    }
    pop_scope();
    return instance;
}