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

# Motivaciones

Existen varios métodos de exploración de los datos sobre archivos CSV, como por ejemplo `csvq`, el cual nos permite ejecutar consultas SQL sobre archivos CSV. Pero, los archivos CSV
no nos permiten establecer una jerarquía de relaciones de forma sencilla de manera que los datos
residan en un único archivo (Ya que tiene mucha similitud con una tabla de una base de datos relacional). Los archivos JSON sí que nos permiten establecer una jerarquía de relaciones
de forma sencilla, pero no permiten ejecutar consultas relacionales sobre ellos de forma sencilla, de ahí que surjan las llamadas bases de datos no relacionales (NoSQL).

Entonces,lo que queremos proporcionar es una manera sencilla de exploración de los datos en un archivo
JSON, de manera que el usuario pueda obtener la información que necesita sin tener que escribir algún
script, o depender de una base de datos NoSQL para poder realizar una tarea tan básica.

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
  - Termina por (strings)
  - No termina por (strings)
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

- En el directorio `test` se encuentran los tests de la práctica (en total 124). En este caso, los tests están divididos en tres directorios:

  - `lexer`: tests para el lexer. En total hay 1 test de error.
  - `parser`: tests para la gramática. En total hay 20 tests de error.
  - `main`: tests para el programa principal. En total hay 93 tests, de los cuales 43 son de error, 5 son de warning y 59 son de ejecución correcta.

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
construir el árbol que representa una _query_ en dichas acciones para, finalmente, devolver la _query_ completa en el axioma de la gramática.

# Manual de uso

Para la entrada de nuestro programa, necesitamos un archivo json y otro archivo graphql (la sintaxis no es exactamente la de graphql, así que explicaremos este
en detalle)

Para mostrar la sintaxis del archivo `graphql` utilizaremos el siguiente json de ejemplo:

```json
{
  "user": "David",
  "bill": {
    "title": "Hello",
    "purchaser": "Trile S.A",
    "seller": "Corunat S.A",
    "products": [
      {
        "name": "Product 1",
        "quantity": 3,
        "price": { "value": 1.0, "currency": "EUR" },
        "new": false,
        "tags": ["tag1", "tag2"]
      },
      {
        "name": "Product 2",
        "quantity": 1,
        "price": { "value": 2.0, "currency": "DOLLAR" },
        "new": true,
        "tags": ["tag3"]
      }
    ]
  },
  "config": {
    "font_size": 12,
    "font_style": "latex"
  }
}
```

## Filtrado de campos

Primero, una query está definida por un nombre, y, opcionalmente, por
un par de llaves que dentro contenga otras queries. Por ejemplo:

```graphql
query1

query2{
  field1{
    sub_field1
  }
  field2
}
```

En este caso, `query1` y `query2` son dos queries, y `query2` tiene dos campos, `field1` y `field2`, a su vez, `field1` tiene un subcampo `sub_field1`.

Un archivo `graphql` está bien formado si contiene dos llaves, y dentro de esas llaves puede tener o no más queries. Por ejemplo

```graphql
{

}
```

o bien,

```graphql
{
  query1
  query2 {
    field1
    field2
  }
}
```

Lo que haría la última query sería filtrar el json entrante para que solo se quedara con los campos `query1` y `query2`, y dentro de `query2` se quedarían los campos `field1` y `field2`.

Siguiendo el archivo json mostrado al inicio de la sección, la query

```graphql
{
  user
  bill {
    title
  }
}
```

Resultaría en el siguiente json:

```json
{
  "user": "David",
  "bill": {
    "title": "Hello"
  }
}
```

Cabe mencionar que permitimos algo que no es posible en GraphQl y nos parece muy útil, que es la opción de obtener todos los campos de un objecto sin tener que especificarlos explícitamente. Por ejemplo, la query

```graphql
{
  bill
}
```

Devolvería todos los campos del campo `bill`, y la query

```graphql
{

}
```

Devolvería todos los campos del json. Esto, en GraphQL devolvería un error.

## Aliases

Se pueden utilizar aliases para renombrar el campo que se quiere filtrar. Por ejemplo, si queremos que traducir los campos al español, la query sería:

```graphql
{
  user: usuario
  bill: factura {
    title: titulo
  }
}
```

Como se puede ver, hay que especificar el nuevo nombre del campo después de los dos puntos.

## Argumentos

Se pueden utilizar operadores para filtrar en los arrays de objectos por un cierto valor en sus campos. Esto
son los llamados **argumentos** en GraphQl. La sintaxis sería la siguiente:

```graphql
{
array_field(field1: value1, field2: value2, ...)
}
```

Los tipos disponibles para los valores son:

- `string`: se especifica entre comillas dobles
- `number`: puede ser tanto un entero como un valor decimal
- `boolean`: `true` o `false`
- `null`: tipo especial que representa el valor nulo

Hay varios operadores disponibles, siendo el default (si está vacío) el operador `=`. Los operadores disponibles son:

- `=`: igualdad
- `!=`: distinto
- `>`: mayor que (solo disponible para tipos numéricos)
- `<`: menor que (solo disponible para tipos numéricos)
- `>=`: mayor o igual que (solo disponible para tipos numéricos)
- `<=`: menor o igual que (solo disponible para tipos numéricos)
- `~`: contiene (solo disponible para tipos string)
- `!~`: no contiene (solo disponible para tipos string)
- `^`: empieza por (solo disponible para tipos string)
- `!^`: no empieza por (solo disponible para tipos string)
- `$`: termina por (solo disponible para tipos string)
- `!$`: no termina por (solo disponible para tipos string)

Además, las operaciones soportadas por los tipos string, permiten un modificador
para que la comprobación de la condición se haga de forma _case insentive_. Esto se especifica poniendo un `*` al final del operador. Por ejemplo:

```graphql
{
  array_field(field: ~*"text")
}
```

Siguiendo el ejemplo del json mostrado al inicio de la sección, la query

```graphql
{
  bill {
    products(name: ~*"pRODUct 1") {
      name
    }
  }
}
```

Tendría como resultado el siguiente json:

```json
{
  "bill": {
    "products": [
      {
        "name": "Product 1"
      }
    ]
  }
}
```

Y la query

```graphql
{
  bill {
    products(quantity: <2) {
      name
    }
  }
}
```

tendría como resultado el json

```json
{
  "bill": {
    "products": [
      {
        "name": "Product 2"
      }
    ]
  }
}
```

Podemos combinar los aliases y los filtros, para poder dividir los arrays, por ejemplo:

```graphql
{
  bill {
    products(new: true): new_products{
      name
    }
    products(new: false): old_products{
      name
    }
  }
}
```

Daría como resultado el json

```json
{
  "bill": {
    "new_products": [
      {
        "name": "Product 1"
      }
    ],
    "old_products": [
      {
        "name": "Product 2"
      }
    ]
  }
}
```

El filtrado se puede realizar sobre campos que sean un array de un tipo en concreto. La condición se cumplirá si **alguno** de los elementos del array cumple la condición. Por ejemplo, la siguiente query:

```graphql
{
  bill {
    products(tags: "tag1") {
      name
      tags
    }
  }
}
```

Devolvería el json

```json
{
  "bill": {
    "products": [
      {
        "name": "Product 1",
        "tags": ["tag1", "tag2"]
      }
    ]
  }
}
```

Se puede ver que el producto 2 no se devuelve, ya que no tiene el tag `tag1`, y el product 1 se devuelve aunque tenga un tag que no cumpla la condición (el otro sí).

Hasta ahora, sólo podíamos consultar los campos del top-level de un array de objectos, es decir, las que están inmediatamente presentes. Pero para poder filtrar utilizando campos que están _nested_ dentro de cada elemento del array, se puede utilizar el operador `.`, concatenando los distintos campos a los que queremos acceder. Por ejemplo, la siguiente query:

```graphql
{
  bill {
    products(price.currency: "EUR") {
      name
      price
    }
  }
}
```

Devolvería el json

```json
{
  "bill": {
    "products": [
      {
        "name": "Product 1",
        "price": {
          "currency": "EUR",
          "value": 1.0
        }
      }
    ]
  }
}
```

# Ejecución

## Compilación

> Asumimos que se está en el directorio `src/`.

Simplemente ejecutar:

```console
make
```

Esto generará el ejecutable `gq`.

## Test

> Asumimos que se está en el directorio `src/`.

Todos los test se pueden ejecutar con:

```console
make test
```

Esto realmente lo que hace es ejecutar el script `test/run_all_tests.sh` que lanza todos los test de forma automática.
A mayores, se puede ejecutar tests individuales utilizando el script `test/run_single_test.sh` de la siguiente forma:

```console
test/run_single_test.sh test/main/valid_array_query
test/run_single_test.sh test/main/error_invalid_json_file
test/run_single_test.sh test/parser/error_no_closing_query
test/run_single_test.sh test/lexer/error_invalid_character
```

Para ver todos los tests disponibles, se puede hacer ejecutando `ls test/parser`, `ls test/lexer` y `ls test/main`.

## Ejemplos

> Para evitar que se muestren los warnings, se puede ejecutar el programa con el parámetro --quiet (o -q).

Para ver la ayuda del programa, ejecutar `./gq -h`

Se han proporcionado algunos ejemplos de _queries_ en el directorio `examples/`.

Se pueden ejecutar con:

```bash
# Product examples

./gq examples/products/product_names.graphql -j examples/products/products.json -o product_names.json;
  cat product_names.json
./gq examples/products/product_new_and_old.graphql -j examples/products/products.json
./gq examples/products/product_zero_and_normal.graphql -j examples/products/products.json
./gq examples/products/product_zero_and_normal_cheap.graphql -j examples/products/products.json
```

```bash
# Pokemon examples

## Get pokemon name and its abilities
./gq examples/pokemon/pokemon_name_and_abilities.graphql -u "https://pokeapi.co/api/v2/pokemon/snorlax"

## Get pokemon names that ends with -ir
./gq examples/pokemon/pokemon_name_ends_with.graphql -u "https://pokeapi.co/api/v2/pokemon?limit=1154"

## Get best stats of a pokemon
./gq examples/pokemon/pokemon_best_stats.graphql -u "https://pokeapi.co/api/v2/pokemon/articuno"

## Get the movements that a pokemon can learn at level 0
./gq examples/pokemon/pokemon_level_0_moves.graphql -u "https://pokeapi.co/api/v2/pokemon/snorlax"
```

```bash
# Countries example

## Get the countries which uses euros as currency
./gq examples/countries/countries_euro.graphql -j examples/countries/countries.json -q

## Get the countries that are non-independent
./gq examples/countries/countries_non_independent.graphql -j examples/countries/countries.json -q

## Get the countries that have a population greater than 100 million people
./gq examples/countries/countries_most_populated.graphql -j examples/countries/countries.json -q

## Get the countries that borders Spain
./gq examples/countries/countries_borders_spain.graphql -j examples/countries/countries.json -q

## Get the countries in which the driving side is left
./gq examples/countries/countries_drive_left.graphql -j examples/countries/countries.json

## Get the countries in which common names contains the word `south`
./gq examples/countries/countries_southern.graphql -j examples/countries/countries.json
```
