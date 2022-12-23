# Autores

- David Rodríguez Bacelar (david.rbacelar@udc.es)
- Jorge Hermo González (jorge.hermo.gonzalez@udc.es)

# Objetivos

El principal objetivo de la práctica es el de **permitir filtrar un archivo JSON** utilizando una sintaxis muy similar a la
de GraphQL.

Actualmente, existen herramientas como jq que permiten procesar JSONs de forma muy sencilla, pero a medida que aumenta el
tamaño o la complejidad del archivo, su uso se vuelve mucho más complicado.
Sin embargo nuestra herramienta, basándonos en la sintaxis de GraphQL, permitirá a los usuarios realizar un procesado del JSON
de manera intuitiva y sencilla, permitiendo obtener sólo la información que necesitan de dicho JSON (el cual puede llegar
a ser muy grande).

## Requistios funcionales

Hemos decidido que nuestra herramienta debe cumplir los siguientes requisitos funcionales:

- Debe permitir filtrar un JSON por los campos que lo componen.
- Debe permitir como entrada un archivo JSON, ya sea desde un fichero, desde la entrada estándar o desde una URL.
- Debe permitir mostrar el resultado en la salida estándar o guardarlo en un fichero.
- Debe permitir trabajar con cualquier tipo de JSON (por ejemplo, con arrays).
- Debe permitir el renombrado de los campos (aliases) en el JSON resultante.
- Debe permitir argumentos de GraphQL, es decir, condiciones que tienen que cumplir
  los campos de un JSON array para que se incluyan o no.
- Debe permitir las siguientes operaciones en los argumentos:
  - Igualdad
  - Distinto
  - Mayor que (números)
  - Menor que (números)
  - Mayor o igual que (números)
  - Menor o igual que (números)
  - Contiene (strings)
  - No contiene (strings)
  - Empieza por (strings)
  - No empieza por (strings)
- Debe permitir que las operaciones sobre strings se realicen de forma case-insensitive o no.
- Debe mostrar mensajes de error detallados (con línea y columna donde se ha producido) de tipo sintácticos, léxicos,
  semánticos y de cualquier otro tipo.

# Estructura del proyecto

En el directorio `src` nos encontramos todo el código fuente de la práctica, que tiene la siguiente estructura:

```console
src/
├── lib/
│   ├── argparse.hpp
│   └── json.hpp
├── test/
│   ├── lexer
│   ├── main
│   ├── parser
│   └── run_all_tests.sh
├── errors.cpp
├── errors.hpp
├── filter.cpp
├── filter.hpp
├── types.cpp
├── types.hpp
├── gq.l
├── gq.y
├── main.cpp
└── Makefile
```

- En el directorio `lib` se encuentran las librerías que hemos utilizado para la práctica. En este caso, la librería `argparse.hpp`
  nos permite parsear los argumentos de la línea de comandos, y la librería `json.hpp` nos permite parsear el JSON de entrada.

- En el directorio `test` se encuentran los tests de la práctica (en total 122). En este caso, los tests están divididos en tres directorios:

  - `lexer`: tests para el lexer. En total hay 1 test de error.
  - `parser`: tests para la gramática. En total hay 20 tests de error.
  - `main`: tests para el programa principal. En total hay 91 tests, de los cuales 36 son de error, 5 son de warning y 54 son de ejecución correcta.

  Además, se proporciona un script `run_all_tests.sh` que ejecuta todos los tests de forma automática.

- En los ficheros `errors.cpp` y `errors.hpp` se encuentran las funciones que se encargan de mostrar los errores y warnings del main.

- En los ficheros `filter.cpp` y `filter.hpp` se encuentran las funciones que se encargan de filtrar el JSON de entrada. Esto contiene
  la mayor parte de la lógica de nuestro programa.

- En los ficheros `types.cpp` y `types.hpp` se encuentran los _structs_ que representan la _query_, los cuales se construyen en el parser
  y se utilizan en el main para filtrar el JSON.

- En los ficheros `gq.l` y `gq.y` se encuentran los ficheros de flex y bison, respectivamente, que se encargan de parsear la entrada
  y construir la representación de la _query_ en memoria.

- En el fichero `main.cpp` se encuentra el código principal del programa, que se encarga de parsear los argumentos de la línea de comandos,
  de parsear el JSON de entrada, de llamar al parser y finalmente a la función de filtrado.

- En el fichero `Makefile` se encuentran las reglas para compilar el programa.

# Lexer

Ya que los ficheros que tenemos que parsear no tienen muchos tokens, el analizador léxico no es muy grande.
La funcionalidad más importante que tiene es la construcción de los _structs_ básicos de la _query_.Además, también se ecarga de guardar
el número de fila y columna del token en el fichero original, para poder posteriormente mostrar mensajes de error más detallados.

# Parser

El parser se encarga de construir la representación de la _query_ en memoria.
Para definir el _happy path_ de la gramática, nos basamos en una descomposición recursiva de la _query_.
Asumimos entonces que:

- Una _query_ tiene una clave.
- Una _query_ puede estar formada por otras _queries_.
- Una _query_ puede tener además un alias y una lista de argumentos.
- A su vez, un argumento está formado por una clave, una operación y un valor.

Definir dicho _happy path_ no fue realmente complicado, sin embargo, al querer darle tanto importancia a los errores,
la gramática se volvió mucho más compleja, ya que queríamos que en los diferentes casos, los errores fuesen lo más detallados
posibles.

Cuando no era factible enumerar todos los posibles tokens que podían dar lugar a cierto error, optamos por utilizar
el token comodín `error`.

Cabe destacar el tratamiento de los errores en los argumentos (reglas del no terminal `argument`). En este caso, queríamos poder mostrar
en qué _query_ se había producido el error, por lo que tuvimos que propagar el error y el mensaje asociado hacia arriba para que en la regla asociada al no terminal `query_key` se pudiera mostrar dicho mensaje.

Una dificultad añadida fue que en vez tener la lógica de filtrado en las acciones semánticas de las propias reglas, decidimos
construir el árbol que representa una _query_ en dichas acciones para, finalmente, devolver la _query_ completa en el axioma de la gramática,