# pl

Filtro de json con la sintaxis de graphql, no buscamos imitar consultas
graphql.

Permitimos más cosas que en graphql. En graphql, el siguiente ejemplo
no sería válido:

```console
{
	continents
}
```

Pero esto no nos parece muy útil, ya que si quisiéramos obtener todos
los campos del json, tendríamos que especificar todos los campos, pero eso
no nos parece bien.

Nosotros permitimos que, si no se especifica ningún campo, se devuelvan
todos los campos del json.

## TODO

Numeros negativos

Comparar case insensitive? con modifiers en cada operacion?

Hacer los errores de las operaciones en la gramatica
