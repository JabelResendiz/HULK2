
// visitor.h

#ifndef VISITOR_H
#define VISITOR_H

#include "../ast/ast.h"
#include "../type_value/type.h"
#include "../scope/scope.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

// Predeclara Visitor
typedef struct ASTVisitor ASTVisitor;

// --- Funciones de visita agrupadas por tipo ---
typedef struct {
    void (*program)(ASTVisitor*, ASTNode*);
    void (*number)(ASTVisitor*, ASTNode*);
    void (*string)(ASTVisitor*, ASTNode*);
    void (*boolean)(ASTVisitor*, ASTNode*);
    void (*variable)(ASTVisitor*, ASTNode*);
} BasicVisitors;

typedef struct {
    void (*binary)(ASTVisitor*, ASTNode*);
    void (*assignment)(ASTVisitor*, ASTNode*);
    void (*call_function)(ASTVisitor*, ASTNode*);
} ExpressionVisitors;

typedef struct {
    void (*block)(ASTVisitor*, ASTNode*);
    void (*dec_function)(ASTVisitor*, ASTNode*);
    void (*let_in)(ASTVisitor*, ASTNode*);
    void (*conditional)(ASTVisitor*, ASTNode*);
    void (*while_loop)(ASTVisitor*, ASTNode*);
} ControlVisitors;

typedef struct {
    void (*type_dec)(ASTVisitor*, ASTNode*);
    void (*type_instance)(ASTVisitor*, ASTNode*);
} TypeVisitors;

typedef struct {
    void (*getter)(ASTVisitor*, ASTNode*);
    void (*setter)(ASTVisitor*, ASTNode*);
} AttributeVisitors;

// --- Estructura del visitor ---
struct ASTVisitor {
    int error_count;
    char** errors;

    // Contexto
    char* current_function;
    TypeValue* current_type;
    int id;
    // Visitadores
    BasicVisitors basic;
    ExpressionVisitors expr;
    ControlVisitors control;
    TypeVisitors types;
    AttributeVisitors attrs;
};

// --- Funciones de utilidades ---


// Reecibe un nodo y despacha su analisis al metodo del visitante
void accept(ASTVisitor* visitor, ASTNode* node);


#endif