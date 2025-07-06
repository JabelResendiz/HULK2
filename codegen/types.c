

// // types.c

// #include "types.h"
// #include "visitor_llvm.h"
// #include "init_codegen.h"

// void build_vtable_table(LLVMVisitor* v,ASTNode* node)
// {

//     LLVMTypeRef A_vtable_struct_type = LLVMStructCreateNamed(v->ctx->context, "A_vtable");
//     LLVMTypeRef A_struct_type = LLVMStructCreateNamed(v->ctx->context, "A");

//     // funciones que reciben un puntero y devuelven un entero
//     LLVMTypeRef method_func_type = LLVMFunctionType
//     (v->ctx->i32_type, 
//     (LLVMTypeRef[]){LLVMPointerType(A_struct_type, 0)}, 
//     1, 
//     0);


//     LLVMTypeRef method_func_ptr_type = LLVMPointerType(method_func_type, 0); // Puntero a ese tipo de función
    
//     // construccion del vtable con 2 metodos de funciones
//     LLVMStructSetBody(A_vtable_struct_type, (LLVMTypeRef[]){method_func_ptr_type}, 2, 0);
    
//     LLVMTypeRef A_vtable_ptr_type = LLVMPointerType(A_vtable_struct_type, 0);

//     // Construccion de la clase del tipo
//     LLVMTypeRef A_struct_fields[] = { v->ctx->i32_type, A_vtable_ptr_type, v->ctx->i32_type, v->ctx->i32_type ,v->ctx->i32_type,v->ctx->i32_type}; // i32 (id), ptr (vtable_ptr), i32 (x), i32(y)
//     LLVMStructSetBody(A_struct_type, A_struct_fields, 6, 0); // aqui donde hay un 4 iba un 3 

//     LLVMTypeRef A_ptr_type = LLVMPointerType(A_struct_type, 0); // Puntero a %A
    
//     //  // --- 5. Definición del Método _A_getX ---
//     // // _A_getX function type: i32 (%A*)* (reusamos method_func_type)
//     // LLVMValueRef getX_func = LLVMAddFunction(v->ctx->module, "_A_getX", method_func_type);

    
//     // // Construye el cuerpo de la función _A_getX
//     // LLVMBasicBlockRef getX_entry = LLVMAppendBasicBlockInContext(v->ctx->context, getX_func, "entry");
//     // LLVMPositionBuilderAtEnd(v->ctx->builder, getX_entry);

//     // LLVMValueRef param_0 = LLVMGetParam(getX_func, 0); // %0 es el puntero 'this'

//     // // %x_ptr = getelementptr inbounds %A, %A* %0, i32 0, i32 2
//     // // El índice 2 es para el tercer campo i32 (el campo 'x')
//     // LLVMValueRef x_ptr = LLVMBuildStructGEP2(v->ctx->builder, A_struct_type, param_0, 2, "x_ptr");

    
//     // // %x_val = load i32, i32* %x_ptr, align 4
//     // LLVMValueRef x_val = LLVMBuildLoad2(v->ctx->builder, v->ctx->i32_type, x_ptr, "x_val");

    
//     // LLVMSetAlignment(x_val, 4); // Establece la alineación para la instrucción load
   
//     // // ret i32 %x_val
//     // LLVMBuildRet(v->ctx->builder, x_val);
    
//     // // DEFINICION DEL METODO A_getY ()

//     // LLVMValueRef getY_func = LLVMAddFunction(v->ctx->module, "_A_getY", method_func_type);

//     // LLVMBasicBlockRef getY_entry = LLVMAppendBasicBlockInContext(v->ctx->context, getY_func, "entry");
//     // LLVMPositionBuilderAtEnd(v->ctx->builder, getY_entry);

//     // LLVMValueRef param_1 = LLVMGetParam(getY_func, 0); // %0 es el puntero 'this'

//     // LLVMValueRef y_ptr = LLVMBuildStructGEP2(v->ctx->builder,A_struct_type,param_1,3,"y_ptr");

//     // LLVMValueRef y_val = LLVMBuildLoad2(v->ctx->builder,v->ctx->i32_type,y_ptr,"y_val");

//     // LLVMSetAlignment(y_val,4);

//     // LLVMBuildRet(v->ctx->builder,y_val);

    
    
//     // --- 6. Definición d  e la Instancia Global de Vtable (@A_vtable_instance) ---
//     // @A_vtable_instance = internal global %A_vtable { i32 (%A*)* @_A_getX }

//     fprintf(stderr,"Jabel Resendiz \n");
//     LLVMValueRef vtable_initializer_values[] = { }; // getX_func ya es de tipo i32 (%A*)*

//     fprintf(stderr,"Aguirre \n");

//     // aqui el segundo 0 debe ser relacionado con los metodos de vtable_initilizer_value
//     LLVMValueRef A_vtable_constant = LLVMConstStruct(vtable_initializer_values, 0, 0);
    
//     fprintf(stderr,"Aguirre\n");

//     LLVMValueRef A_vtable_instance_global = LLVMAddGlobal(v->ctx->module, A_vtable_struct_type, "A_vtable_instance");
//     LLVMSetInitializer(A_vtable_instance_global, A_vtable_constant);
//     LLVMSetLinkage(A_vtable_instance_global, LLVMInternalLinkage);
//     LLVMSetUnnamedAddr(A_vtable_instance_global, 1);


// }