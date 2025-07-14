
// inheritance.c

#include "inheritance.h"
#include <string.h>
#include <stdlib.h>

/// @brief Buscar el nombre de un tipo en la cadena de herencia
/// @param type_name el nombre del tipo a buscar
/// @param chain la cadena de herencia
/// @return 1 si la encuentra ; 0 si no la encuentra
int find_type_in_inheritance_chain(char *type_name, InheritanceChain *chain)
{
    while (chain)
    {
        if (!strcmp(chain->type_name, type_name))
            return 1;
        chain = chain->next;
    }
    return 0;
}

/// @brief Agrega un tipo nuevo a la cadena de herencia
/// @param iCh la candea de herencia a manipular
/// @param new_type el nombre del tipo
/// @return Una cadena de herencia con el nuevo tipo a la cabeza
InheritanceChain *extend_inheritance_chain(InheritanceChain *iCh, char *new_type)
{
    if(!strcmp(new_type,"")) return iCh;

    InheritanceChain* new_iCh = malloc(sizeof(InheritanceChain));
    new_iCh->type_name = new_type;
    new_iCh->next = iCh;

    return new_iCh;
}

/// @brief Liberar recursivamente una estructra de cadena de herencia
/// @param iCh La cadena a eliminar
/// @return 
InheritanceChain *free_inheritance_chain(InheritanceChain *iCh)
{   
    // if(!iCh) return NULL;

    // free_inheritance_chain(iCh->next);

    // free(iCh->type_name);
    // free(iCh);

    if(iCh) free(iCh->next);
    free(iCh);

    return NULL;
}



