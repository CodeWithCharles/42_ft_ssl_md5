# La construction Merkle–Damgård

MD5 et SHA-256 sont tous les deux bâtis sur le même moule : la **construction
Merkle–Damgård**. La comprendre une fois, c'est comprendre les deux algos (et
aussi SHA-1, SHA-512, etc.).

## L'idée

On ne sait pas mélanger un message de taille arbitraire d'un coup. En revanche,
on sait construire une **fonction de compression** qui prend :

```
   état (taille fixe)  +  un bloc du message (taille fixe)  ─►  nouvel état
```

Merkle–Damgård enchaîne cette compression bloc après bloc, en partant d'un état
initial constant (**IV**), et le dernier état devient le digest :

```
   IV ─►[compress]─►[compress]─►[compress]─►…─►[compress]─► état final ─► digest
          ▲            ▲            ▲                ▲
        bloc 1       bloc 2       bloc 3          bloc n
```

Chaque bloc dépend de **tout** ce qui précède : c'est le « chaînage ». Changer un
seul octet du message change l'état passé à toutes les compressions suivantes →
effet avalanche.

## Les 4 étapes

### 1. Initialiser l'état (IV)

Un ensemble de mots de 32 bits, constants et spécifiés par l'algo (4 mots pour
MD5, 8 pour SHA-256). Ce ne sont pas des nombres au hasard : ils viennent de
constantes mathématiques (racines de nombres premiers) pour n'avoir « rien dans
la manche » (*nothing-up-my-sleeve numbers*).

→ Code : `md5_init` / `sha256_init` (chargent `g_*_iv`).

### 2. Le padding (le point crucial ⚠️)

Le message doit être complété pour que sa longueur soit un **multiple exact de la
taille de bloc** (64 octets ici). Le padding se fait en 3 temps :

```
   [ message ][ 0x80 ][ 0x00 0x00 … 0x00 ][ longueur du message en bits ]
               │        │                   │
               │        │                   └─ 8 octets (64 bits)
               │        └─ autant de zéros que nécessaire
               └─ un seul octet 0x80  (= le bit '1' suivi de sept '0')
```

Règles précises :

1. **Toujours** ajouter un octet `0x80` (un bit à 1). *Toujours*, même si le
   message est déjà un multiple de 64 → sinon deux messages différents pourraient
   avoir le même padding.
2. Ajouter des `0x00` jusqu'à ce qu'il reste exactement **8 octets** pour la fin
   du bloc, c'est-à-dire jusqu'à la position `56 mod 64`.
3. Écrire la **longueur du message original en bits**, sur 64 bits.

Cas limite : s'il ne reste pas la place pour les 8 octets de longueur après le
`0x80` (message de 56 à 63 octets dans le dernier bloc), on **complète le bloc de
zéros, on le compresse**, puis on ouvre un nouveau bloc rempli de zéros + la
longueur. C'est pour ça que les frontières **55 / 56 / 64 octets** sont des cas
de test obligatoires.

**Pourquoi encoder la longueur ?** C'est le *length strengthening* (renforcement
Merkle–Damgård). Sans lui, on pourrait fabriquer des collisions triviales. Avec,
deux messages de longueurs différentes ne peuvent pas produire le même dernier
bloc.

→ Code : `md_finalize` dans `src/hash/md_core.c`. La ligne
`pad_to = block_size - len_bytes` généralise le « 56 » à n'importe quel algo
(SHA-512 : bloc 128, champ longueur 16 → `pad_to = 112`).

### 3. Compresser chaque bloc

On découpe le message padé en blocs de 64 octets et on appelle la compression sur
chacun. C'est ici que les deux algos **diffèrent réellement** (voir [MD5](md5.md)
et [SHA-256](sha256.md)).

En pratique, on ne padde pas tout le message d'un coup : on **absorbe** les
données au fil de l'eau (`update` peut être appelé plusieurs fois), en gardant un
tampon partiel ; dès qu'il atteint 64 octets, on compresse et on le vide. Le
padding n'intervient qu'à la toute fin, dans `final`.

→ Code : `md_absorb` (bufferisation + déclenchement de la compression).

### 4. Sérialiser l'état → digest

L'état final (4 ou 8 mots de 32 bits) est écrit en octets pour produire le
digest. **C'est là que l'endianness compte.**

→ Code : `md_serialize32`.

## Endianness : LE vs BE (le piège n°1)

L'**endianness** est l'ordre dans lequel on lit/écrit les octets d'un mot de
32 bits. Prenons le mot `0x11223344` :

```
   Little-endian (MD5)  :  44 33 22 11   (octet de poids faible en premier)
   Big-endian   (SHA-256):  11 22 33 44   (octet de poids fort en premier)
```

Trois endroits sont concernés, et **il faut la même endianness aux trois** :

| Étape                       | MD5           | SHA-256    |
|-----------------------------|---------------|------------|
| Lire un mot du bloc message | little-endian | big-endian |
| Écrire la longueur finale   | little-endian | big-endian |
| Sérialiser l'état → digest  | little-endian | big-endian |

> **Règle d'or du code** : on ne s'appuie **jamais** sur l'endianness de la
> machine. On lit et écrit les octets **explicitement** (décalages `>> 8`, `>> 16`…),
> donc le binaire donne le même résultat sur une machine LE ou BE. Voir
> `read_le32` / `read_be32` et `md_serialize32`.

→ Dans le code, un simple paramètre `t_md_endian` (`MD_LE` / `MD_BE`) porté par
chaque algo suffit à basculer tout le comportement. C'est ce qui permet à un seul
`md_core` de servir les deux.

## Récapitulatif

```
   init()   ── charge l'IV
     │
   update() ── absorbe des octets, compresse chaque bloc de 64 o plein   (× N)
     │
   final()  ── padding (0x80 + zéros + longueur), compresse le dernier bloc,
              puis sérialise l'état en digest
```

Tout le reste (les IV, la fonction de compression, l'endianness) n'est que du
**paramétrage** de ce squelette. C'est précisément ce que capture l'interface
`t_hash_algo`.

---

Suite : **[MD5](md5.md)** · **[SHA-256](sha256.md)**
