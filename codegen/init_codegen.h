
#ifndef INIT_CODEGEN_H
#define INIT_CODEGEN_H

#include <llvm-c/Core.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include <stdio.h>
#include "../ast/ast.h"

// estructura para almacenar informacion de un miembro
typedef struct LLVMTypeMemberInfo
{
    char *name;
    LLVMTypeRef llvm_type;
    int index;
    // struct LLVMTypeMemberInfo* next;
} LLVMTypeMemberInfo;

typedef struct LLVMMethodInfo
{

    char *name;
    int vtable_index;
    LLVMTypeRef llvm_func_type;
    LLVMValueRef llvm_func_value;
    ASTNode *node;
    // struct LLVMMethodInfo* next;

} LLVMMethodInfo;

// Estructura para almacenar información de un tipo definido por el usuario
typedef struct LLVMUserTypeInfo
{
    int id;                         // el id del tipo
    char *name;                     // Nombre de la clase (ej. "A")
    LLVMTypeRef struct_type;        // El LLVMTypeRef para la estructura de la clase (ej. %A)
    LLVMTypeRef vtable_struct_type; // El LLVMTypeRef para la estructura de la vtable (ej. %A_vtable)
    LLVMTypeRef class_ptr_type;     // LO ultimo agregado (funcionaba antes )
    LLVMTypeRef vtable_ptr_type;    // El LLVMTypeRef para el puntero a la vtable (ej. %A_vtable*)
    LLVMValueRef vtable_global;     // La instancia global de la vtable (ej. @A_vtable_instance)

    LLVMTypeRef *vtable_slot_ptr_types;
    LLVMTypeRef *vtable_slot_types;
    LLVMTypeRef *struct_fields;
    LLVMValueRef *vtable_initializer_values;

    // LLVMTypeRef method_func_type;
    //  Aquí podrías añadir una lista de miembros, una lista de métodos, etc.
    LLVMTypeMemberInfo **members; // una lista de los parametros
    int num_data_members;         // la cantidad de parametros

    // LLVMValueRef* method_llvm_funcs; // Array de LLVMValueRef de las funciones LLVM para cada metodo virtual
    // int num_methods_virtual;        // numero de metodos virtuales
    // ASTNode **method_decl_nodes_ast;
    LLVMMethodInfo **methods;
    int num_methods_virtual;

    struct LLVMUserTypeInfo *parent_info;

    struct LLVMUserTypeInfo *next; // Para una lista enlazada de tipos definidos por el usuario
} LLVMUserTypeInfo;

typedef struct LLVMCoreContext
{
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMContextRef context;
    LLVMValueRef current_stack_depth_var;
    int max_stack_depth;

    // Tipos básicos de LLVM (globales para todo el compilador)
    LLVMTypeRef i32_type;
    LLVMTypeRef i64_type;
    LLVMTypeRef ptr_type; // Puntero genérico (i8*)
    LLVMTypeRef i8_type;  // byte (para ptr_type y cadenas)
    LLVMTypeRef i1_type;
    LLVMTypeRef void_type;
    LLVMTypeRef double_type;

    // // Tipo genérico de puntero a función para métodos de vtable
    // LLVMTypeRef method_func_type_proto;     // Prototipo de la función del método (ej. i32 (ptr))
    // LLVMTypeRef method_func_ptr_type_proto; // Prototipo del puntero a la función del método (ej. i32 (ptr)*)

    // Funciones externas y globales
    LLVMValueRef malloc_func;
    LLVMTypeRef malloc_func_type;
    LLVMValueRef printf_func;
    LLVMTypeRef printf_func_type;
    LLVMValueRef puts_func;     // From your init_codegen.h
    LLVMTypeRef puts_func_type; // From your init_codegen.h
    LLVMValueRef exit_func;     // From your init_codegen.h
    LLVMTypeRef exit_func_type; // From your init_codegen.h

    LLVMValueRef format_string_global; // The "%d\n" string for printf

    // Other builtins from your init_codegen.h, need to be stored if used
    LLVMValueRef strcpy_func;
    LLVMTypeRef strcpy_func_type;
    LLVMValueRef strcat_func;
    LLVMTypeRef strcat_func_type;
    LLVMValueRef strlen_func;
    LLVMTypeRef strlen_func_type;
    LLVMValueRef sqrt_func;
    LLVMTypeRef sqrt_func_type;
    LLVMValueRef sin_func;
    LLVMTypeRef sin_func_type;
    LLVMValueRef cos_func;
    LLVMTypeRef cos_func_type;
    LLVMValueRef log_func;
    LLVMTypeRef log_func_type;
    LLVMValueRef exp_func;
    LLVMTypeRef exp_func_type;
    LLVMValueRef rand_func;
    LLVMTypeRef rand_func_type;
    LLVMValueRef pow_func;
    LLVMTypeRef pow_func_type;
    LLVMValueRef fmod_func;
    LLVMTypeRef fmod_func_type;
    LLVMValueRef snprintf_func;
    LLVMTypeRef snprintf_func_type;
    LLVMValueRef strcmp_func;
    LLVMTypeRef strcmp_func_type;

    LLVMUserTypeInfo *user_types;

} LLVMCoreContext;

// --------------------------METHODS-------------------------------

LLVMTypeRef type_to_llvm(LLVMCoreContext *ctx, Type *type);

LLVMValueRef createGlobalString(LLVMCoreContext *ctx, const char *str, const char *name);

int find_field_index(LLVMUserTypeInfo *type_info, const char *param_name);
LLVMTypeMemberInfo *find_member_info(LLVMUserTypeInfo *type_info, const char *param_name);

int get_id_type(LLVMCoreContext *ctx, const char *name);

LLVMUserTypeInfo *find_user_type(LLVMCoreContext *ctx, const char *name);

LLVMMethodInfo *find_method_info(LLVMUserTypeInfo *type_info, const char *name);

// Inicializacion/limpieza
LLVMCoreContext *llvm_core_context_create();

LLVMTypeRef *build_struct_fields(LLVMCoreContext *ctx, ASTNode *node, LLVMUserTypeInfo *user);
LLVMValueRef *build_struct_method(LLVMCoreContext *ctx, ASTNode *node, LLVMUserTypeInfo *type_info);
LLVMValueRef *build_vtable_initializer(LLVMCoreContext *ctx, ASTNode *node, LLVMUserTypeInfo *type_info);

void free_llvm_core_context(LLVMCoreContext *ctx);
void free_llvm_type_member(LLVMTypeMemberInfo **member);
void free_llvm_method(LLVMMethodInfo **method);

// Funciones runtime
void llvm_declare_builtins(LLVMCoreContext *ctx);

// Manejor de errores
void llvm_handle_stack_overflow(
    LLVMCoreContext *ctx,
    ASTNode *node);

LLVMValueRef createGlobalString(LLVMCoreContext *ctx, const char *str, const char *name);

#endif