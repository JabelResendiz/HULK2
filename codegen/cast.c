
#include "codegen.h"

// LLVMValueRef codegen_is_type(LLVMVisitor *v, ASTNode *node)
// {
//     fprintf(stderr, "\n" GREEN "--- DEBUG: In codegen_is_type ---" RESET "\n");

//     LLVMValueRef instance = v->codegen_accept(v, node->data.cast_test.exp);

//     fprintf(stderr, GREEN "DEBUG: Después de codegen_accept para la expresión:\n" RESET);
//     fprintf(stderr, GREEN "  Tipo de 'instance': " RESET);
//     LLVMDumpType(LLVMTypeOf(instance));
//     fprintf(stderr, GREEN "  Valor de 'instance': " RESET);
//     LLVMDumpValue(instance);
//     fprintf(stderr, "\n");

//     LLVMTypeRef current_instance_llvm_type = LLVMTypeOf(instance);

//     if (LLVMGetTypeKind(current_instance_llvm_type) != LLVMPointerTypeKind)
//     {
//         fprintf(stderr, RED "ERROR: 'instance' is not a pointer type (Kind: %d). Cannot perform 'is' check.\n" RESET, LLVMGetTypeKind(current_instance_llvm_type));
//         return LLVMConstInt(LLVMInt1Type(), 0, 0); // Return false
//     }

//     LLVMTypeRef element_type_if_ptr_to_ptr = LLVMGetElementType(current_instance_llvm_type);
//     if (element_type_if_ptr_to_ptr != NULL && LLVMGetTypeKind(element_type_if_ptr_to_ptr) == LLVMPointerTypeKind)
//     {
//         fprintf(stderr, GREEN "DEBUG: Instance is a pointer to a pointer. Loading once.\n" RESET);
//         fprintf(stderr, GREEN "  Inner pointer type: " RESET);
//         LLVMDumpType(element_type_if_ptr_to_ptr);
//         fprintf(stderr, "\n");
//         instance = LLVMBuildLoad2(v->ctx->builder, element_type_if_ptr_to_ptr, instance, "dereferenced_instance");
//         fprintf(stderr, GREEN "DEBUG: Instance after first load (now a single pointer): " RESET);
//         LLVMDumpValue(instance);
//         fprintf(stderr, "\n");
//         current_instance_llvm_type = LLVMTypeOf(instance);
//     }
//     else
//     {
//         fprintf(stderr, GREEN "DEBUG: Instance is a single pointer (or opaque ptr without an inner pointer type). No dereference needed here.\n" RESET);
//     }

//     Type *target_type_ast = node->data.cast_test.type;

//     if (is_builtin_type(target_type_ast))
//     {
//         fprintf(stderr, GREEN "DEBUG: Target type '%s' is a built-in type. Comparing static types.\n" RESET, target_type_ast->name);
//         int eq = type_equals(target_type_ast, node->data.cast_test.exp->return_type);
//         return LLVMConstInt(LLVMInt1Type(), eq, 0);
//     }

//     // --- FOR USER-DEFINED TYPES (CLASSES) ---

//     // 1. Resolve the LLVM struct type for the *actual* instance's type.
//     LLVMUserTypeInfo *instance_user_type_info = find_user_type(v->ctx, node->data.cast_test.exp->return_type->name);
//     if (!instance_user_type_info)
//     {
//         fprintf(stderr, RED "ERROR: Could not find LLVMUserTypeInfo for instance type '%s'. Cannot perform 'is' check.\n" RESET, node->data.cast_test.exp->return_type->name);
//         return LLVMConstInt(LLVMInt1Type(), 0, 0);
//     }
//     LLVMTypeRef instance_struct_llvm_type = instance_user_type_info->struct_type;

//     if (instance_struct_llvm_type == NULL)
//     {
//         fprintf(stderr, RED "CRITICAL ERROR: LLVM struct type is NULL in LLVMUserTypeInfo for type '%s'.\n" RESET, node->data.cast_test.exp->return_type->name);
//         return LLVMConstInt(LLVMInt1Type(), 0, 0);
//     }

//     fprintf(stderr, GREEN "DEBUG: Resolved instance's actual LLVM struct type (from UserTypeInfo): " RESET);
//     LLVMDumpType(instance_struct_llvm_type);
//     fprintf(stderr, "\n");

//     // 2. Cast the generic 'instance' pointer to the specific struct pointer type.
//     LLVMTypeRef expected_instance_ptr_type = LLVMPointerType(instance_struct_llvm_type, 0);
//     if (current_instance_llvm_type != expected_instance_ptr_type)
//     {
//         fprintf(stderr, GREEN "DEBUG: Casting instance pointer from general 'ptr' to specific struct pointer type ('%s*').\n" RESET, instance_user_type_info->name);
//         instance = LLVMBuildPointerCast(v->ctx->builder, instance, expected_instance_ptr_type, "casted_instance_ptr");
//         fprintf(stderr, GREEN "DEBUG: Instance pointer after cast: " RESET);
//         LLVMDumpValue(instance);
//         fprintf(stderr, "\n");
//     }

//     // 3. Get the runtime ID from the instance. (This is the actual type ID of the object)
//     int type_id_field_index = 1; // Index 1 for Type ID
//     fprintf(stderr, GREEN "DEBUG: Getting GEP for type ID at index %d.\n" RESET, type_id_field_index);
//     LLVMValueRef id_ptr = LLVMBuildStructGEP2(v->ctx->builder, instance_struct_llvm_type, instance, type_id_field_index, "type_id_ptr");
//     fprintf(stderr, GREEN "DEBUG: Loading instance type ID from GEP.\n" RESET);
//     LLVMValueRef instance_runtime_id = LLVMBuildLoad2(v->ctx->builder, v->ctx->i32_type, id_ptr, "instance_type_id");

//     fprintf(stderr, GREEN "DEBUG: Instance Runtime ID: " RESET);
//     LLVMDumpValue(instance_runtime_id);
//     fprintf(stderr, "\n");

//     // 4. Get the target type's ID (the ID of the type we are checking against).
//     LLVMUserTypeInfo *target_user_type_info = find_user_type(v->ctx, target_type_ast->name);
//     if (!target_user_type_info)
//     {
//         fprintf(stderr, RED "ERROR: Target type '%s' not found in LLVMUserTypeInfo. Cannot get ID.\n" RESET, target_type_ast->name);
//         return LLVMConstInt(LLVMInt1Type(), 0, 0);
//     }
//     int target_type_id_value = target_user_type_info->id;

//     fprintf(stderr, GREEN "DEBUG: Target Type ID: %d\n" RESET, target_type_id_value);

//     // --- CRITICAL CHANGE: Implement inheritance check directly in LLVM IR ---

//     // Get the current function and its entry block
//     LLVMBasicBlockRef current_block = LLVMGetInsertBlock(v->ctx->builder);
//     LLVMValueRef parent_function = LLVMGetBasicBlockParent(current_block);

//     // Create basic blocks for the loop and its branches
//     LLVMBasicBlockRef loop_check_bb = LLVMAppendBasicBlock(parent_function, "is_type_loop_check");
//     LLVMBasicBlockRef loop_body_bb = LLVMAppendBasicBlock(parent_function, "is_type_loop_body");
//     LLVMBasicBlockRef match_bb = LLVMAppendBasicBlock(parent_function, "is_type_match");
//     LLVMBasicBlockRef no_match_bb = LLVMAppendBasicBlock(parent_function, "is_type_no_match");
//     LLVMBasicBlockRef merge_bb = LLVMAppendBasicBlock(parent_function, "is_type_merge");

//     // Branch from current block to loop check
//     LLVMBuildBr(v->ctx->builder, loop_check_bb);

//     // Set builder to loop_check_bb to start building the loop's condition
//     LLVMPositionBuilderAtEnd(v->ctx->builder, loop_check_bb);

//     // PHI node for the current type ID being checked in the loop
//     // It starts with instance_runtime_id
//     LLVMValueRef current_checked_id_phi = LLVMBuildPhi(v->ctx->builder, v->ctx->i32_type, "current_checked_id");
//     LLVMAddIncoming(current_checked_id_phi, &instance_runtime_id, &current_block, 1);

//     // Condition 1: Check if current_checked_id is equal to target_type_id_value
//     LLVMValueRef target_id_llvm = LLVMConstInt(v->ctx->i32_type, target_type_id_value, 0);
//     LLVMValueRef is_equal_to_target = LLVMBuildICmp(v->ctx->builder, LLVMIntEQ, current_checked_id_phi, target_id_llvm, "is_equal_to_target");

//     // Condition 2: Check if current_checked_id is the "no parent" sentinel (-1)
//     LLVMValueRef no_parent_sentinel = LLVMConstInt(v->ctx->i32_type, -1, 0);
//     LLVMValueRef is_no_parent = LLVMBuildICmp(v->ctx->builder, LLVMIntEQ, current_checked_id_phi, no_parent_sentinel, "is_no_parent");

//     // Branch based on conditions:
//     // If equal to target -> match_bb
//     // Else if no parent -> no_match_bb (loop ends, no match found)
//     // Else (has parent and not target) -> loop_body_bb (continue loop)
//     LLVMBuildCondBr(v->ctx->builder, is_equal_to_target, match_bb, loop_body_bb);

//     // Set builder to loop_body_bb to build the loop's body
//     LLVMPositionBuilderAtEnd(v->ctx->builder, loop_body_bb);

//     // Get the parent ID for the current_checked_id
//     // This requires a compile-time lookup for the parent_id based on current_checked_id.
//     // We cannot dynamically look up C structures in LLVM IR like this.
//     // INSTEAD, we need a way to store ALL parent_ids in a lookup table accessible from LLVM IR.
//     // A global array of parent IDs, indexed by type ID, is the most straightforward.
//     // Example: int parent_ids_lookup_table[MAX_TYPE_ID + 1];
//     // This table would be initialized in your C compiler, and LLVM would access it.

//     LLVMUserTypeInfo** all_user_types = get_type_info_array(v->ctx); // Suponiendo que esto te da un array de LLVMUserTypeInfo*
//     int max_type_id = get_max_type_id(v->ctx);

//     LLVMValueRef next_checked_id = NULL;

//     // We'll use a series of select instructions or a nested if-else structure
//     // to map the current_checked_id to its parent_id in LLVM IR.
//     // This is cumbersome for many types. A global array/function for parent IDs is far better.

//     // --- Using a GLOBAL ARRAY of Parent IDs (Recommended for performance and simplicity in IR) ---
//     // You would need to create a global LLVM array initialized with parent IDs.
//     // e.g., @parent_ids = private constant [N x i32] [i32 ID_PARENT_0, i32 ID_PARENT_1, ...]
//     // And then load from it.

//     LLVMValueRef parent_ids_global_array = LLVMGetNamedGlobal(v->ctx->module, "parent_ids_lookup_table");
//     if (!parent_ids_global_array) {
//         // Create the global array if it doesn't exist. This assumes a fixed size.
//         // You'd need to populate this array with parent_ids (or -1 if no parent)
//         // during your type analysis phase.
//         LLVMTypeRef i32_array_type = LLVMArrayType(v->ctx->i32_type, max_type_id + 1);
//         parent_ids_global_array = LLVMAddGlobal(v->ctx->module, i32_array_type, "parent_ids_lookup_table");
//         LLVMSetLinkage(parent_ids_global_array, LLVMExternalLinkage); // or LLVMPrivateLinkage depending on how you initialize it
//         LLVMSetGlobalConstant(parent_ids_global_array, 1); // It's a constant table
//         // You'll need to set its initializer somewhere (e.g., in init_codegen).
//         // LLVMSetInitializer(parent_ids_global_array, LLVMConstArray(v->ctx->i32_type, ...));
//         fprintf(stderr, GREEN "DEBUG: Declared global parent_ids_lookup_table.\n" RESET);
//     }

//     // Get a GEP to the element in the parent_ids_lookup_table
//     LLVMValueRef indices[] = { LLVMConstInt(v->ctx->i32_type, 0, 0), current_checked_id_phi };
//     LLVMValueRef parent_id_ptr_in_table = LLVMBuildGEP2(v->ctx->builder, LLVMGetElementType(LLVMGlobalGetValueType(parent_ids_global_array)), parent_ids_global_array, indices, 2, "parent_id_ptr_in_table");

//     // Load the parent ID from the table
//     next_checked_id = LLVMBuildLoad2(v->ctx->builder, v->ctx->i32_type, parent_id_ptr_in_table, "next_checked_id");

//     // After loading the next_checked_id, branch back to the loop check
//     LLVMBuildBr(v->ctx->builder, loop_check_bb);

//     // Add incoming value for the PHI node (for the loop backedge)
//     LLVMAddIncoming(current_checked_id_phi, &next_checked_id, &loop_body_bb, 1);

//     // Build the match_bb (if target found)
//     LLVMPositionBuilderAtEnd(v->ctx->builder, match_bb);
//     LLVMBuildBr(v->ctx->builder, merge_bb);

//     // Build the no_match_bb (if loop finished without finding target)
//     LLVMPositionBuilderAtEnd(v->ctx->builder, no_match_bb);
//     LLVMBuildBr(v->ctx->builder, merge_bb);

//     // Build the merge_bb
//     LLVMPositionBuilderAtEnd(v->ctx->builder, merge_bb);

//     // PHI node for the final result (true or false)
//     LLVMValueRef final_result_phi = LLVMBuildPhi(v->ctx->builder, v->ctx->i1_type, "is_type_result");
//     LLVMValueRef true_val = LLVMConstInt(v->ctx->i1_type, 1, 0);
//     LLVMValueRef false_val = LLVMConstInt(v->ctx->i1_type, 0, 0);

//     // Add incoming values to the final result PHI
//     LLVMAddIncoming(final_result_phi, &true_val, &match_bb, 1);
//     LLVMAddIncoming(final_result_phi, &false_val, &no_match_bb, 1);

//     fprintf(stderr, GREEN "--- DEBUG: End of codegen_is_type ---" RESET "\n");
//     return final_result_phi;
// }

static int is_super_type(LLVMUserTypeInfo *parent, LLVMUserTypeInfo *child)
{
    LLVMUserTypeInfo *current = child;

    while (current)
    {
        if (current->id == parent->id)
            return current->id;

        current = current->parent_info;
    }

    return -1;
}

LLVMValueRef codegen_is_type(LLVMVisitor *v, ASTNode *node)
{

    // Generate code to get the instance
    LLVMValueRef instance = codegen_accept(v, node->data.cast_test.exp);

    // fprintf(stderr,RED"el tipo dinamico es de %s\n"RESET, node->data.cast_test.exp->return_type->name);

    fprintf(stderr, GREEN "DEBUG: Después de codegen_accept para la expresión:\n" RESET);
    fprintf(stderr, GREEN "  Tipo de 'instance': \n" RESET);
    LLVMDumpType(LLVMTypeOf(instance)); // Muestra el tipo LLVM de 'instance'
    fprintf(stderr, "  Valor de 'instance':\n");
    LLVMDumpValue(instance); // Muestra el LLVM IR de 'instance' (ej. @obj_ptr o %0)
    fprintf(stderr, "\n");

    LLVMTypeRef current_instance_llvm_type = LLVMTypeOf(instance);

    // Make sure we have a properly dereferenced instance pointer
    if (LLVMGetTypeKind(current_instance_llvm_type) != LLVMPointerTypeKind)
    {
        fprintf(stderr, RED "ERROR: 'instance' is not a pointer type (Kind: %d). Cannot perform 'is' check.\n" RESET, LLVMGetTypeKind(current_instance_llvm_type));
        return LLVMConstInt(LLVMInt1Type(), 0, 0); // Return false
    }

    LLVMTypeRef element_type_if_ptr_to_ptr = LLVMGetElementType(current_instance_llvm_type);
    if (element_type_if_ptr_to_ptr != NULL && LLVMGetTypeKind(element_type_if_ptr_to_ptr) == LLVMPointerTypeKind)
    {
        fprintf(stderr, GREEN "DEBUG: Instance is a pointer to a pointer. Loading once.\n" RESET);
        fprintf(stderr, GREEN "  Inner pointer type: " RESET);
        LLVMDumpType(element_type_if_ptr_to_ptr);
        fprintf(stderr, "\n");
        instance = LLVMBuildLoad2(v->ctx->builder, element_type_if_ptr_to_ptr, instance, "dereferenced_instance");
        fprintf(stderr, GREEN "DEBUG: Instance after first load (now a single pointer): " RESET);
        LLVMDumpValue(instance);
        fprintf(stderr, "\n");
        current_instance_llvm_type = LLVMTypeOf(instance); // Update type after load
    }
    else
    {
        fprintf(stderr, GREEN "DEBUG: Instance is a single pointer (or opaque ptr without an inner pointer type). No dereference needed here.\n" RESET);
    }

    // Get the target type's AST node (the type we are comparing against, e.g., 'MyClass')
    Type *target_type_ast = node->data.cast_test.type;

    // Handle built-in types (int, double, bool, etc.) - no runtime ID needed for these
    if (is_builtin_type(target_type_ast))
    {
        fprintf(stderr, GREEN "DEBUG: Target type '%s' is a built-in type. Comparing static types.\n" RESET, target_type_ast->name);
        int eq = type_equals(target_type_ast, node->data.cast_test.exp->return_type);
        return LLVMConstInt(LLVMInt1Type(), eq, 0);
    }

    // --- FOR USER-DEFINED TYPES (CLASSES) ---

    // **THIS IS THE CRITICAL CHANGE:**
    // Get the LLVM struct type for the instance directly from your LLVMUserTypeInfo.
    // Use the `return_type` of the expression to find the correct class information.
    LLVMUserTypeInfo *instance_user_type_info = find_user_type(v->ctx, node->data.cast_test.exp->return_type->name);
    if (!instance_user_type_info)
    {
        fprintf(stderr, RED "ERROR: Could not find LLVMUserTypeInfo for instance type '%s'. Cannot perform 'is' check.\n" RESET, node->data.cast_test.exp->return_type->name);
        return LLVMConstInt(LLVMInt1Type(), 0, 0); // Semantic error, return false
    }
    // This is the correct LLVMTypeRef for the struct (e.g., %MyClass)
    LLVMTypeRef instance_struct_llvm_type = instance_user_type_info->struct_type;

    if (instance_struct_llvm_type == NULL)
    {
        fprintf(stderr, RED "CRITICAL ERROR: LLVM struct type is NULL in LLVMUserTypeInfo for type '%s'.\n" RESET, node->data.cast_test.exp->return_type->name);
        return LLVMConstInt(LLVMInt1Type(), 0, 0); // Semantic error, return false
    }

    fprintf(stderr, GREEN "DEBUG: Resolved instance's actual LLVM struct type (from UserTypeInfo): " RESET);
    LLVMDumpType(instance_struct_llvm_type);
    fprintf(stderr, "\n");

    // Cast the generic 'instance' pointer to the specific struct pointer type.
    // This is crucial for LLVMBuildStructGEP2 to work correctly with opaque 'ptr'.
    LLVMTypeRef expected_instance_ptr_type = LLVMPointerType(instance_struct_llvm_type, 0);
    if (current_instance_llvm_type != expected_instance_ptr_type)
    {
        fprintf(stderr, GREEN "DEBUG: Casting instance pointer from general 'ptr' to specific struct pointer type ('%s*').\n" RESET, instance_user_type_info->name);
        instance = LLVMBuildPointerCast(v->ctx->builder, instance, expected_instance_ptr_type, "casted_instance_ptr");
        fprintf(stderr, GREEN "DEBUG: Instance pointer after cast: " RESET);
        LLVMDumpValue(instance);
        fprintf(stderr, "\n");
    }

    // Get the runtime ID from the instance.
    // Based on your `build_struct_fields` definition:
    // Index 0: VTable Pointer
    // Index 1: Type ID (i32)
    int type_id_field_index = 1; // Correct index for Type ID based on your layout

    fprintf(stderr, GREEN "DEBUG: Getting GEP for type ID at index %d.\n" RESET, type_id_field_index);
    // Use the *specific* struct type for GEP, not the generic 'ptr' type.
    LLVMValueRef id_ptr = LLVMBuildStructGEP2(v->ctx->builder, instance_struct_llvm_type, instance, type_id_field_index, "type_id_ptr");

    fprintf(stderr, GREEN "DEBUG: Loading instance type ID from GEP.\n" RESET);
    LLVMValueRef instance_id = LLVMBuildLoad2(v->ctx->builder, v->ctx->i32_type, id_ptr, "instance_type_id");

    fprintf(stderr, GREEN "DEBUG: Instance Runtime ID: " RESET);
    LLVMDumpValue(instance_id);
    fprintf(stderr, "\n");

    // Get the target type's ID (the ID of the type we are checking against).
    LLVMUserTypeInfo *target_user_type_info = find_user_type(v->ctx, target_type_ast->name);
    if (!target_user_type_info)
    {
        fprintf(stderr, RED "ERROR: Target type '%s' not found in LLVMUserTypeInfo. Cannot get ID.\n" RESET, target_type_ast->name);
        return LLVMConstInt(LLVMInt1Type(), 0, 0); // Semantic error, return false
    }
    int target_type_id_value = target_user_type_info->id;

    fprintf(stderr, GREEN "DEBUG: Target Type ID: %d\n" RESET, target_type_id_value);

    LLVMValueRef target_id_llvm = LLVMConstInt(v->ctx->i32_type, target_type_id_value, 0);

    int super_type_check_result = is_super_type(target_user_type_info, instance_user_type_info);

    // Compare IDs and return the i1 result.
    // LLVMValueRef result = LLVMBuildICmp(v->ctx->builder, LLVMIntEQ, instance_id, target_id_llvm, "type_test");

    LLVMValueRef result = LLVMConstInt(LLVMInt1Type(), (super_type_check_result != -1), 0);

    fprintf(stderr, GREEN "el nombre de mi tipo es %s\n" RESET, instance_user_type_info->name);

    fprintf(stderr, GREEN "--- DEBUG: End of codegen_is_type ---" RESET "\n");

    return result;
}

LLVMValueRef codegen_as_type(LLVMVisitor *v, ASTNode *node)
{
}

LLVMValueRef codegen_base_function(LLVMVisitor *v, ASTNode *node)
{

    fprintf(stderr, RED "ESTAMO EN EL CODEGEN BASE FUNCTION" RESET);

    fprintf(stderr, "EL tipo de retorno es %s\n", node->dynamic_type->name);
    fprintf(stderr, "el tipo de retorno es %s\n", node->return_type->name);
    fprintf(stderr, "el nombre del metodo base es %s\n", node->name_method_base);

    // exit(1);
    const char *method_name = node->name_method_base;
    Type *current_class_type_value = node->data.base_method.current_class_type; // El tipo de la clase actual (ej. B)
    Type *return_type_value = current_class_type_value->parent;                 // El tipo de retorno del método base

    fprintf(stderr, "Metadatos del node: %s\n", method_name);
    fprintf(stderr, "el tipo actual es %s\n", current_class_type_value->name);
    fprintf(stderr, "el tipo del padre actual es %s\n", return_type_value->name);

    if (!method_name || !current_class_type_value || !return_type_value)
    {
        fprintf(stderr, RED "ERROR: Campos nulos en nodo AST_CALL_BASE_METHOD (Línea: %d).\n" RESET, node->line);
        return NULL;
    }

    // 1. Obtener el puntero 'self' de la función actual
    LLVMValueRef self_ptr = LLVMGetParam(v->current_function, 0); // Asume que 'self' está en el ámbito actual
    if (!self_ptr)
    {
        fprintf(stderr, RED "ERROR: 'self' pointer not found in scope for base method call (Línea: %d).\n" RESET, node->line);
        return NULL;
    }

    fprintf(stderr, "\nDEBUG (Attr Getter): Object instance pointer type: %s\n", LLVMPrintTypeToString(LLVMTypeOf(self_ptr)));

    // 2. Identificar la clase base LLVMUserTypeInfo
    LLVMUserTypeInfo *current_class_info = find_user_type(v->ctx, current_class_type_value->name);
    if (!current_class_info)
    {
        fprintf(stderr, RED "ERROR: Información de tipo de clase actual '%s' no encontrada para base method call (Línea: %d).\n" RESET, current_class_type_value->name, node->line);
        return NULL;
    }

    fprintf(stderr, "el current class info es %s\n", current_class_info->name);

    LLVMUserTypeInfo *base_class_info = current_class_info->parent_info; // Acceso al parent_type_info
    if (!base_class_info)
    {
        fprintf(stderr, RED "ERROR: Clase base no encontrada o no definida para '%s' (Línea: %d).\n" RESET, current_class_type_value->name, node->line);
        return NULL;
    }
    fprintf(stderr, "DEBUG: Clase base identificada: '%s'\n", base_class_info->name);

    LLVMMethodInfo *method_info = find_method_info(base_class_info, method_name);

    // LLVMValueRef self_casted = LLVMBuildBitCast(v->ctx->builder, self_ptr, LLVMPointerType(base_class_info->struct_type, 0), "casted_self");

    // LLVMValueRef vtable_ptr_field_addr = LLVMBuildStructGEP2(
    //     v->ctx->builder, base_class_info->struct_type, self_casted, 1, "vtable_ptr_addr");

    // LLVMValueRef vtable_ptr_field_addr = LLVMBuildStructGEP2(v->ctx->builder, base_class_info->struct_type, self_ptr, 1, "vtable_ptr_addr");

    // LLVMValueRef vtable_ptr_loaded = LLVMBuildLoad2(v->ctx->builder, base_class_info->vtable_ptr_type, vtable_ptr_field_addr, "vtable_ptr_loaded");
    // LLVMSetAlignment(vtable_ptr_loaded, 8); // Adjust alignment if necessary

    //LLVMValueRef func_ptr_addr = LLVMBuildStructGEP2(v->ctx->builder, base_class_info->vtable_struct_type, vtable_ptr_loaded, method_info->vtable_index, "func_ptr_slot_addr");
    // LLVMValueRef loaded_func_ptr = LLVMBuildLoad2(v->ctx->builder, LLVMPointerType(method_info->llvm_func_type, 0), func_ptr_addr, "loaded_dynamic_func_ptr");
    // LLVMSetAlignment(loaded_func_ptr, 8);

    fprintf(stderr, "3-HASTA AQUI TODO BIEN, método: %s encontrado en el índice: %d\n", method_info->name, method_info->vtable_index);

    LLVMTypeRef method_signature_type = method_info->llvm_func_type; // El tipo de función COMPLETO

    unsigned int num_llvm_params = LLVMCountParamTypes(method_signature_type); // Número total de parámetros LLVM (incluyendo 'this')
    LLVMValueRef *call_args = (LLVMValueRef *)malloc(num_llvm_params * sizeof(LLVMValueRef));
    if (!call_args)
    {
        perror(RED "Error en malloc para call_args" RESET);
        exit(EXIT_FAILURE);
    }

    call_args[0] = self_ptr; // El puntero 'this'

    // Procesar los argumentos explícitos del método desde el AST
    unsigned int ast_arg_count = node->data.func_node.arg_count;
    // Comprobar que el número de argumentos del AST coincide con los esperados por LLVM (menos 'this')
    if (ast_arg_count != (num_llvm_params - 1))
    {
        fprintf(stderr, RED "Error: Número de argumentos para el método '%s' no coincide (esperado %d, recibido %d).\n" RESET,
                method_info->name, num_llvm_params - 1, ast_arg_count);
        free(call_args);
        return NULL;
    }

    for (unsigned int i = 0; i < ast_arg_count; i++)
    {
        // Generar código para cada argumento del AST
        call_args[i + 1] = codegen_accept(v, node->data.func_node.args[i]);
        if (!call_args[i + 1])
        {
            fprintf(stderr, RED "Error: No se pudo generar el argumento %d para el método '%s'.\n" RESET, i + 1, method_info->name);
            free(call_args);
            return NULL;
        }
    }

    // 5. Construir la instrucción de llamada LLVM
    //LLVMValueRef call_result = LLVMBuildCall2(v->ctx->builder, method_signature_type, loaded_func_ptr, call_args, num_llvm_params, "method_call_result");
    LLVMValueRef call_result = LLVMBuildCall2(
    v->ctx->builder,
    method_signature_type,
    method_info->llvm_func_value, // ← llamada directa, NO cargar de vtable
    call_args,
    num_llvm_params,
    "method_call_result"
);
    
    
    free(call_args);

    char *ir_string = LLVMPrintValueToString(call_result);
    fprintf(stderr, RED "call_result as string:\n%s\n" RESET, ir_string);

    return call_result;

    // // 3. Castear el puntero 'self' al tipo de la clase base
    // LLVMValueRef base_self_ptr = LLVMBuildBitCast(v->ctx->builder, self_ptr, LLVMPointerType(base_class_info->struct_type, 0), "base_self_ptr");

    // fprintf(stderr, "2-DEBUG: Clase base identificada: '%s' con %d\n", base_class_info->name, base_class_info->num_methods_virtual);

    // for(int i=0;i<base_class_info->num_methods_virtual;i++)
    // {
    //     fprintf(stderr,"El nombre es %s\n", base_class_info->methods[i]->name);
    // }
    // // 4. Encontrar el índice del método en la vtable de la clase base
    // int method_idx = -1;
    // for (int i = 0; i < base_class_info->num_methods_virtual; i++) {
    //     LLVMMethodInfo* base_method_node_ast = base_class_info->methods[i];

    //     if (base_method_node_ast && strcmp(base_method_node_ast->name, method_name) == 0) {
    //         method_idx = i;
    //         break;
    //     }
    // }

    // fprintf(stderr, "3-DEBUG: Clase base identificada: '%s'\n", base_class_info->name);

    // if (method_idx == -1) {
    //     fprintf(stderr, RED "ERROR: Método base '%s' no encontrado en la vtable de la clase base '%s' (Línea: %d).\n" RESET, method_name, base_class_info->name, node->line);
    //     return NULL;
    // }
    // fprintf(stderr, "DEBUG: Método base '%s' encontrado en índice %d.\n", method_name, method_idx);

    // // 5. Cargar el puntero a la función desde la vtable de la clase base
    // // Primero, obtener el puntero a la vtable de la instancia base_self_ptr
    // LLVMValueRef base_vtable_ptr_field = LLVMBuildStructGEP2(v->ctx->builder, base_class_info->struct_type, base_self_ptr, 1, "base_vtable_field");
    // LLVMValueRef base_vtable_ptr_loaded = LLVMBuildLoad2(v->ctx->builder, base_class_info->vtable_ptr_type, base_vtable_ptr_field, "base_vtable_ptr_loaded");
    // LLVMSetAlignment(base_vtable_ptr_loaded, 8);

    // fprintf(stderr, "2-DEBUG: Método base '%s' encontrado en índice %d.\n", method_name, method_idx);

    // // Luego, obtener la dirección del puntero a la función dentro de la vtable
    // LLVMValueRef func_ptr_addr = LLVMBuildStructGEP2(v->ctx->builder, base_class_info->vtable_struct_type, base_vtable_ptr_loaded, method_idx, "base_func_ptr_addr");

    // // Obtener el tipo de la firma del método base
    // LLVMTypeRef base_method_signature_type =  base_class_info->methods[method_idx]->llvm_func_type;

    // // Cargar el puntero a la función real
    // LLVMValueRef loaded_func_ptr = LLVMBuildLoad2(v->ctx->builder, LLVMPointerType(base_method_signature_type, 0), func_ptr_addr, "loaded_base_func_ptr");
    // LLVMSetAlignment(loaded_func_ptr, 8);

    // // 6. Preparar los argumentos para la llamada
    // LLVMValueRef *call_args = (LLVMValueRef*)malloc((1 + node->data.func_node.arg_count) * sizeof(LLVMValueRef));
    // if (!call_args) { perror(RED "Error en malloc para call_args (codegen_base_function)" RESET); exit(EXIT_FAILURE); }

    // fprintf(stderr, "3-DEBUG: Método base '%s' encontrado en índice %d.\n", method_name, method_idx);

    // call_args[0] = base_self_ptr; // El primer argumento es el puntero 'this' (ya casteado a la base)

    // fprintf(stderr,"el numero de varaibles es %d\n", node->data.func_node.arg_count);

    // for (int i = 0; i < node->data.func_node.arg_count; i++) {
    //     call_args[1 + i] = codegen_accept(v, node->data.func_node.args[i]);
    //     if (!call_args[1 + i]) {
    //         fprintf(stderr, RED "ERROR: No se pudo generar código para el argumento %d de la llamada al método base (Línea: %d).\n" RESET, i, node->line);
    //         free(call_args);
    //         return NULL;
    //     }
    // }

    // // 7. Generar la llamada a la función
    // LLVMValueRef call_result = LLVMBuildCall2(v->ctx->builder, base_method_signature_type, loaded_func_ptr, call_args, 1 + node->data.func_node.arg_count, "base_call_result");
    // free(call_args);

    // fprintf(stderr, "DEBUG: Llamada al método base '%s' generada.\n", method_name);

    // return call_result;
}