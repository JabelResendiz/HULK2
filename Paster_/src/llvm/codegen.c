

// codegen.c

#include "codegen.h"
#include "init_codegen.h"
#include "visitor_llvm.h"
#include "../llvm_scope/llvm_scope.h"
#include <stdio.h>
#include <string.h>
#include "../type_value/type.h"

// Convierte un tipo personalizado de HULK a su equivalente en LLVM (LLVMTypeRef)
LLVMTypeRef type_to_llvm(LLVMVisitor*v,TypeValue *type)
{
    if (compare_types(type, &TYPE_NUM))
    {
        return LLVMDoubleType(); // doble en LLVM
    }
    else if (compare_types(type, &TYPE_STRING))
    {
        return LLVMPointerType(LLVMInt8Type(), 0); // puntero a i8* lo que equivale un char*
    }
    else if (compare_types(type, &TYPE_BOOLEAN))
    {
        return LLVMInt1Type(); // se usa i1, el tipo entero de 1 bit en LLVM
    }
    else if (compare_types(type, &TYPE_VOID))
    {
        return LLVMVoidType(); // retorno el tipo void de LLVM
    }
    else if (compare_types(type, &TYPE_OBJ))
    {
        // usamos un puntero void* que puede apuntar a cualquier tipo
        return LLVMPointerType(LLVMInt8Type(), 0);
    }

    else if (type->def_node != NULL) {
        // Es un tipo personalizado
        fprintf(stderr, "Módulo actual: %p\n", v->ctx->module);

        LLVMTypeRef struct_type = LLVMGetTypeByName(v->ctx->module, type->name);
        if (!struct_type) {
            fprintf(stderr, "Error: Tipo %s no encontrado\n", type->name);
            exit(1);
        }
        fprintf(stderr, "EN TYPE_TO_LLVM : struct_type: %s\n", LLVMPrintTypeToString(struct_type));

        fprintf(stderr,"Sera un puntero\n");
        // Retornamos un puntero al tipo estructurado
        LLVMTypeRef l =LLVMPointerType(struct_type, 0);

        // // fprintf(stderr, "l: %s\n", LLVMPrintTypeToString(l));
        // // fprintf(stderr, "element_type_1 (after LLVMGetElementType): %s\n", LLVMPrintTypeToString(LLVMGetElementType(l)));

        // LLVMTypeRef l =struct_type;
        return l;
    }
    fprintf(stderr, "Error: Tipo desconocido %s\n", type->name);
    exit(1);
}

// Genera la funcion main en IR (Intermediate Representation) de LLVM a partir de un AST
void compile_to_llvm(ASTNode *ast, const char *filename)
{

    // Initializa el LLVM
    LLVMCoreContext *ctx = llvm_core_context_create();

    // inicializador del Visitor
    LLVMVisitor visitor =
        {
            .ctx = ctx,
            .basic =
                {
                    .program = codegen_program,
                    .number = codegen_number,
                    .string = codegen_string,
                    .boolean = codegen_boolean,
                    .variable = codegen_variable},
            .expr =
                {
                    .binary = codegen_binary_op,
                    .assignment = codegen_assignments,
                    .call_function = codegen_call_function},
            .control =
                {
                    .block = codegen_block,
                    .dec_function = codegen_dec_function,
                    .let_in = codegen_let_in,
                    .conditional = codegen_conditional,
                    .while_loop = codegen_while},
            .types =
                {
                    .type_dec = codegen_type_dec,
                    .type_instance = codegen_type_instance},
            .attrs =
                {
                    .attr_getter = codegen_attr_getter,
                    .attr_setter = codegen_attr_setter,
                    .method_getter = codegen_method_getter}

        };

    // Declara funciones propias del lenguaje
    llvm_declare_builtins(ctx);

    // regsitrar las firmas de funciones antes de generar los cuerpos
    find_function_dec(&visitor, ast);

    // generar los cuerpos de las funciones
    make_body_function_dec(&visitor, ast);
    
    // find_type_dec(&visitor,ast);

    // make_body_type_dec(&visitor,ast);

    // Create scope
    push_scope();

    // crea la funcion main con retorno i32 0
    LLVMTypeRef main_type = LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0);
    LLVMValueRef main_func = LLVMAddFunction(ctx->module, "main", main_type);
    LLVMBasicBlockRef entry = LLVMAppendBasicBlock(main_func, "entry");

    // posiciona el builder en el bloque de entrada de main
    LLVMPositionBuilderAtEnd(ctx->builder, entry);

    if (ast)
    {
        // llama a codegen para recorrer el AST completo
        codegen_accept(&visitor, ast);
    }

    LLVMBasicBlockRef current_block = LLVMGetInsertBlock(ctx->builder);
    if (!LLVMGetBasicBlockTerminator(current_block))
    {
        LLVMBuildRet(ctx->builder, LLVMConstInt(LLVMInt32Type(), 0, 0));
    }

    fprintf(stderr, "compilador80\n");

    // Escribe en el archivo .ll
    char *error = NULL;
    if (LLVMPrintModuleToFile(ctx->module, filename, &error))
    {
        fprintf(stderr, "Error writing IR: %s\n", error);
        LLVMDisposeMessage(error);
        exit(1);
    }

    // Libera recursos
    free_llvm_core_context(ctx);
}

LLVMValueRef codegen_program(LLVMVisitor *v, ASTNode *node)
{
    push_scope();
    LLVMValueRef last = NULL;

    fprintf(stderr, "JABEL RESENDIZ\n");

    for (int i = 0; i < node->data.program.count; i++)
    {
        ASTNode *stmt = node->data.program.statements[i];
        if (stmt->type != AST_DECL_FUNC)
        {
            last = codegen_accept(v, stmt);
        }
        fprintf(stderr, "JABEL RESENDIZ\n");
    }

    pop_scope();
    return last ? last : LLVMConstInt(LLVMInt32Type(), 0, 0);
}

void find_function_dec(LLVMVisitor *visitor, ASTNode *node)
{
    if (!node)
        return;

    // Si es una declaración de función, procesarla y buscar dentro de su cuerpo
    if (node->type == AST_DECL_FUNC)
    {
        make_function_dec(visitor, node);
        // Buscar funciones anidadas dentro del cuerpo de la función
        find_function_dec(visitor, node->data.func_node.body);
        return;
    }

    // Recursivamente buscar en los diferentes tipos de nodos
    switch (node->type)
    {
    case AST_PROGRAM:
    case AST_BLOCK:
        for (int i = 0; i < node->data.program.count; i++)
        {
            find_function_dec(visitor, node->data.program.statements[i]);
        }
        break;

    case AST_LET:
        // Buscar en las declaraciones
        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            if (node->data.func_node.args[i]->type == AST_ASSIGNMENT)
            {
                find_function_dec(visitor, node->data.func_node.args[i]->data.binary_op.right);
            }
        }
        // Buscar en el cuerpo
        find_function_dec(visitor, node->data.func_node.body);
        break;
    }
}

// agregar la firma de la funcion
LLVMValueRef make_function_dec(LLVMVisitor *v, ASTNode *node)
{
    const char *name = node->data.func_node.name;
    TypeValue *return_type = node->data.func_node.body->computed_type;
    ASTNode **params = node->data.func_node.args;
    int param_count = node->data.func_node.arg_count;

    // Obtener tipos de parámetros
    LLVMTypeRef *param_types = malloc(param_count * sizeof(LLVMTypeRef));
    for (int i = 0; i < param_count; i++)
    {
        param_types[i] = type_to_llvm(v,params[i]->computed_type);
    }

    // Crear y registrar la firma de la función
    LLVMTypeRef func_type = LLVMFunctionType(
        type_to_llvm(v,return_type),
        param_types,
        param_count,
        0);

    LLVMValueRef func = LLVMAddFunction(v->ctx->module, name, func_type);
    free(param_types);
    return func;
}

void make_body_function_dec(LLVMVisitor *visitor, ASTNode *node)
{
    if (!node)
        return;

    // Si es una declaración de función, generar su cuerpo y procesar funciones anidadas
    if (node->type == AST_DECL_FUNC)
    {
        codegen_accept(visitor, node);
        // Procesar funciones anidadas en el cuerpo de la función
        make_body_function_dec(visitor, node->data.func_node.body);
        return;
    }

    // Recursivamente procesar los diferentes tipos de nodos
    switch (node->type)
    {
    case AST_PROGRAM:
    case AST_BLOCK:
        for (int i = 0; i < node->data.program.count; i++)
        {
            make_body_function_dec(visitor, node->data.program.statements[i]);
        }
        break;

    case AST_LET:
        // Procesar declaraciones
        for (int i = 0; i < node->data.func_node.arg_count; i++)
        {
            if (node->data.func_node.args[i]->type == AST_ASSIGNMENT)
            {
                make_body_function_dec(visitor, node->data.func_node.args[i]->data.binary_op.right);
            }
        }
        // Procesar el cuerpo
        make_body_function_dec(visitor, node->data.func_node.body);
        break;
    }
}

// /// @brief Declara los tipos de mi programa
// void find_type_dec(LLVMVisitor *visitor, ASTNode *node)
// {
//     if (!node)
//         return;

//     if (node->type == AST_TYPE)
//     {
//         make_type_dec(visitor, node);

//         return;
//     }

//     // Recursivo si hay mas tipos en el AST
//     switch (node->type)
//     {
//     case AST_PROGRAM:
//     case AST_BLOCK:
//         for (int i = 0; i < node->data.program.count; i++)
//         {
//             find_type_dec(visitor, node->data.program.statements[i]);
//         }
//     }
// }

// /// @brief  Aqui voy a agregar la firma del tipo a mi modulo y mi tabla
// void make_type_dec(LLVMVisitor* v,ASTNode* node)
// {
//     const char* name = node->data.typeDef.name_type;
//     const char * parent_name = node->data.typeDef.name_parent;
//     ASTNode** args= node->data.typeDef.args;
//     int args_count = node->data.typeDef.args_count;

//     LLVMTypeRef* llvm_param_types = malloc(args_count * sizeof(LLVMTypeRef));
//     TypeValue** param_types  = malloc(args_count * sizeof(TypeValue));

//     for(int i=0;i<args_count;i++)
//     {
//         llvm_param_types[i] = type_to_llvm(args[i]->computed_type);
//         param_types[i]= args[i]->computed_type;
//     }

//     // Crear y registrar la firma del tipo
//     LLVMTypeRef struct_type = LLVMStructCreateNamed(v->ctx->context, name);
//     register_llvm_type(name, struct_type);

    
// }



// void make_body_type_dec(LLVMVisitor* visitor, ASTNode* node) {
//     if (!node) return;

//     if (node->type == AST_TYPE) {
        

//         codegen_accept(visitor,node);
//         return ;
//     }

//     // Solo procesar si es bloque o programa
//     if (node->type == AST_PROGRAM || node->type == AST_BLOCK) {
//         for (int i = 0; i < node->data.program.count; ++i) {
//             make_body_type_dec(visitor, node->data.program.statements[i]);
//         }
//     }
// }


