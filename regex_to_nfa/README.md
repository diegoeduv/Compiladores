# Regex to NFA

Pequeño proyecto en C que:

- Convierte una expresión regular infija a notación postfija con concatenación explícita.
- Construye un Autómata Finito No Determinista (NFA) usando el algoritmo de Thompson.
- Simula el NFA para validar cadenas de texto.

Soporta los operadores:

- `|`  Alternación
- `*`  Cero o más
- `+`  Uno o más
- `?`  Cero o uno
- `()` Agrupación
- Concatenación implícita

---

## Compilación con Docker (recomendado)

Construir la imagen:

```bash
docker build -t regex_to_nfa_validator .
```

Ejecutar el validador automático:

```bash
docker run --rm regex_to_nfa_validator
```

## Uso manual (sin Docker)

Compilar con CMake:

```bash
cmake .
make
```

Esto genera el ejecutable:

```bash
./regex_to_nfa
```

## Modo 1: Convertir a postfijo (`-r`)

```bash
echo "a(b|c)*" | ./regex_to_nfa -r
```

Salida esperada:

```text
abc|*.
```

## Modo 2: Validar cadenas (`-t`)

```bash
printf "%s\n" "(ab)*" "ab" "aba" "abab" | ./regex_to_nfa -t
```

Salida:

```text
101
```

Cada dígito corresponde a una cadena evaluada:

- `1` → Aceptada
- `0` → Rechazada