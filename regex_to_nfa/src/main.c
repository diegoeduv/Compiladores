/**
 * @file main.c
 * @brief Punto de entrada del programa: conversión regex->postfijo y validación de cadenas vía NFA.
 *
 * El programa opera en dos modos, seleccionados por línea de comandos:
 *  - -r: Lee una regex desde stdin, la transforma a notación postfija con concatenación explícita y la imprime.
 *  - -t: Lee una regex desde stdin, construye un NFA y evalúa cada línea posterior como una cadena a validar.
 *
 * Formato de entrada (stdin):
 *  - Modo -r:
 *      Línea 1: expresión regular (sin salto de línea final relevante).
 *  - Modo -t:
 *      Línea 1: expresión regular.
 *      Líneas 2..EOF: cadenas a evaluar (una por línea).
 *
 * Formato de salida (stdout):
 *  - Modo -r:
 *      Expresión postfija en una sola línea.
 *  - Modo -t:
 *      Secuencia de '1' y '0' (sin espacios), donde cada dígito corresponde a una cadena de entrada.
 *
 * Errores:
 *  - Si el argumento no es válido o falta, imprime un mensaje de uso a stderr y termina con código 1.
 */

#include "regex.h"
#include "nfa.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

/**
 * @brief Imprime una expresión regular ya convertida a notación postfija.
 *
 * @param r Expresión regular en representación interna (arreglo de tokens).
 *
 */
void print_postfix(regex r)
{
    for (int i = 0; i < r.size; i++)
    {
        printf("%c", r.items[i].value);
    }
    printf("\n");
}

/**
 * @brief Lee cadenas desde stdin y evalúa aceptación contra el NFA construido desde la regex.
 *
 * @param regex_str Cadena con la expresión regular en notación infija (terminada en '\0').
 *
 */
void test_strings_stdin(const char *regex_str)
{
    regex r = parse_regex(regex_str);
    nfa n = regex_to_nfa(r);

    char buf[1024];
    while (fgets(buf, sizeof(buf), stdin))
    {
        buf[strcspn(buf, "\r\n")] = '\0';
        int result = match_nfa(n, buf, strlen(buf));
        printf("%d", result ? 1 : 0);
    }
    printf("\n");
}

/**
 * @brief Función principal: procesa argumentos y ejecuta el modo correspondiente.
 *
 * @param argc Número de argumentos.
 * @param argv Arreglo de argumentos.
 * @return 0 en ejecución exitosa, 1 en error de uso o lectura.
 *
 */
int main(int argc, char *argv[])
{
    int opt;
    char regex_str[1024];

    while ((opt = getopt(argc, argv, "rt")) != -1)
    {
        switch (opt)
        {
            case 'r':
                if (!fgets(regex_str, sizeof(regex_str), stdin))
                    return 1;
                regex_str[strcspn(regex_str, "\r\n")] = '\0';
                print_postfix(parse_regex(regex_str));
                return 0;

            case 't':
                if (!fgets(regex_str, sizeof(regex_str), stdin))
                    return 1;
                regex_str[strcspn(regex_str, "\r\n")] = '\0';
                test_strings_stdin(regex_str);
                return 0;

            default:
                fprintf(stderr, "Usage: %s -r | -t\n", argv[0]);
                return 1;
        }
    }

    fprintf(stderr, "Usage: %s -r | -t\n", argv[0]);
    return 1;
}