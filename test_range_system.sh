#!/bin/bash

echo "🧪 Probando el sistema con la palabra reservada 'range'..."

echo -e "\n📝 Archivo de prueba:"
cat test_range.hulk

echo -e "\n🔍 Ejecutando análisis léxico y sintáctico..."
./main test_range.hulk

echo -e "\n✅ Prueba completada." 