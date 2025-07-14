
#include "../codegen.h"
#include "../typeId.h"

LLVMValueRef codegen_type_dec(LLVMVisitor *v, ASTNode *node)
{

    fprintf(stderr,"Estoy en el delcaracion de tipo\n");

    const char *type_name = node->data.typeDef.name_type;
    register_type_id(type_name, next_type_id++);

    push_scope();

    
    for (int i = 0; i < node->data.typeDef.args_count; i++)
    {
        ASTNode *param = node->data.typeDef.args[i];
        LLVMTypeRef param_type = type_to_llvm(v,param->computed_type);

        LLVMValueRef param_alloca = LLVMBuildAlloca(v->ctx->builder, param_type, param->data.var_name);
        declare_variable(param->data.var_name, param_alloca);
    }

    LLVMTypeRef struct_type = LLVMGetTypeByName(v->ctx->module, type_name);

    if (!struct_type)
    {
        struct_type = LLVMStructCreateNamed(v->ctx->context, type_name);
    }

    // contar la cnatidad de alloca
    int field_count = 0;

    for (int i = 0; i < node->data.typeDef.body_count; i++)
    {
        ASTNode *def = node->data.typeDef.body_elements[i];
        if (def->type == AST_ASSIGNMENT)
        {
            field_count++;
        }
    }

    // Crear un array para campos de tipo

    int extra_fields = node->data.typeDef.name_parent[0] != '\0' ? 3 : 2;
    LLVMTypeRef *field_types = malloc((field_count + extra_fields) * sizeof(LLVMTypeRef));
    int field_idx = 0;

    // agregar el tipo ID como un primer campo (32 bits entero)
    field_types[field_idx++] = LLVMInt32Type();

    // agregar un puntero a la vtbla como secgundo parametro
    char vtable_type_name[256];
    snprintf(vtable_type_name, sizeof(vtable_type_name), "%s_vtable", type_name);

    LLVMTypeRef vtable_type = LLVMGetTypeByName(v->ctx->module, vtable_type_name);

    if (!vtable_type)
    {
        vtable_type = LLVMStructCreateNamed(v->ctx->context, vtable_type_name);
    }

    field_types[field_idx++] = LLVMPointerType(vtable_type, 0);

    // agregar puntero de los padres
    if (node->data.typeDef.name_parent[0] != '\0')
    {
        LLVMTypeRef parent_struct_type = LLVMGetTypeByName(v->ctx->module, node->data.typeDef.name_parent);

        if (!parent_struct_type)
        {
            parent_struct_type = LLVMStructCreateNamed(v->ctx->context, node->data.typeDef.name_parent);
        }
        field_types[field_idx++] = LLVMPointerType(parent_struct_type, 0);
    }

    // para campos de tipos

    for (int i = 0; i < node->data.typeDef.body_count; i++)
    {
        ASTNode *def = node->data.typeDef.body_elements[i];

        if (def->type == AST_ASSIGNMENT)
        {
            TypeValue *field_type = def->data.binary_op.right->computed_type;
            field_types[field_idx++] = type_to_llvm(v,field_type);
        }
    }

    // Set the body of the struct (even if it was a forward declaration)
    LLVMStructSetBody(struct_type, field_types, field_idx, 0);
    free(field_types);

    // Generate methods for this type
    generate_type_methods(v, type_name, struct_type, node);
    printf("Debug: Generated struct type %s\n", type_name);

    pop_scope();
    return NULL;
}

void generate_type_methods(LLVMVisitor *v, const char *type_name, LLVMTypeRef struct_type, ASTNode *type_node)
{
    
    LLVMBuilderRef saved_builder = v->ctx->builder;
    v->ctx->builder = LLVMCreateBuilder();

    int method_count = 0;
    for (int i = 0; i < type_node->data.typeDef.body_count; i++)
    {
        ASTNode *def = type_node->data.typeDef.body_elements[i];

        if (def->type == AST_DECL_FUNC)
        {
            method_count++;
        }
    }

    if (method_count == 0)
    {
        printf("Debug: No methods to generate for type %s\n", type_name);
        LLVMDisposeBuilder(v->ctx->builder);
        v->ctx->builder = saved_builder;
        printf("Debug: Exiting method generation for type %s\n", type_name);
        return;
    }

    fprintf(stderr,"ESTAMOS AQUI\n");

    // Arrays for storing method information
    LLVMTypeRef *vtable_fn_types = malloc(method_count * sizeof(LLVMTypeRef));
    LLVMValueRef *method_ptrs = malloc(method_count * sizeof(LLVMValueRef));
    char **method_names = malloc(method_count * sizeof(char *));
    int method_idx = 0;

    fprintf(stderr,"ESTAMOS AQUI\n");
    
    for (int i = 0; i < type_node->data.typeDef.body_count; i++)
    {

        ASTNode *def = type_node->data.typeDef.body_elements[i];

        if (def->type == AST_DECL_FUNC)
        {
            method_names[method_idx] = strdup(def->data.func_node.name);

            int param_count = def->data.func_node.arg_count + 1;

            LLVMTypeRef *param_types = malloc(param_count * sizeof(LLVMTypeRef));

            param_types[0] = LLVMPointerType(struct_type, 0);

            for (int j = 0; j < def->data.func_node.arg_count; j++)
            {
                param_types[j + 1] = type_to_llvm(v,def->data.func_node.args[j]->computed_type);
            }

            // Get return type
            LLVMTypeRef return_type;
            if (def->data.func_node.body->computed_type)
            {
                return_type = type_to_llvm(v,def->data.func_node.body->computed_type);
                // printf("Debug: Method %s return type is %s\n",
                //        def->data.func_node.name, def->data.func_node.body->return_type->name);
            }
            else
            {
                // printf("Warning: No return type specified for method %s, using void\n",
                //        def->data.func_node.name);
                return_type = LLVMVoidType();
            }

            // Create function type and store it for vtable
            LLVMTypeRef func_type = LLVMFunctionType(return_type, param_types, param_count, 0);
            vtable_fn_types[method_idx] = LLVMPointerType(func_type, 0);

            // printf("Debug: Created function type for method %s: %s\n",
            //        def->data.func_node.name, LLVMPrintTypeToString(func_type));

            // Add function declaration with original name
            LLVMValueRef func = LLVMAddFunction(v->ctx->module, def->data.func_node.name, func_type);
            method_ptrs[method_idx] = func;

            // Generate method body
            LLVMBasicBlockRef entry = LLVMAppendBasicBlock(func, "entry");
            LLVMPositionBuilderAtEnd(v->ctx->builder, entry);

            // Store old scope and create new one
            push_scope();

            // Add 'this' pointer to scope
            LLVMValueRef this_ptr = LLVMGetParam(func, 0);
            declare_variable("self", this_ptr);

            // Add parameters to scope
            for (int j = 0; j < def->data.func_node.arg_count; j++)
            {
                LLVMValueRef param = LLVMGetParam(func, j + 1);
                declare_variable(def->data.func_node.args[j]->data.var_name, param);
            }

            // Generate body
            LLVMValueRef result = codegen_accept(v, def->data.func_node.body);

            if (!result)
            {
                // printf("Error: Method %s body returned NULL\n", def->data.func_node.name);
                LLVMBuildRetVoid(v->ctx->builder);
            }
            else
            {
                // printf("Debug: Method %s returned value of type: %s\n",
                //        def->data.func_node.name,
                //        LLVMPrintTypeToString(LLVMTypeOf(result)));

                if (LLVMGetTypeKind(return_type) == LLVMVoidTypeKind)
                {
                    LLVMBuildRetVoid(v->ctx->builder);
                }
                else
                {
                    LLVMBuildRet(v->ctx->builder, result);
                }
            }

            pop_scope();
            free(param_types);
            method_idx++;
        }
    }

    fprintf(stderr,"ESTAMOS AQUI\n");
    
    // Get the existing vtable type and set its body
    char vtable_type_name[256];
    snprintf(vtable_type_name, sizeof(vtable_type_name), "%s_vtable", type_name);
    //printf("Debug: Setting vtable type %s body with %d methods\n", vtable_type_name, method_count);

    LLVMTypeRef vtable_type = LLVMGetTypeByName(v->ctx->module, vtable_type_name);
    if (!vtable_type)
    {
        //printf("Error: Could not find vtable type %s\n", vtable_type_name);
        return;
    }
    LLVMStructSetBody(vtable_type, vtable_fn_types, method_count, 0);

    // Create vtable global
    char vtable_name[256];
    snprintf(vtable_name, sizeof(vtable_name), "%s_vtable_instance", type_name);
    //printf("Debug: Creating vtable global %s\n", vtable_name);

    LLVMValueRef vtable = LLVMAddGlobal(v->ctx->module, vtable_type, vtable_name);

    // Initialize vtable with method pointers
    LLVMValueRef vtable_init = LLVMConstStruct(method_ptrs, method_count, 0);
    LLVMSetInitializer(vtable, vtable_init);

    // printf("Debug: Vtable initialization complete. Contents:\n");
    // for (int i = 0; i < method_count; i++)
    // {
    //     printf("  Slot %d: %s -> %s\n", i, method_names[i],
    //            LLVMPrintValueToString(method_ptrs[i]));
    // }

    // Cleanup
    for (int i = 0; i < method_count; i++)
    {
        free(method_names[i]);
    }
    free(method_names);
    free(vtable_fn_types);
    free(method_ptrs);

    LLVMDisposeBuilder(v->ctx->builder);
    v->ctx->builder = saved_builder;
}


