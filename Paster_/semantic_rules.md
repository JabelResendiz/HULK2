



1. Una variables solo puede ser definida una vez en todo el programa
2. Los nombres de variables y funciones no comparten el mismo ambito (se pueden lalmar igual)
3. No se pueden redefinir las funciones predefinidas
4. Una función puede tener distintas definiciones siempre que tengan distinta cantidad de argumentos (es decir, funciones del mismo nombre pero con cantidad de
argumentos distintos se consideran funciones distintas).
5. Toda variable y función tiene que haber sido definida antes de ser usada en una
expresión (salvo las funciones pre-definidas).
6. Todos los argumentos definidos en una misma función tienen que ser diferentes
entre sí, aunque pueden ser iguales a variables definidas globalmente o a argumentos definidos en otras funciones
7. En el cuerpo de una función, los nombres de los argumentos ocultan los nombres de
variables iguales.



1. `alloca` en LLVM es una instruccion que reserva espacio en el stack( no en el heap) para una variable
2. Equivale a declaraciones como `int x` en C donde x vive en el stack

```llvm
%x = alloc i32 ; Reserva espacio para un entero (i32) en el stack
```

3. `LLVMTypeRef` es un tipo de dato en LLVM que representa:
- Numeros (double, int)
- cadenas (char*)
- booleanos(bool)
- estructuras

`Piensa en LLVMTypeRef es como una etiqueta que le dice a LLVM que esa variable es un numero, texto`


4. Instrucciones LLVM clave:

- LLVMFunctionType(): Define el tipo de una función (retorno + parámetros).

- LLVMAddFunction(): Añade una función al módulo actual.

- LLVMAppendBasicBlock(): Crea un bloque básico (conjunto de instrucciones).

- LLVMPositionBuilderAtEnd(): Posiciona el "builder" (herramienta para generar código) en un bloque.

- LLVMGetInsertBlock() / LLVMGetBasicBlockTerminator(): Verifican si el bloque actual ya termina con un ret o br.

- LLVMBuildRet(): Añade una instrucción ret (retorno de función).

- LLVMPrintModuleToFile(): Escribe el módulo LLVM a un archivo.