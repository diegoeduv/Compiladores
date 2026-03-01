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

/* Fragmento NFA usado por Thompson: par (inicio, aceptación). */
typedef struct {
    int start;
    int accept;
} frag;

/* Pila dinámica de fragmentos para construir el NFA en una sola pasada sobre postfix. */
typedef struct {
    frag *data;
    int size;
    int cap;
} frag_stack;

static void fs_init(frag_stack *s) {
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

static void fs_free(frag_stack *s) {
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

/* Reserva capacidad para la pila de fragmentos (crecimiento geométrico). */
static void fs_reserve(frag_stack *s, int new_cap) {
    if (new_cap <= s->cap) return;
    int cap = s->cap ? s->cap : 16;
    while (cap < new_cap) cap *= 2;
    frag *p = (frag *)realloc(s->data, (size_t)cap * sizeof(frag));
    if (!p) exit(1);
    s->data = p;
    s->cap = cap;
}

static void fs_push(frag_stack *s, frag f) {
    fs_reserve(s, s->size + 1);
    s->data[s->size++] = f;
}

static frag fs_pop(frag_stack *s) {
    if (s->size == 0) {
        frag bad = { -1, -1 };
        return bad;
    }
    return s->data[--s->size];
}

/* Determina si un token del postfix es literal (no operador). */
static int is_literal_token(char c) {
    return !(c == '|' || c == '*' || c == '+' || c == '?' || c == '.' ||
             c == '(' || c == ')' ||
             c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\0');
}

/*
 * Construye un NFA a partir de una regex en notación postfija.
 * Implementa Thompson usando una pila de fragmentos (start, accept).
 */
nfa regex_to_nfa(regex r) {
    nfa n;
    nfa_init(&n);

    frag_stack st;
    fs_init(&st);

    for (int i = 0; i < r.size; i++) {
        char t = r.items[i].value;

        /* Literal: crea fragmento de 2 estados con transición etiquetada. */
        if (is_literal_token(t)) {
            int s = nfa_new_state(&n);
            int a = nfa_new_state(&n);
            nfa_add_transition(&n, s, a, (unsigned char)t);
            fs_push(&st, (frag){ s, a });
            continue;
        }

        /* Concatenación: conecta accept de f1 con start de f2 mediante epsilon. */
        if (t == '.') {
            frag f2 = fs_pop(&st);
            frag f1 = fs_pop(&st);
            if (f1.start < 0 || f2.start < 0) exit(1);

            nfa_add_transition(&n, f1.accept, f2.start, NFA_EPSILON);
            fs_push(&st, (frag){ f1.start, f2.accept });
            continue;
        }

        /* Alternación: nuevo start y accept, con ramificación epsilon hacia ambos fragmentos. */
        if (t == '|') {
            frag f2 = fs_pop(&st);
            frag f1 = fs_pop(&st);
            if (f1.start < 0 || f2.start < 0) exit(1);

            int s = nfa_new_state(&n);
            int a = nfa_new_state(&n);

            nfa_add_transition(&n, s, f1.start, NFA_EPSILON);
            nfa_add_transition(&n, s, f2.start, NFA_EPSILON);
            nfa_add_transition(&n, f1.accept, a, NFA_EPSILON);
            nfa_add_transition(&n, f2.accept, a, NFA_EPSILON);

            fs_push(&st, (frag){ s, a });
            continue;
        }

        /* Kleene star: permite 0 o más repeticiones (incluye transición epsilon start->accept). */
        if (t == '*') {
            frag f = fs_pop(&st);
            if (f.start < 0) exit(1);

            int s = nfa_new_state(&n);
            int a = nfa_new_state(&n);

            nfa_add_transition(&n, s, a, NFA_EPSILON);
            nfa_add_transition(&n, s, f.start, NFA_EPSILON);
            nfa_add_transition(&n, f.accept, f.start, NFA_EPSILON);
            nfa_add_transition(&n, f.accept, a, NFA_EPSILON);

            fs_push(&st, (frag){ s, a });
            continue;
        }

        /* Plus: 1 o más repeticiones (no permite vacío). */
        if (t == '+') {
            frag f = fs_pop(&st);
            if (f.start < 0) exit(1);

            int a = nfa_new_state(&n);

            nfa_add_transition(&n, f.accept, a, NFA_EPSILON);
            nfa_add_transition(&n, f.accept, f.start, NFA_EPSILON);

            fs_push(&st, (frag){ f.start, a });
            continue;
        }

        /* Question mark: 0 o 1 ocurrencia. */
        if (t == '?') {
            frag f = fs_pop(&st);
            if (f.start < 0) exit(1);

            int s = nfa_new_state(&n);
            int a = nfa_new_state(&n);

            nfa_add_transition(&n, s, a, NFA_EPSILON);
            nfa_add_transition(&n, s, f.start, NFA_EPSILON);
            nfa_add_transition(&n, f.accept, a, NFA_EPSILON);

            fs_push(&st, (frag){ s, a });
            continue;
        }

        exit(1);
    }

    /* Al terminar, debe quedar exactamente un fragmento en la pila. */
    frag result = fs_pop(&st);
    if (st.size != 0 || result.start < 0) {
        fs_free(&st);
        nfa_free(&n);
        exit(1);
    }

    n.start = result.start;
    n.accept = result.accept;

    fs_free(&st);
    return n;
}

/* Operaciones sobre conjuntos de estados activos representados como bitset uint8_t. */
static void set_clear(uint8_t *set, int n) { memset(set, 0, (size_t)n); }
static void set_copy(uint8_t *dst, const uint8_t *src, int n) { memcpy(dst, src, (size_t)n); }
static int set_contains(const uint8_t *set, int idx) { return set[idx] != 0; }
static void set_add(uint8_t *set, int idx) { set[idx] = 1; }

/*
 * Calcula la epsilon-closure del conjunto de entrada "in".
 * Marca en "out" todos los estados alcanzables desde "in" usando solo transiciones epsilon.
 */
static void epsilon_closure(const nfa *n, const uint8_t *in, uint8_t *out) {
    set_clear(out, n->nstates);

    int *stack = (int *)malloc((size_t)n->nstates * sizeof(int));
    if (!stack) exit(1);
    int top = 0;

    for (int i = 0; i < n->nstates; i++) {
        if (in[i]) {
            out[i] = 1;
            stack[top++] = i;
        }
    }

    while (top > 0) {
        int s = stack[--top];
        const nfa_state *st = &n->states[s];

        for (int k = 0; k < st->ntrans; k++) {
            nfa_transition tr = st->trans[k];
            if (tr.symbol == NFA_EPSILON && !out[tr.to]) {
                out[tr.to] = 1;
                stack[top++] = tr.to;
            }
        }
    }

    free(stack);
}

/*
 * Move: dado un conjunto de estados "in" y un símbolo c,
 * produce en "out" el conjunto de estados alcanzables consumiendo exactamente c.
 */
static void move_on_char(const nfa *n, const uint8_t *in, unsigned char c, uint8_t *out) {
    set_clear(out, n->nstates);

    for (int s = 0; s < n->nstates; s++) {
        if (!in[s]) continue;

        const nfa_state *st = &n->states[s];
        for (int k = 0; k < st->ntrans; k++) {
            nfa_transition tr = st->trans[k];
            if (tr.symbol == c) out[tr.to] = 1;
        }
    }
}

/*
 * Simula el NFA sobre una cadena (str, len) usando epsilon-closure.
 * Devuelve 1 si accept es alcanzable tras consumir toda la cadena, 0 en caso contrario.
 */
int match_nfa(nfa n, const char *str, size_t len) {
    if (n.start < 0 || n.accept < 0) return 0;

    uint8_t *cur = (uint8_t *)malloc((size_t)n.nstates);
    uint8_t *tmp = (uint8_t *)malloc((size_t)n.nstates);
    uint8_t *nxt = (uint8_t *)malloc((size_t)n.nstates);
    if (!cur || !tmp || !nxt) exit(1);

    set_clear(cur, n.nstates);
    set_add(cur, n.start);

    epsilon_closure(&n, cur, tmp);
    set_copy(cur, tmp, n.nstates);

    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)str[i];
        move_on_char(&n, cur, c, tmp);
        epsilon_closure(&n, tmp, nxt);
        set_copy(cur, nxt, n.nstates);
    }

    int ok = set_contains(cur, n.accept);

    free(cur);
    free(tmp);
    free(nxt);
    return ok;
}