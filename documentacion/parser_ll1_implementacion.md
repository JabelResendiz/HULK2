# Implementación del Parser LL(1) Iterativo en C para HULK

## Resumen General
El parser LL(1) implementado en C para el lenguaje HULK es un analizador sintáctico predictivo que utiliza una tabla LL(1) para decidir qué producción aplicar en cada paso, de forma iterativa (sin recursión). El parser está diseñado para integrarse con un lexer que tokeniza el código fuente, y ambos trabajan juntos para construir el árbol de sintaxis concreta (CST).

## Componentes Principales

### 1. **Lexer**
- El lexer convierte el código fuente en una secuencia de tokens.
- Soporta comentarios de línea (`//`), los cuales son ignorados completamente durante el análisis léxico y no llegan al parser.
- El lexer puede ser generado (con autómatas) o implementado con Flex (`lexer.l`).

### 2. **Parser LL(1) Iterativo**
- Utiliza una pila para simular el proceso de derivación de la gramática.
- En cada paso, compara el símbolo en la cima de la pila con el token actual:
  - Si ambos son terminales e iguales, avanza el token y la pila.
  - Si la cima es un no terminal, consulta la tabla LL(1) para decidir qué producción aplicar.
  - Si la producción es epsilon, simplemente retira el no terminal de la pila.
- El parser construye el árbol CST a medida que aplica las producciones.

### 3. **Tabla LL(1)**
- Se construye a partir de la gramática, los conjuntos FIRST y FOLLOW.
- Permite decidir de forma eficiente qué producción aplicar para cada par (no terminal, terminal).

### 4. **Integración y Flujo**
1. El archivo fuente se lee y se tokeniza.
2. Los tokens se almacenan en un array y se pasa al parser.
3. El parser procesa los tokens usando la tabla LL(1) y construye el CST.
4. Si el análisis es exitoso, se imprime el árbol CST.

## Manejo de Comentarios
- Los comentarios de línea (`// ...`) son ignorados por el lexer y nunca llegan al parser.
- Esto se logra con un patrón especial en el lexer que detecta y descarta los comentarios.

## Ventajas de la Implementación
- **Iterativo:** No usa recursión, por lo que es más robusto ante gramáticas grandes.
- **Modular:** Lexer y parser están desacoplados y pueden evolucionar por separado.
- **Extensible:** Fácil de agregar nuevos tokens, reglas o soporte para más tipos de comentarios.

## Ejemplo de Flujo
```hulk
let x = 42 in print(x); // Esto es un comentario
```
- El lexer ignora el comentario.
- El parser procesa los tokens y construye el CST correspondiente.

---

**Autor:** [Tu nombre]
**Fecha:** [Fecha actual] 