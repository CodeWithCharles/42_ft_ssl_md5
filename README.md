# ft_ssl

Ré-implémentation en C d'une partie d'OpenSSL : les commandes de hachage
`md5` et `sha256` (+ bonus `sha224`, `sha512`, `sha384` et un mode interactif).

Ce README explique **le flux du programme et l'architecture**, dans l'ordre où
on construit le projet. Pour la **crypto** (Merkle–Damgård, padding, endianness,
détail de chaque algo), voir [`docs/`](docs/README.md).

---

## Démarrer

```sh
make            # compile ft_ssl (libft en submodule, cf. plus bas)
make test       # lance la suite de tests (diff contre md5sum/sha256sum/...)
make re         # fclean + all

./ft_ssl md5 -s "hello world"       # MD5 ("hello world") = 5eb63bbb...
./ft_ssl sha256 fichier             # SHA256 (fichier) = ...
echo -n abc | ./ft_ssl md5          # (stdin)= 900150983cd24fb0...
./ft_ssl                            # mode interactif (lit les commandes sur stdin)
```

> La libft est un **submodule**. Après `git clone`, fais `git clone --recursive`
> (ou `git submodule update --init`) — le Makefile le rattrape aussi tout seul.

---

## L'idée directrice : un système de plugins

Toute l'architecture tient dans une phrase : **ajouter un algorithme de hachage
doit coûter 1 fichier + 1 ligne**, sans toucher au parsing, à l'exécuteur ou à la
sortie. Pour ça, chaque algo est un **plugin** décrit par une struct générique
`t_hash_algo` (nom, tailles, pointeurs `init`/`update`/`final`) branchée dans une
**table de dispatch**. Le reste du programme ne connaît que cette interface,
jamais un algo en particulier — donc **zéro `if (algo == MD5)`** nulle part.

---

## Le flux, de `argv` au digest

```
argv ──► [dispatch]      argv[1] = commande → find_command() → t_hash_algo
     │
     ├─► [parsing CLI]   flags + opérandes → liste ordonnée de t_source
     │                   (parse.c : moteur générique piloté par une table de flags)
     │
     └─► [exécuteur]     pour chaque source, en ordre :
             open/read par chunks ──► algo.init → algo.update* → algo.final
                                              │
                                              └─► [sortie] format sujet (output.c)
```

### 1. Dispatch de la commande — `src/cli/dispatch.c`
`main` lit `argv[1]` et le cherche dans la table `g_commands` (`find_command`).
Chaque entrée associe un **nom** → un **algo** (`t_hash_algo`) → une **table de
flags**. Commande inconnue → `print_invalid_command` (liste les commandes, façon
openssl) et exit 1.

### 2. Parsing CLI — `src/cli/parse.c` + `src/cli/flags.c`
Les arguments sont traités **dans l'ordre d'apparition**. Le moteur
`parse_arguments` est **générique** : pour chaque flag, il fait un *lookup* dans
la table de flags de la commande (`g_hash_flags`) et appelle le *handler*
correspondant. Ajouter un flag = **une ligne** dans la table.

Le piège du sujet est géré par un booléen `seen_operand` : **dès qu'on a croisé
le premier fichier, on arrête de reconnaître les flags**. C'est ce qui fait que
dans `-s foo file -s bar`, le deuxième `-s` devient un *fichier* introuvable.

Chaque entrée produit une `t_source` (`SRC_STDIN` / `SRC_STRING` / `SRC_FILE`)
poussée dans une **liste ordonnée** (`t_list`) — pas de hachage à ce stade, on ne
fait que décrire *quoi* hacher et dans *quel ordre*.

### 3. Exécuteur — `src/exec/exec.c`
`run_sources` parcourt la liste et, selon le *kind*, ouvre la bonne entrée
(string en mémoire / `fd` fichier / `fd` 0). La lecture se fait **par chunks de
64 Ko** (`hash_fd`) : on `read`, on `update`, on jette → **un fichier de plusieurs
Go passe sans jamais être chargé en entier**. Erreur d'ouverture → message façon
OpenSSL sur stderr, on continue les sources suivantes, exit final à 1.

### 4. Le cœur de hachage — `src/hash/`
C'est là que vit le pattern **`init` → `update` (N fois) → `final`** (mode
streaming). La construction **Merkle–Damgård**, commune à toute la famille SHA/MD,
est factorisée dans `src/hash/md_core.c` :

| Fonction         | Rôle                                                     |
|------------------|----------------------------------------------------------|
| `md_absorb`      | bufferise les octets, compresse dès qu'un bloc est plein |
| `md_finalize`    | **padding** (bit `0x80`, zéros, longueur) + flush final  |
| `md_serialize32` | écrit le digest (mots 32 bits) en little/big-endian      |
| `md_serialize64` | idem pour la famille 64 bits (SHA-512/384)               |

Chaque algo n'apporte donc que **ce qui lui est propre** : sa fonction de
compression (`md5_transform`, `sha256_transform`, …), ses constantes et son IV.
Les dimensions (tailles de bloc/digest/mots) sont centralisées dans
[`includes/hash_const.h`](includes/hash_const.h).

> Détail crypto important : **MD5 est little-endian, SHA-256 big-endian**. Voir
> [`docs/merkle-damgard.md`](docs/merkle-damgard.md).

### 5. Formatage de la sortie — `src/exec/output.c`
`print_digest` applique le format du sujet en fonction du *kind* de source et des
options :

| Cas              | Sortie                          |
|------------------|---------------------------------|
| stdin (défaut)   | `(stdin)= <hash>`               |
| fichier          | `MD5 (file) = <hash>`           |
| `-s`             | `MD5 ("string") = <hash>`       |
| `-r`             | ordre inversé : `<hash> file`   |
| `-q`             | hash seul                       |
| `-p`             | `("<contenu stdin>")= <hash>`   |

Le tag (`MD5`, `SHA256`, …) est le nom de commande en majuscules. Le mode `-p`
fait un **echo streaming** de stdin (`print_p_open`/`print_p_close`) tout en
hachant le flux — sans bufferiser.

### 6. Mode interactif — `src/cli/interactive.c`
Lancé sans commande (`./ft_ssl`), le programme lit des **commandes ligne par
ligne** sur stdin, façon `openssl`. Chaque ligne est découpée par un **tokenizer
quote-aware** (`'...'` / `"..."`, collage `-s"a b"`) en un `argv` synthétique,
puis **réinjectée dans le même pipeline** (`find_command` → `parse_arguments` →
`run_sources`) — donc aucune logique dupliquée. `exit`/`quit` ou EOF terminent.

---

## Carte du dépôt

```
includes/
  ft_ssl.h        Types CLI (t_ssl, t_source, t_command, table de flags) + protos
  hash.h          Interface plugin (t_hash_algo, t_hash_ctx) + API md_core
  hash_const.h    Dimensions de chaque algo (blocs, digests, rondes…)
src/
  main.c          Point d'entrée : dispatch → parse → exec (ou mode interactif)
  cli/
    dispatch.c    Table des commandes g_commands + find_command
    parse.c       Moteur de parsing séquentiel (seen_operand) + liste de sources
    flags.c       Table g_hash_flags + handlers (-p -q -r -s)
    interactive.c REPL stdin + tokenizer quote-aware
  exec/
    exec.c        run_sources : ouverture + lecture par chunks + streaming
    output.c      Formats de sortie du sujet
  hash/
    md_core.c     Squelette Merkle–Damgård partagé (absorb/finalize/serialize)
    md5.c         Compression MD5 (little-endian)
    sha256.c      Compression SHA-256 / SHA-224 (big-endian)
    sha512.c      Moteur 64 bits SHA-512 / SHA-384 (bonus)
docs/             Documentation crypto détaillée (à lire pour comprendre les algos)
tests/            run_tests.sh (make test)
libft/            submodule : utilitaires (ft_printf, get_next_line, listes…)
```

---

## Ajouter un algorithme (le test de l'archi)

C'est la promesse de départ, concrètement :

1. Créer `src/hash/<algo>.c` : sa fonction de compression + `init`/`update`/`final`
   (en s'appuyant sur `md_core`), et exporter un `const t_hash_algo g_<algo>_algo`.
2. Ajouter ses dimensions dans `includes/hash_const.h`.
3. Ajouter **une ligne** dans `g_commands` (`src/cli/dispatch.c`) + un `extern`.

Parsing, exécuteur, sortie, Makefile (sources auto-découvertes) : **rien à
toucher**. C'est exactement comme ça que SHA-256 puis les bonus ont été ajoutés.

---

## Contraintes du sujet respectées

- Fonctions autorisées uniquement : `open` `close` `read` `write` `malloc` `free`
  (+ `strerror` pour les messages d'erreur).
- **Zéro crash** (segfault / double free / …), **zéro fuite** (valgrind clean).
- Gestion d'erreurs façon OpenSSL, codes retour alignés sur `md5sum`.
- Dispatch par **tables de pointeurs de fonctions**, pas de forêt de `if/else`.
- Lecture en streaming : un fichier de plusieurs Go passe sans exploser la RAM.

## Documentation crypto

- [`docs/merkle-damgard.md`](docs/merkle-damgard.md) — la théorie commune (à lire en premier)
- [`docs/md5.md`](docs/md5.md) — MD5
- [`docs/sha256.md`](docs/sha256.md) — SHA-256 & SHA-224
- [`docs/sha512.md`](docs/sha512.md) — SHA-512 & SHA-384 (bonus)
