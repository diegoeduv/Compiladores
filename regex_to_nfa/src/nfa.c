/**
 * @file nfa.c
 * @brief Implementación de la construcción y simulación de un NFA a partir de una expresión regular en notación postfija.
 *
 * Este módulo implementa:
 * - Gestión dinámica de estados y transiciones.
 * - Construcción de un NFA mediante el algoritmo de Thompson.
 * - Simulación del NFA utilizando epsilon-closure.
 */

#include "nfa.h"
#include "regex.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/**
 * @brief Inicializa un estado del NFA.
 *
 * @param s Puntero al estado a inicializar.
 *
 * Establece el arreglo de transiciones en NULL y reinicia los contadores.
 */
static void state_init(nfa_state *s) {
    s->trans = NULL;
    s->ntrans = 0;
    s->cap = 0;
}

/**
 * @brief Libera la memoria asociada a un estado del NFA.
 *
 * @param s Puntero al estado.
 */
static void state_free(nfa_state *s) {
    free(s->trans);
    s->trans = NULL;
    s->ntrans = 0;
    s->cap = 0;
}

/**
 * @brief Asegura capacidad suficiente para almacenar transiciones.
 *
 * @param s Estado objetivo.
 * @param new_cap Capacidad mínima requerida.
 *
 * Realiza realloc si la capacidad actual es insuficiente.
 */
static void state_reserve(nfa_state *s, int new_cap) {
    if (new_cap <= s->cap) return;

    int cap = s->cap ? s->cap : 8;
    while (cap < new_cap) cap *= 2;

    nfa_transition *p =
        (nfa_transition *)realloc(s->trans, (size_t)cap * sizeof(nfa_transition));
    if (!p) exit(1);

    s->trans = p;
    s->cap = cap;
}

/**
 * @brief Agrega una transición a un estado.
 *
 * @param s Estado origen.
 * @param to Estado destino.
 * @param symbol Símbolo asociado a la transición (0 representa epsilon).
 */
static void state_add_transition(nfa_state *s, int to, unsigned char symbol) {
    state_reserve(s, s->ntrans + 1);
    s->trans[s->ntrans].to = to;
    s->trans[s->ntrans].symbol = symbol;
    s->ntrans += 1;
}

/**
 * @brief Inicializa la estructura principal del NFA.
 *
 * @param n Puntero al NFA.
 */
static void nfa_init(nfa *n) {
    n->states = NULL;
    n->nstates = 0;
    n->cap = 0;
    n->start = -1;
    n->accept = -1;
}

/**
 * @brief Reserva espacio para estados adicionales.
 *
 * @param n NFA objetivo.
 * @param new_cap Capacidad mínima requerida.
 */
static void nfa_reserve_states(nfa *n, int new_cap) {
    if (new_cap <= n->cap) return;

    int cap = n->cap ? n->cap : 16;
    while (cap < new_cap) cap *= 2;

    nfa_state *p =
        (nfa_state *)realloc(n->states, (size_t)cap * sizeof(nfa_state));
    if (!p) exit(1);

    n->states = p;
    n->cap = cap;
}

/**
 * @brief Crea un nuevo estado en el NFA.
 *
 * @param n NFA objetivo.
 * @return Identificador del nuevo estado.
 */
static int nfa_new_state(nfa *n) {
    nfa_reserve_states(n, n->nstates + 1);
    int id = n->nstates;
    state_init(&n->states[id]);
    n->nstates += 1;
    return id;
}

/**
 * @brief Agrega una transición entre estados.
 */
static void nfa_add_transition(nfa *n, int from, int to, unsigned char symbol) {
    state_add_transition(&n->states[from], to, symbol);
}

/**
 * @brief Libera completamente un NFA.
 *
 * @param n Puntero al NFA.
 */
void nfa_free(nfa *n) {
    if (!n) return;

    for (int i = 0; i < n->nstates; i++) {
        state_free(&n->states[i]);
    }

    free(n->states);
    n->states = NULL;
    n->nstates = 0;
    n->cap = 0;
    n->start = -1;
    n->accept = -1;
}