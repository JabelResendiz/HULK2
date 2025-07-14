# 🧠 HULK Compiler

Compilador para el lenguaje **HULK**, desarrollado como un proyecto educativo en C utilizando **Flex**, **Bison**, **LLVM** y una arquitectura modular.

---

## 📁 Estructura

- **src/**: Contiene el `main.c` principal del compilador.
- **ast/**: Nodo y estructura del AST (Árbol de Sintaxis Abstracta).
- **check/**: Módulo de análisis semántico y chequeo de tipos.
- **lexer/**: Código generado por **Flex** a partir del analizador léxico.
- **parser/**: Código generado por **Bison** a partir del analizador sintáctico.
- **llvm/**: Módulo de generación de código LLVM IR.
- **llvm_scope/**: Manejador de contexto y scopes para LLVM.
- **scope/**: Gestión de ámbitos para el análisis semántico.
- **visitor/**: Patrón Visitor para recorrer el AST.
- **utils/**: Funciones auxiliares para impresión y utilidades.
- **build/**: Directorio de salida del ejecutable y archivos temporales.

---

## ⚙️ Comandos Make

```bash
make compile     # 🔧 Compila todo el proyecto y ejecuta el parser
make execute     # 🧪 Ejecuta el programa compilado desde LLVM IR
make clean       # 🧹 Limpia todos los archivos generados
make debug       # 🐞 Abre el depurador GDB con el ejecutable principal
```