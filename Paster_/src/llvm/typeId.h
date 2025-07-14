

#ifndef TYPEID_H
#define TYPEID_H

#include <stdio.h>

#define HASH_SIZE 256

typedef struct TypeId
{
    char* type_name;
    int id;
    struct TypeId* next;
}TypeId;

// static TypeId* type_id = NULL;
static int next_type_id = 1;
static TypeId* type_id[HASH_SIZE];

unsigned int hash(const char* str);
void register_type_id(const char * type_name,int id);
int get_type_id(const char* type_name);

#endif
