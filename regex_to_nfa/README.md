# Regex to NFA / Regex a NFA

This project implements the conversion of a regular expression into a Non-Deterministic Finite Automaton (NFA) using Thompson’s construction, along with NFA simulation for string validation.  
Este proyecto implementa la conversión de una expresión regular en un Autómata Finito No Determinista (NFA) utilizando la construcción de Thompson, junto con la simulación del NFA para validar cadenas.

---

## Supported Operators / Operadores Soportados

- `|`  Alternation / Alternación  
- `*`  Zero or more / Cero o más  
- `+`  One or more / Uno o más  
- `?`  Zero or one / Cero o uno  
- `()` Grouping / Agrupación  
- Implicit concatenation / Concatenación implícita  

---

## Build with Docker (Recommended) / Compilación con Docker (Recomendado)

Build the image / Construir la imagen

```bash
docker build -t regex_to_nfa_validator .
```

Run the validator / Ejecutar el validador

```bash
docker run --rm regex_to_nfa_validator
```

## Manual Build (Without Docker) / Compilación Manual (Sin Docker)

Compile using CMake / Compilar usando CMake

```bash
cmake .
make
```

This generates the executable:
Esto genera el ejecutable:

```bash
./regex_to_nfa
```

## Mode 1: Convert to Postfix (-r) / Modo 1: Convertir a Postfijo (-r)

```bash
echo "a(b|c)*" | ./regex_to_nfa -r
```

Expected output / Salida esperada:

```text
abc|*.
```

## Mode 2: Validate Strings (`-t`) / Modo 2: Validar Cadenas (`-t`)

```bash
printf "%s\n" "(ab)*" "ab" "aba" "abab" | ./regex_to_nfa -t
```

Expected output / Salida esperada:

```text
101
```

Each digit corresponds to an evaluated string:
Cada dígito corresponde a una cadena evaluada:

- `1` → Accepted/Aceptada 
- `0` → Rejected/Rechazada