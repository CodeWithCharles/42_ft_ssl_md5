# ft_ssl — Documentation des algorithmes de hachage

Cette documentation explique **de A à Z** le fonctionnement des deux fonctions
de hachage du projet, MD5 et SHA-256, et comment elles sont implémentées ici.

## Plan

1. **[Fonctions de hachage & Merkle–Damgård](merkle-damgard.md)** — la théorie
   commune : ce qu'est une fonction de hachage cryptographique, la construction
   Merkle–Damgård, le **padding**, l'**endianness**. À lire en premier : MD5 et
   SHA-256 partagent 90 % de leur squelette.
2. **[MD5](md5.md)** — digest 128 bits, little-endian, 64 rondes en 4 groupes.
3. **[SHA-256](sha256.md)** — digest 256 bits, big-endian, message schedule et
   fonctions Σ/σ. Inclut aussi **SHA-224** (bonus), même moteur tronqué.
4. **[SHA-512 & SHA-384](sha512.md)** (bonus) — le moteur **64 bits** : mots de
   64 bits, bloc 128 o, 80 rondes. SHA-384 = SHA-512 tronqué.
5. **[Whirlpool](whirlpool.md)** (bonus) — l'exception : **Miyaguchi–Preneel**
   autour d'un chiffre type AES (matrice 8×8, GF(2⁸)). Même coquille
   Merkle–Damgård à l'extérieur, tout l'intérieur différent.

## Vue d'ensemble : une fonction de hachage, c'est quoi ?

Une **fonction de hachage cryptographique** prend un message de **taille
quelconque** et produit une empreinte (« digest ») de **taille fixe** :

```
   message (n octets)  ─────►  H  ─────►  digest (16 o pour MD5, 32 o pour SHA-256)
```

Propriétés visées :

- **Déterministe** : même message → même digest.
- **Rapide** à calculer.
- **Résistante à la préimage** : impossible de retrouver le message depuis le digest.
- **Effet avalanche** : changer 1 bit du message change ~50 % des bits du digest.
- **Résistante aux collisions** : difficile de trouver deux messages de même digest.

> ⚠️ MD5 est aujourd'hui **cassé** (collisions calculables en secondes) ; SHA-256
> reste sûr. On les recode ici pour comprendre les mécaniques, pas pour sécuriser.

## Comment c'est câblé dans le code

L'architecture est un **système de plugins** : chaque algo est une instance de
`t_hash_algo` (nom, tailles, `init`/`update`/`final`) branchée dans une table de
dispatch. Le squelette Merkle–Damgård commun vit dans un seul module.

| Concept                              | Fichier / fonction                              |
|--------------------------------------|-------------------------------------------------|
| Interface générique d'un algo        | `includes/hash.h` → `t_hash_algo`, `t_hash_ctx` |
| Dimensions (tailles de bloc/digest…) | `includes/hash_const.h`                         |
| Absorption des données (streaming)   | `src/hash/md_core.c` → `md_absorb`              |
| **Padding** + longueur + flush final | `src/hash/md_core.c` → `md_finalize`            |
| Sérialisation du digest (LE/BE)      | `src/hash/md_core.c` → `md_serialize32`         |
| Compression MD5                      | `src/hash/md5.c` → `md5_transform`              |
| Compression SHA-256                  | `src/hash/sha256.c` → `sha256_transform`        |
| Table des commandes                  | `src/cli/dispatch.c` → `g_commands`             |

Le pattern `init` → `update` (appelable N fois, mode streaming) → `final` permet
de hacher un fichier de plusieurs Go **sans jamais le charger entièrement en
mémoire** : on lit par blocs, on absorbe, on jette.

## Vecteurs de test de référence

| Entrée   | MD5                                | SHA-256                                                            |
|----------|------------------------------------|-------------------------------------------------------------------|
| `""`     | `d41d8cd98f00b204e9800998ecf8427e` | `e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855` |
| `"abc"`  | `900150983cd24fb0d6963f7d28e17f72` | `ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad` |

(vérifiables avec `md5sum` / `sha256sum` ; attention, `echo` ajoute un `\n`.)
