# Practica 3

**Autor: Torresetti Lisandro**

## Objetivo

Implementar un módulo de software para trabajar con retardos no bloqueantes a partir de las funciones creadas en la práctica 2.
Esta práctica apunta a crear distintos directorios, fomo files para lograr un mejor encapsulamiento del código y evitar que toda la lógica se encuentre en el archivo _main.c_

A continuación se describe lo que requiere cada punto de esta práctica:
- **Punto 1:** crear los directorios y archivos a los cuales se van a migrar las funciones de la práctica anterior (la 2)
- **Punto 2:** Hacer titilar el LED de la place STM32 NucleF446RE a distintas frecuencias, y además utilizar las estructuras y funciones creadas en la práctica anterior
- **Punto 3:** Implementar una nueva función que permita obtener el estado del _delay_ sin modificarlo. Esta función debe devolver `true` si el delay sigue corriendo y `false` en caso contrario. Es necesario utilizar esta función para saber cuando es posible actualizar el _delay_