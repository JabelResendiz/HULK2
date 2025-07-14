# Makefile raíz para HulkCompiler-dev

.PHONY: compile execute clean debug

compile:
	$(MAKE) -C src all

execute:
	./src/hulk_compiler

clean:
	$(MAKE) -C src clean

# Abre el depurador GDB con el ejecutable principal
debug:
	gdb ./src/hulk_compiler
