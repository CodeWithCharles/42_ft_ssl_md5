# Whirlpool (bonus, l'exception AES)

> Prérequis : lire **[Merkle–Damgård](merkle-damgard.md)** d'abord. Whirlpool
> partage la **coquille** de Merkle–Damgård (découpage en blocs + padding avec
> longueur), mais **tout l'intérieur est différent** de la famille MD/SHA. C'est
> le seul algo du projet qui ne repose pas sur un schéma type Davies–Meyer.

## Fiche technique

| Propriété           | Valeur                                            |
|---------------------|---------------------------------------------------|
| Taille du digest    | 512 bits = **64 octets**                          |
| État interne        | 512 bits vus comme une **matrice 8×8 d'octets**   |
| Taille de bloc      | 512 bits = **64 octets**                          |
| Champ longueur      | **256 bits (32 octets)**, big-endian              |
| Construction        | **Miyaguchi–Preneel** autour d'un chiffre par bloc |
| Nombre de rondes    | **10** (par bloc)                                 |
| Corps fini          | GF(2⁸) modulo `x⁸+x⁴+x³+x²+1` = **`0x11D`**        |

## Ce qui change par rapport à MD5/SHA

MD5 et SHA construisent leur compression sur un schéma proche de **Davies–Meyer**
(`état += f(état, bloc)`, feed-forward additif). Whirlpool utilise **Miyaguchi–
Preneel**, qui enroule un **vrai chiffrement par bloc** noté `W` :

```
H_i = W_{H_{i-1}}(m_i)  XOR  m_i  XOR  H_{i-1}
```

L'état précédent `H_{i-1}` joue **deux rôles** : il est la **clé** du chiffrement
du bloc `m_i`, et l'un des termes du XOR final. L'IV est `H_0 = 0` (512 bits nuls),
et le digest est l'état final `H_t` (les 64 octets, dans l'ordre de la matrice).

**Mais la coquille reste Merkle–Damgård** : on découpe le message en blocs de
64 octets, et le padding est le même schéma (bit `0x80`, zéros, longueur) — avec
un **champ de longueur de 256 bits big-endian** au lieu de 64. C'est pour ça que
le cœur partagé `md_core` (`md_absorb` / `md_finalize`) fonctionne tel quel :
`md_finalize(..., len_bytes = 32, MD_BE, wp_transform)`.

## Le chiffre par bloc W (un cousin d'AES)

L'état de 512 bits est une **matrice 8×8 d'octets** (remplie ligne par ligne :
l'octet `k` du bloc va en `(k/8, k%8)`). Le chiffrement est un *whitening* suivi
de **10 rondes**, chaque ronde étant la composition de 4 opérations :

```
RF[K] = AK[K] ∘ MR ∘ SC ∘ SB          (appliquée de droite à gauche)
W(K) sur un bloc P :  état = P ⊕ K₀ , puis état = RF[Kᵣ](état) pour r = 1..10
```

- **SB — SubBytes (γ)** : chaque octet passe dans une **S-box** de 256 entrées.
- **SC — ShiftColumns (π)** : la colonne `j` est décalée cycliquement vers le bas
  de `j` positions → `b[i][j] = a[(i−j) mod 8][j]`.
- **MR — MixRows (θ)** : chaque ligne est multipliée par une matrice **MDS 8×8
  circulante** de première ligne `(1, 1, 4, 1, 8, 5, 2, 9)`, dans GF(2⁸).
- **AK — AddKey (σ)** : XOR avec la clé de ronde.

Dans le code, **SB et SC sont fusionnés** : `sc[i][j] = S[état[(i−j) mod 8][j]]`.

### Le key schedule (le point qui piège)

Les clés de ronde sont dérivées de la clé `K = H_{i-1}` par **la même fonction de
ronde**, en *lockstep* avec le chiffrement des données :

```
K₀ = K
Kᵣ = RF[RCᵣ](Kᵣ₋₁)          ← la clé avance avec les constantes de ronde RCᵣ
```

où la **constante de ronde `RCᵣ`** est une matrice dont seule la première ligne
est non nulle : `RCᵣ[0][j] = S[8(r−1) + j]` (les octets de la S-box, 8 par ronde).
À chaque ronde on calcule d'abord `Kᵣ` (avec `RCᵣ`), puis on l'utilise comme clé
pour la piste données. Différence unique entre les deux pistes : ce qu'on XOR au
`σ` — `RCᵣ` pour la clé, `Kᵣ` pour les données.

## La S-box, générée (pas hardcodée)

Plutôt que de coller 256 octets (source d'erreurs), la S-box est **générée** à
partir de trois mini-boxes 4 bits du standard :

```
E   = [1,B,9,C,D,6,F,3,E,8,7,4,A,2,5,0]
E⁻¹ = inverse de E
R   = [7,C,B,D,E,4,9,F,6,3,8,A,2,5,1,0]

pour x de 0 à 255 :
    u = E[x>>4] ;  v = E⁻¹[x&0xF] ;  t = R[u^v]
    S[x] = (E[u^t] << 4) | E⁻¹[v^t]
```

Vérification : `S[0] = 0x18`, `S[1] = 0x23` (conformes à la table du standard).

## Vecteurs de référence (NESSIE)

| Entrée             | Whirlpool                                     |
|--------------------|-----------------------------------------------|
| `""`               | `19fa61d75522a466…b138cc42a66eb3` (64 o)      |
| `"abc"`            | `4e2448a4c6f486bb…225292076d4eef5` (64 o)     |
| `"message digest"` | `378c84a4126e2dc6…62e86dbd37a8903e` (64 o)    |

## Où c'est dans le code

| Concept                              | Fichier / fonction                        |
|--------------------------------------|-------------------------------------------|
| Génération de la S-box               | `src/hash/whirlpool.c` → `wp_build_sbox`  |
| Multiplication GF(2⁸)                | `src/hash/whirlpool.c` → `gf_mul`         |
| Une ronde (SB·SC·MR·AK)              | `src/hash/whirlpool.c` → `wp_round`       |
| Chiffre W (whitening + 10 rondes)    | `src/hash/whirlpool.c` → `wp_cipher`      |
| Miyaguchi–Preneel                    | `src/hash/whirlpool.c` → `wp_transform`   |
| Padding (256 bits) + absorption      | `src/hash/md_core.c` (réutilisé tel quel) |

> Le fait que Whirlpool se branche via **1 fichier + 1 ligne** dans `g_commands`,
> sans toucher `md_core`, le parsing, l'exécuteur ou la sortie, est la validation
> ultime de l'architecture en plugins du projet.
