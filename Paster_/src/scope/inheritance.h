
#ifndef INHERITANCE_H
#define INHERITANCE_H

typedef struct InheritanceChain {
    char* type_name;
    struct InheritanceChain* next;
} InheritanceChain;


int find_type_in_inheritance_chain(char* type_name, InheritanceChain* chain);
InheritanceChain* free_inheritance_chain(InheritanceChain* iCh);
InheritanceChain* extend_inheritance_chain(InheritanceChain* iCh, char* new_type);

#endif