
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



























LLVMValueRef codegen_is_type(LLVMVisitor *v, ASTNode *node)
{

    // Generate code to get the instance
    LLVMValueRef instance = codegen_accept(v, node->data.cast_test.exp);

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
        return LLVMConstInt( LLVMInt1Type(), 0, 0); // Return false
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
        return LLVMConstInt( LLVMInt1Type(), eq, 0);
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
        return LLVMConstInt( LLVMInt1Type(), 0, 0); // Semantic error, return false
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

    // Compare IDs and return the i1 result.
    LLVMValueRef result = LLVMBuildICmp(v->ctx->builder, LLVMIntEQ, instance_id, target_id_llvm, "type_test");

    fprintf(stderr, GREEN "--- DEBUG: End of codegen_is_type ---" RESET "\n");
    return result;
}

LLVMValueRef codegen_as_type(LLVMVisitor *v, ASTNode *node)
{
}
