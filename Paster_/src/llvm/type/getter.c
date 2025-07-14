

#include "../codegen.h"
#include "common.h"

LLVMValueRef codegen_attr_getter(LLVMVisitor *v, ASTNode *node)
{
    fprintf(stderr, "ESTAMOS EN EL GETTER DE UN ATRIBUTO\n");

    //LLVMTypeRef llvm_instance_type = NULL;

   // if (node->data.binary_op.left->type == AST_VAR)
   // {
        fprintf(stderr, "\nestamos aqui con %s\n", node->data.binary_op.left->computed_type->name);

        LLVMTypeRef llvm_instance_type = type_to_llvm(v, node->data.binary_op.left->computed_type);

        fprintf(stderr, "llvm_instance_type_1: %s\n", LLVMPrintTypeToString(llvm_instance_type));
        //fprintf(stderr, "element_type_1 (after LLVMGetElementType): %s\n", LLVMPrintTypeToString(LLVMGetElementType(llvm_instance_type_1)));

        //exit(1);
   // }

    
    // Genera le valor LLVM para la expresion del lado izq, (instancia)
    LLVMValueRef instance_ptr = codegen_accept(v, node->data.binary_op.left);

    fprintf(stderr, "\nEXITOS\n");

    // obtener el tipo llvm de ese valor
    // LLVMTypeRef llvm_instance_type =

    // obtiene el nombre del campo a aceeder, por ejemplo "attr"
    const char *name = node->data.binary_op.right->data.var_name;

    // recuperar el tipo semantico de alto nivel
    TypeValue *instance_type = node->data.binary_op.left->computed_type;

    // if (LLVMGetTypeKind(llvm_instance_type) != LLVMPointerTypeKind)
    // {
    //     fprintf(stderr, "Error: instance_type_ref no es un puntero\n");
    //     return NULL;
    // }

    // obtener el real del struct, removiendo el puntero ,si se tiene * se elimina
    //LLVMTypeRef struct_type = LLVMGetElementType(llvm_instance_type);
    LLVMTypeRef struct_type = llvm_instance_type;

    if (LLVMGetTypeKind(struct_type) != LLVMStructTypeKind)
    {
        fprintf(stderr, "Error: struct_type no es una estructura\n");
        return NULL;
    }

    TypeValue *field_type = instance_type;

    // Buscar el indice del campo name dentro de al estructura instance_type
    int field_idx = find_field_index(instance_type, name);

    fprintf(stderr, "1-HASTA AQUI TODO BIEN\n");

    if (field_idx == -1 &&
        instance_type->super_type &&
        !is_builtin_type(instance_type->super_type))
    {
        // si no lo encuntra y hay herencia , la cual no es niguna de los tipos builtins
        // intentara buscarlo en el padre
        fprintf(stderr,"Se hizo algo aqui ?\n");

        LLVMValueRef parent_ptr = LLVMBuildStructGEP2(v->ctx->builder, struct_type, instance_ptr, 2, "parent_ptr");

        // actauliza punteros y tipos
        // actualiza la instancia a la del padre porque ahora buscar ahi
        instance_ptr = LLVMBuildLoad2(v->ctx->builder, LLVMGetElementType(LLVMTypeOf(parent_ptr)), parent_ptr, "parent_instance");

        llvm_instance_type = LLVMTypeOf(instance_ptr);
        struct_type = LLVMGetElementType(llvm_instance_type);
        field_type = instance_type->super_type;

        // convierte _Hijo_campo en campo y luego _Padre_campo para buscarlo correctamente

        char *base_name;
        if (strncmp(name, "_", 1) == 0)
        {
            base_name = remove_type_prefix(name, instance_type->name);
        }
        else
        {
            base_name = strdup(name);
        }

        char *parent_field_name = generate_underscored_name(field_type->name, base_name);

        field_idx = find_field_index(field_type, parent_field_name);

        free(base_name);

        if (parent_field_name != name)
        {
            free(parent_field_name);
        }
    }

    fprintf(stderr, "2-HAST AQUI TODO BIEN\n ");

    if (field_idx == -1)
    {
        return NULL;
    }

    // General el getelementptr para acceder al campo con indice field_idx
    LLVMValueRef field_ptr = LLVMBuildStructGEP2(v->ctx->builder, struct_type, instance_ptr, field_idx, "field_ptr");

    fprintf(stderr, "3-HAST AQUI TODO BIEN\n ");

    // if (!v->ctx->builder)
    // {
    //     fprintf(stderr, "Error: builder is NULL\n");
    // }
    // if (!field_ptr)
    // {
    //     fprintf(stderr, "Error: field_ptr is NULL\n");
    // }
    // LLVMTypeRef field_ptr_type = LLVMTypeOf(field_ptr);
    // if (!field_ptr_type)
    // {
    //     fprintf(stderr, "Error: LLVMTypeOf(field_ptr) is NULL\n");
    // }
    // else
    // {
    //     fprintf(stderr, "field_ptr_type: %s\n", LLVMPrintTypeToString(field_ptr_type));
    // }

    // if (LLVMGetTypeKind(field_ptr_type) != LLVMPointerTypeKind)
    // {
    //     fprintf(stderr, "Error: field_ptr is not a pointer\n");
    //     return NULL;
    // }

    // LLVMTypeRef element_type = LLVMGetElementType(field_ptr_type);
    // if (!element_type)
    // {
    //     fprintf(stderr, "Error: element_type is NULL\n");
    //     return NULL;
    // }

    // fprintf(stderr, "element_type: %s\n", LLVMPrintTypeToString(element_type));
    // fprintf(stderr, "struct_type: %s\n", LLVMPrintTypeToString(struct_type));
    // fprintf(stderr, "field_index: %d\n", field_idx);

    // Cargar el valor
    
    LLVMValueRef field_value = LLVMBuildLoad2(v->ctx->builder, LLVMGetElementType(LLVMTypeOf(field_ptr)), field_ptr, name);

    // devuelve el valor
    return field_value;
}

LLVMValueRef codegen_method_getter(LLVMVisitor *v, ASTNode *node)
{
}
