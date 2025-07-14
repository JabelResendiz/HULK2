#include "parser_generator.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s <archivo_gramatica> [archivo_salida]\n", argv[0]);
        printf("Ejemplo: %s gramatica.ll parser_generated.c\n", argv[0]);
        return 1;
    }
    
    const char* grammar_file = argv[1];
    const char* output_file = (argc > 2) ? argv[2] : "parser_generated.c";
    
    printf("Generador de Parser LL(1)\n");
    printf("========================\n");
    printf("Archivo de gramática: %s\n", grammar_file);
    printf("Archivo de salida: %s\n", output_file);
    printf("\n");
    
    // Cargar la gramática desde el archivo
    printf("Cargando gramática...\n");
    Grammar* grammar = grammar_load_from_file(grammar_file);
    if (!grammar) {
        fprintf(stderr, "Error: No se pudo cargar la gramática desde %s\n", grammar_file);
        return 1;
    }
    
    printf("Gramática cargada exitosamente.\n");
    printf("Símbolos: %d\n", grammar->symbol_count);
    printf("Producciones: %d\n", grammar->production_count);
    printf("\n");
    
    // Imprimir la gramática cargada
    grammar_print(grammar);
    printf("\n");
    
    // Generar la tabla de parsing
    printf("Generando tabla de parsing LL(1)...\n");
    ParsingTable* table = generate_parsing_table(grammar);
    if (!table) {
        fprintf(stderr, "Error: No se pudo generar la tabla de parsing\n");
        grammar_destroy(grammar);
        return 1;
    }
    
    printf("Tabla de parsing generada exitosamente.\n");
    printf("No terminales: %d\n", table->nonterminal_count);
    printf("Terminales: %d\n", table->terminal_count);
    printf("\n");
    
    // Imprimir la tabla de parsing
    parsing_table_print(table);
    printf("\n");
    
    // Generar código C del parser
    printf("Generando código C del parser...\n");
    generate_parser_code(grammar, table, output_file);
    printf("Código generado en: %s\n", output_file);
    
    // Limpiar memoria
    parsing_table_destroy(table);
    grammar_destroy(grammar);
    
    printf("\nGeneración completada exitosamente.\n");
    return 0;
} 