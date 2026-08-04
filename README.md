# Emux

Emux é uma linguagem de programação interpretada, cujo foco inicial é criar emuladores, é de médio nível(está entre Asm e C++), possui algumas peculiaridades como não ser possível definir variáveis localmente.

## Como usar

O código é dividido em secções (ex: `[Main]`), são basicamente um escopo(Obs:), o uso básico da linguagem é:

```emux
[Main]

func _Start() -> u8
{
	return 0
}
```

### Secções

Secções são uma espécie de escopo é definida assim: `[NomeDaSecção]`, que pode ter dependências(dependências que são outras secçoes, obrigatoriamente devem possuir a funcão \_Start), quando o \_Start da seccão atual for chamado, o \_Start de todas as dependências será chamado na ordem que foram declaradas. A secção Main e sua função \_Start é o ponto de entrada(entry point) do programa.

Como por exemplo:

```emux
[Banana]

func _Start() -> u8 {
	return 0
}

[Main]
func _Start() -> u8 {
	return 0
}
```

### Variáveis

Em Emux, variáveis só podem ser definidas na seccão Vars, e devem respeitar a sintaxe: `nome : tipo`, por exemplo `a : u8`.

Os tipos são definidos no primeiro caractere a seguir aos `:`, e são `u` de unsigned, `i` de int, `f` de float. Os caracteres seguintes representam o tamanho em bits caso a letra seja minuscúla `u`, ou bytes caso seja maiuscúla `U`. Ainda temos um tipo especial, `b` ou `B`, que representa um bloco na memória é o unico que pode ter mais que 64 bits(ou 8 bytes).

Os valores das váriaveis podem ser alterados com `=`.

Como por exemplo:

```
[Vars]

a : u8
b : i10

[Main]

func _Start() -> u8
{
	a = 5
	b = -10
	return 0
}
```