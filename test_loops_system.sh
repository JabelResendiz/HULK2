#!/bin/bash

echo "🧪 Probando el sistema con ciclos dentro de funciones..."

echo -e "\n📝 Archivo de prueba script.hulk:"
cat script.hulk

echo -e "\n📝 Archivo de prueba test_loops_in_functions.hulk:"
cat test_loops_in_functions.hulk

echo -e "\n🔍 Ejecutando análisis léxico y sintáctico en script.hulk..."
./main script.hulk

echo -e "\n🔍 Ejecutando análisis léxico y sintáctico en test_loops_in_functions.hulk..."
./main test_loops_in_functions.hulk

echo -e "\n✅ Prueba completada." 