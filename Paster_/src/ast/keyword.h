

//keyword.h

#ifndef KEYWORD_H
#define KEYWORD_H

extern char* keywords[];
extern char scape_chars[];

int match_keyword(char* name);

#endif