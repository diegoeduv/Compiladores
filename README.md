# Compiler / Compiladores

This repository contains the development of a compiler as part of the Compilers course.

Este repositorio contiene el desarrollo de un compilador como parte del curso de Compiladores.

## Project Structure / Estructura del Proyecto

The project is built using **CMake** and follows a modular C structure:
El proyecto está construido utilizando **CMake** y sigue una estructura modular en C:

* `src/`: Core logic (Lexer, Parser, etc.). / Lógica central (Léxico, Sintáctico, etc.).
* `include/`: Header files. / Archivos de cabecera.
* `tests/`: Test suites and source code samples for compilation. / Pruebas y muestras de código para compilar.

## Branching Strategy / Estrategia de Ramas

To maintain code integrity and professional workflow, the following strategy will be followed:
Para mantener la integridad del código y un flujo de trabajo profesional, se seguirá la siguiente estrategia:

* **Main Branch**: Represents the "production" or stable state. No direct commits are allowed (except for the initial setup).
    **Rama Main**: Representa el estado estable o de "producción". No se permiten commits directos (excepto la configuración inicial).
* **Feature Branches (`feature/name`)**: Used for developing new compiler phases or components.
    **Ramas de Funcionalidad (`feature/nombre`)**: Utilizadas para desarrollar nuevas fases o componentes del compilador.
* **Fix Branches (`fix/name`)**: Used for bug fixes and patches.
    **Ramas de Corrección (`fix/nombre`)**: Utilizadas para correcciones de errores y parches.

*All features must be merged into main via Pull Request/Merge Request.*
*Todas las funcionalidades deben integrarse a main mediante Pull Request/Merge Request.*

---

## Requirements / Requisitos

* C Compiler (GCC/Clang)
* CMake (3.10+)