#ifndef REGEX_H
#define REGEX_H

#include <stddef.h>

/* Token individual de la expresión regular */
typedef struct {
    char value; /* Literal u operador ('|', '*', '+', '?', '.', '(', ')') */
} regex_item;

/* Representación interna de la regex en notación postfija */
typedef struct {
    regex_item *items; /* Arreglo dinámico de tokens */
    int size; /* Número de tokens */
    int cap; /* Capacidad reservada */
} regex;

/* Convierte una regex infija a notación postfija con concatenación explícita */
regex parse_regex(const char *regex_str);

/* Libera la memoria asociada a la estructura regex */
void regex_free(regex *r);

#endif