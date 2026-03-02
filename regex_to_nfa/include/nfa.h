#ifndef NFA_H
#define NFA_H

#include <stddef.h>
#include "regex.h"

/* Valor utilizado para representar transiciones epsilon */
enum { NFA_EPSILON = 0 };

/* Transición entre estados */
typedef struct {
    int to; /* Estado destino */
    unsigned char symbol; /* Símbolo consumido (0 = epsilon) */
} nfa_transition;

/* Estado del NFA */
typedef struct {
    nfa_transition *trans; /* Arreglo dinámico de transiciones */
    int ntrans; /* Número de transiciones */
    int cap; /* Capacidad reservada */
} nfa_state;

/* Estructura principal del NFA */
typedef struct {
    nfa_state *states; /* Arreglo dinámico de estados */
    int nstates; /* Número total de estados */
    int cap; /* Capacidad reservada */
    int start; /* Estado inicial */
    int accept; /* Estado de aceptación */
} nfa;

/* Construye un NFA a partir de una regex en postfijo */
nfa regex_to_nfa(regex r);

/* Evalúa si una cadena es aceptada por el NFA */
int match_nfa(nfa n, const char *str, size_t len);

/* Libera la memoria asociada al NFA */
void nfa_free(nfa *n);

#endif