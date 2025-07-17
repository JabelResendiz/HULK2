#!/bin/bash

# Script para probar el nuevo sistema de manejo de errores

echo "🧪 Probando el nuevo sistema de manejo de errores..."
echo "=================================================="

# Compilar el proyecto
echo "🔨 Compilando el proyecto..."
make clean
make compile

if [ $? -ne 0 ]; then
    echo "❌ Error en la compilación"
    exit 1
fi

echo "✅ Compilación exitosa"
echo ""

# Probar con archivo con errores léxicos
echo "📝 Probando errores léxicos..."
echo "--------------------------------"
./build/HULK test_errors.hulk

echo ""
echo "=================================================="
echo "✅ Pruebas completadas" 