# MD5 (Message Digest 5)

> Prérequis : lire **[Merkle–Damgård](merkle-damgard.md)** d'abord. On ne décrit
> ici que ce qui est **spécifique à MD5**.

## Fiche technique

| Propriété              | Valeur                                   |
|------------------------|------------------------------------------|
| Taille du digest       | 128 bits = **16 octets**                 |
| Mots d'état            | **4** mots de 32 bits (A, B, C, D)       |
| Taille de bloc         | 512 bits = **64 octets**                 |
| Champ longueur         | 64 bits, **little-endian**               |
| Endianness             | **little-endian** partout                |
| Nombre de rondes       | **64** (4 groupes de 16)                 |

## État initial (IV)

```
   A = 0x67452301
   B = 0xefcdab89
   C = 0x98badcfe
   D = 0x10325476
```

→ Code : `g_md5_iv` dans `src/hash/md5.c`.

## Les constantes

- **K[0..63]** : `K[i] = floor(2^32 × |sin(i + 1)|)` (i en radians). 64 valeurs.
  → `g_md5_k`.
- **s[0..63]** : les montants de **rotation gauche** par ronde, un motif de 4
  valeurs répété par groupe (`{7,12,17,22}`, `{5,9,14,20}`, `{4,11,16,23}`,
  `{6,10,15,21}`). → `g_md5_s`.

## La compression d'un bloc

On lit le bloc de 64 octets comme **16 mots de 32 bits en little-endian** :
`M[0..15]`. On copie l'état courant dans 4 registres de travail `a, b, c, d`, puis
on fait **64 rondes**. Chaque ronde utilise une **fonction non-linéaire** qui
change selon le groupe :

| Rondes  | Fonction                         | Index du mot message `g` |
|---------|----------------------------------|--------------------------|
| 0–15    | `F = (B ∧ C) ∨ (¬B ∧ D)`          | `g = i`                  |
| 16–31   | `G = (D ∧ B) ∨ (¬D ∧ C)`          | `g = (5i + 1) mod 16`    |
| 32–47   | `H = B ⊕ C ⊕ D`                   | `g = (3i + 5) mod 16`    |
| 48–63   | `I = C ⊕ (B ∨ ¬D)`               | `g = (7i) mod 16`        |

Chaque ronde applique :

```
   tmp = a + Fonction(b, c, d) + K[i] + M[g]
   a = d
   d = c
   c = b
   b = b + rotl(tmp, s[i])
```

Autrement dit : on calcule une valeur mélangée, on **fait tourner les 4
registres**, et on injecte le résultat pivoté dans `b`. Les additions sont
modulo 2³² (débordement d'`uint32_t` volontaire).

### Le feed-forward (renforcement de Davies–Meyer)

Après les 64 rondes, on **rajoute** les registres de travail à l'état :

```
   A += a ; B += b ; C += c ; D += d
```

Cette addition finale rend la compression **non inversible** : même en connaissant
l'état de sortie, on ne peut pas remonter à l'entrée. C'est ce qui fait d'une
simple permutation une vraie fonction à sens unique.

→ Code : toute cette section = `md5_transform`.

## Sérialisation → digest

Les 4 mots finaux `A, B, C, D` sont écrits **en little-endian**, bout à bout :

```
   digest = LE(A) ‖ LE(B) ‖ LE(C) ‖ LE(D)     (16 octets)
```

→ Code : `md_serialize32(ctx, digest, MD5_WORDS, MD_LE)`.

## Exemple déroulé : `MD5("abc")`

- Message : `61 62 63` (3 octets, soit 24 bits).
- Padding :
  ```
  61 62 63 80 00 00 … 00   (jusqu'à l'octet 55)
  18 00 00 00 00 00 00 00  (octets 56–63 : longueur = 24 bits = 0x18, en LE)
  ```
  → un seul bloc de 64 octets.
- Après compression + feed-forward et sérialisation LE :
  ```
  900150983cd24fb0d6963f7d28e17f72
  ```

Note le `18` (=24) placé en **premier** octet du champ longueur : c'est
l'écriture little-endian de la longueur. En SHA-256, ce sera `00…00 18`.

## Test rapide

```sh
./ft_ssl md5 -s "abc"          # MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
printf "" | ./ft_ssl md5       # (stdin)= d41d8cd98f00b204e9800998ecf8427e
```

---

Voir aussi : **[SHA-256](sha256.md)** · **[Merkle–Damgård](merkle-damgard.md)**
