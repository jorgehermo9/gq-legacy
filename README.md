<div align="center"> <h1><strong>GQ</strong></h1> </div>
<div align="center">

A **fast** and **feature rich** JSON filtering tool

</div>

#

<details open>
<summary><strong>&nbsp;FEATURES &nbsp;✨</strong></summary>
<br>

- **Filtering**: filter JSON objects getting only the fields you want.
- **Url and file compatibility**: it can read as input a JSON file, got from a url, a file or from stdin.
- **All JSON types**: it supports all JSON types including arrays, objects...
- **Aliases**: you can use aliases to rename fields in the output file.
- **Conditions**: you can establish conditions that field's values must meet.
  (To see a detailed list of available operations refer to [Operations](#operations))
- **Error proof**: (almost) all possible errors and warnings are handled and reported to the user using a custom lexer and parser.

</details>

#

<details open>
<summary><strong>&nbsp;INSTALLATION &nbsp;🛠</strong></summary>
<br>

...

</details>

#

<details open>
<summary><strong>&nbsp;USAGE &nbsp;🪧</strong></summary>

Let's see how easy is to use GQ to filter a JSON file.

Over this JSON:

```json
{
  "user": "John",
  "bill": {
    "purchaser": "One S.A",
    "seller": "Two S.A",
    "products": [
      {
        "name": "Product 1",
        "quantity": 3,
        "price": { "value": 1.0, "currency": "EUR" },
        "tags": ["tag1", "tag2"]
      },
      {
        "name": "Product 2",
        "quantity": 1,
        "price": { "value": 2.0, "currency": "USD" },
        "tags": ["tag3"]
      }
    ]
  }
}
```

- We can use this simple graphql-like query to get only the name and price of each product:

```graphql
{
  bill {
    products {
      name
      price
    }
  }
}
```
