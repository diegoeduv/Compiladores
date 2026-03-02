/**
 * @file regex.c
 * @brief Implementación del parser de expresiones regulares.
 *
 * Este módulo transforma una expresión regular en notación infija
 * con concatenación implícita en una representación postfija con
 * concatenación explícita, utilizando el algoritmo de Shunting-Yard.
 *
 * Soporta los operadores:
 *  - Alternación: |
 *  - Cerradura de Kleene: *
 *  - Uno o más: +
 *  - Cero o uno: ?
 *  - Paréntesis: ( )
 */

#include "regex.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Inicializa una estructura regex vacía.
 *
 * @param r Puntero a la estructura regex.
 *
 * Postcondición:
 *  - r->items == NULL
 *  - r->size == 0
 *  - r->cap == 0
 */
static void regex_init(regex *r) {
    r->items = NULL;
    r->size = 0;
    r->cap = 0;
}

/**
 * @brief Asegura capacidad suficiente en el arreglo dinámico.
 *
 * @param r Expresión regular.
 * @param new_cap Capacidad mínima requerida.
 *
 */
static void regex_reserve(regex *r, int new_cap) {
    if (new_cap <= r->cap) return;

    int cap = r->cap ? r->cap : 16;
    while (cap < new_cap) cap *= 2;

    regex_item *p =
        (regex_item *)realloc(r->items, (size_t)cap * sizeof(regex_item));
    if (!p) exit(1);

    r->items = p;
    r->cap = cap;
}

/**
 * @brief Inserta un nuevo símbolo en la estructura regex.
 *
 * @param r Expresión regular.
 * @param v Símbolo a insertar.
 */
static void regex_push(regex *r, char v) {
    regex_reserve(r, r->size + 1);
    r->items[r->size].value = v;
    r->size += 1;
}

/**
 * @brief Libera la memoria asociada a una estructura regex.
 */
void regex_free(regex *r) {
    if (!r) return;
    free(r->items);
    r->items = NULL;
    r->size = 0;
    r->cap = 0;
}

/**
 * @brief Determina si un carácter es espacio en blanco.
 */
static int is_space(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

/**
 * @brief Determina si un carácter es un literal.
 *
 * Un literal es cualquier símbolo que no sea operador ni espacio.
 */
static int is_literal(char c) {
    if (is_space(c)) return 0;

    if (c == '.' || c == '|' || c == '*' ||
        c == '+' || c == '?' ||
        c == '(' || c == ')')
        return 0;

    return 1;
}

/**
 * @brief Determina si un símbolo puede finalizar un átomo.
 */
static int is_atom_end(char c) {
    return is_literal(c) || c == ')' || c == '*' || c == '+' || c == '?';
}

/**
 * @brief Determina si un símbolo puede iniciar un átomo.
 */
static int is_atom_start(char c) {
    return is_literal(c) || c == '(';
}

/**
 * @brief Inserta explícitamente el operador de concatenación '.'.
 *
 * @param infix Expresión en notación infija.
 * @return Nueva expresión con concatenación explícita.
 *
 * Complejidad: O(n)
 */
static regex insert_concat_explicit(const char *infix) {
    regex out;
    regex_init(&out);

    char prev = 0;
    int has_prev = 0;

    for (size_t i = 0; infix[i] != '\0'; i++) {
        char c = infix[i];
        if (is_space(c)) continue;

        if (!has_prev) {
            regex_push(&out, c);
            prev = c;
            has_prev = 1;
            continue;
        }

        if (is_atom_end(prev) && is_atom_start(c)) {
            regex_push(&out, '.');
        }

        regex_push(&out, c);
        prev = c;
    }

    return out;
}

/**
 * @brief Devuelve la precedencia de un operador.
 *
 * Mayor valor implica mayor precedencia.
 */
static int precedence(char op) {
    switch (op) {
        case '*':
        case '+':
        case '?': return 3;
        case '.': return 2;
        case '|': return 1;
        default:  return 0;
    }
}

/**
 * @brief Determina si un operador es asociativo a la izquierda.
 */
static int is_left_associative(char op) {
    return (op == '.' || op == '|');
}

/**
 * @brief Estructura auxiliar para pila de operadores.
 */
typedef struct {
    char *data;
    int size;
    int cap;
} char_stack;

/**
 * @brief Inicializa pila.
 */
static void cs_init(char_stack *s) {
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

/**
 * @brief Libera pila.
 */
static void cs_free(char_stack *s) {
    free(s->data);
    s->data = NULL;
    s->size = 0;
    s->cap = 0;
}

/**
 * @brief Reserva capacidad para la pila.
 */
static void cs_reserve(char_stack *s, int new_cap) {
    if (new_cap <= s->cap) return;

    int cap = s->cap ? s->cap : 16;
    while (cap < new_cap) cap *= 2;

    char *p = (char *)realloc(s->data, (size_t)cap);
    if (!p) exit(1);

    s->data = p;
    s->cap = cap;
}

/**
 * @brief Inserta operador en pila.
 */
static void cs_push(char_stack *s, char v) {
    cs_reserve(s, s->size + 1);
    s->data[s->size++] = v;
}

/**
 * @brief Extrae operador de la pila.
 */
static char cs_pop(char_stack *s) {
    if (s->size == 0) return 0;
    return s->data[--s->size];
}

/**
 * @brief Consulta el elemento superior de la pila.
 */
static char cs_top(const char_stack *s) {
    if (s->size == 0) return 0;
    return s->data[s->size - 1];
}

/**
 * @brief Indica si la pila está vacía.
 */
static int cs_empty(const char_stack *s) {
    return s->size == 0;
}

/**
 * @brief Convierte expresión infija (con concatenación explícita) a postfija.
 *
 * @param with_concat Expresión con operador '.' explícito.
 * @return Expresión en notación postfija.
 *
 * Complejidad: O(n)
 */
static regex to_postfix(const regex *with_concat) {
    regex out;
    regex_init(&out);

    char_stack ops;
    cs_init(&ops);

    for (int i = 0; i < with_concat->size; i++) {
        char t = with_concat->items[i].value;

        if (is_literal(t)) {
            regex_push(&out, t);
            continue;
        }

        if (t == '(') {
            cs_push(&ops, t);
            continue;
        }

        if (t == ')') {
            while (!cs_empty(&ops) && cs_top(&ops) != '(') {
                regex_push(&out, cs_pop(&ops));
            }
            if (!cs_empty(&ops) && cs_top(&ops) == '(') {
                cs_pop(&ops);
            } else {
                cs_free(&ops);
                regex_free(&out);
                exit(1);
            }
            continue;
        }

        if (t == '*' || t == '+' || t == '?') {
            regex_push(&out, t);
            continue;
        }

        while (!cs_empty(&ops)) {
            char top = cs_top(&ops);
            if (top == '(') break;

            int p_top = precedence(top);
            int p_t = precedence(t);

            if (p_top > p_t || (p_top == p_t && is_left_associative(t))) {
                regex_push(&out, cs_pop(&ops));
            } else {
                break;
            }
        }

        cs_push(&ops, t);
    }

    while (!cs_empty(&ops)) {
        char top = cs_pop(&ops);
        if (top == '(' || top == ')') {
            cs_free(&ops);
            regex_free(&out);
            exit(1);
        }
        regex_push(&out, top);
    }

    cs_free(&ops);
    return out;
}

/**
 * @brief Punto de entrada del parser de expresiones regulares.
 *
 * @param regex_str Expresión regular en notación infija.
 * @return Expresión regular en notación postfija con concatenación explícita.
 * 
 * Primero inserta concatenación explícita y aplica Shunting-Yard.
 *
 * Complejidad total: O(n)
 */
regex parse_regex(const char *regex_str) {
    regex with_concat = insert_concat_explicit(regex_str);
    regex postfix = to_postfix(&with_concat);
    regex_free(&with_concat);
    return postfix;
}