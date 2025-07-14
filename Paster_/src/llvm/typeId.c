

#include "typeId.h"
#include <string.h>
#include <stdlib.h>

unsigned int hash(const char *str)
{
    unsigned int hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c;

    return hash % HASH_SIZE;
}

void register_type_id(const char *type_name, int id)
{
    unsigned int h = hash(type_name);
    TypeId *entry = malloc(sizeof(TypeId));
    entry->type_name = strdup(type_name);
    entry->id = id;
    entry->next = type_id[h];
    type_id[h] = entry;
}

int get_type_id(const char *type_name)
{
    unsigned int h = hash(type_name);
    TypeId *curr = type_id[h];
    while (curr)
    {
        if (strcmp(curr->type_name, type_name) == 0)
            return curr->id;
        curr = curr->next;
    }
    return -1; 
}
