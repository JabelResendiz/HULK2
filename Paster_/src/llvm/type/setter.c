
#include "../codegen.h"
#include "common.h"

LLVMValueRef codegen_attr_setter(LLVMVisitor *v, ASTNode *node)
{
    LLVMValueRef instance = codegen_accept(v, node->data.setter.instance);

    const char *property_name = node->data.setter.property->data.var_name;

    LLVMValueRef value = codegen_accept(v, node->data.setter.value);

    LLVMTypeRef llvm_instance_type = LLVMTypeOf(instance);

    if (LLVMGetTypeKind(llvm_instance_type) == LLVMPointerTypeKind)
    {
        LLVMTypeRef elementType = LLVMGetElementType(llvm_instance_type);
        if (LLVMGetTypeKind(elementType) == LLVMPointerTypeKind)
        {
            // instance = LLVMBuildLoad(v->ctx->builder, instance, "loaded_instance");
            LLVMTypeRef pointee_type = LLVMGetElementType(LLVMTypeOf(instance));
            instance = LLVMBuildLoad2(v->ctx->builder, pointee_type, instance, "loaded_instance");
        }
    }

    LLVMTypeRef new_llvm_instance_type = LLVMTypeOf(instance);
    TypeValue *instance_type = node->data.setter.instance->computed_type;

    int field_index = find_field_index(instance_type, property_name);

    LLVMTypeRef struct_type = LLVMGetElementType(new_llvm_instance_type);
    LLVMValueRef field_ptr = LLVMBuildStructGEP2(
        v->ctx->builder,
        struct_type,
        instance,
        field_index,
        "field_ptr");

    LLVMBuildStore(v->ctx->builder, value, field_ptr);

    return value;
}