
//keywords.c

#include "keyword.h"
#include <string.h>

char * keywords[]=
{
    "string","number","boolean","true","false",
    "function","let","in", "is","as","type",
    "new","inherits","while"
};

char scape_chars []= {'n','t','\\','\"'};

int match_keyword(char* name)
{
    int keywords_count = sizeof(keywords)/sizeof(char*);

    for(int i=0;i<keywords_count;i++)
    {
        if(!strcmp(keywords[i],name)) return 1;
    }

    return 0;
}