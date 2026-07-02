# ft_ssl — état de session

_Dernière mise à jour : 2026-07-02 (Phase 3 verrouillée)_

## Avancement roadmap

- [x] **Phase 0 — Squelette & archi**
  - `includes/hash.h` : interface plugin (`t_hash_ctx`, `t_hash_algo`, tailles max via `state[]` brut).
  - `includes/ft_ssl.h` : `t_options`, `t_source`(enum kind), `t_ssl` (contexte), table de flags (`t_flag_spec` + handler), `t_command` (porte algo + table de flags).
  - Makefile : libft en submodule (`libftfull.a`), sources auto via `find` (wildcard récursive), objets dans `build/`.
- [x] **Phase 1 — Parsing CLI & sources**
  - Moteur générique (`parse_arguments`) + config par commande (table de flags) → ajouter un flag = 1 ligne.
  - Règle du piège : `seen_operand` → après le 1er fichier, plus de flags. Vérifié sur `-s foo file -s bar`.
  - `-s` gère valeur collée (`-sfoo`) ET séparée (`-s foo`).
  - Sources en `t_list` ordonnée ; STDIN par défaut ou via `-p` (en tête).
  - `src/cli/{dispatch,parse,flags}.c`.
- [x] **Phase 2 — MD5**
  - `src/hash/md5.c` : Merkle–Damgård, padding (0x80 / zéros jusqu'à 56 mod 64 / longueur 64 bits **little-endian**), 64 rondes, feed-forward.
  - Little-endian partout (lecture mots, longueur, digest). N'exploite jamais l'endianness machine.
  - Streaming (`update` multi-appels) OK.
- [x] **Exécuteur** (`src/exec/exec.c`)
  - `run_sources` → dispatch par kind (string / fichier / stdin), `read` par chunks 64 Ko.
  - Erreurs façon OpenSSL via `fd_printf(2, ...)`, on continue les sources suivantes, exit 1.
- [x] **Phase 3 — Formatage de sortie** (`src/exec/output.c` + refacto `exec.c`)
  - `print_digest` (kind + options) : normal / `-r` / `-q`. Table de formats en 1 point.
  - Asymétries du sujet respectées : `(stdin)= ` (pas d'espace avant `=`) vs
    `MD5 (x) = ` (espace avant `=`). Tag = `ft_toupper` du nom de commande.
  - Priorités combos confirmées vs sujet : `-q` écrase `-r` (hash seul) ;
    `-p` ignore `-r` (toujours `("...")= H`) ; `-r -s` garde les guillemets.
  - `-p` : echo **streaming** avec strip du `\n` final (struct `t_echo`,
    hold-back 1 octet) → `("42 is nice")= H` sur 1 ligne, mais le hash porte
    sur le flux complet (`\n` inclus). Jamais de buffering.
  - `ft_printf`/`fd_printf` non bufferisés (write char par char) → l'ordre
    `ft_printf` / `write(1,...)` de l'echo est garanti.

## Tests passés
- Phase 2 : 7/7 vecteurs vs md5sum (vide, abc, 55/56/64 octets, stdin, gros fichier).
- Phase 3 : les 4 formats + tous les combos page 7 (`-p`, `-q -r`, `-r -p -s file -s`,
  `-r -q -p -s file`, `MD5 (file)=`/reverse, `-s` apostrophe → `a3c990a1…`), au pixel.
  Cas limites OK : stdin vide, `-p` sans newline final.
- Valgrind clean (exit 0) sur combo multi-sources `-p -s foo file bar`.

## Prochain pas — Phase 4 : SHA-256
Test de vérité de l'archi : **1 fichier `src/hash/sha256.c` + 1 ligne dans la
table d'algos** (`g_sha256_algo`), et brancher la commande `sha256` dans le
dispatch. Si on doit toucher `exec.c`/`output.c`/parsing → l'archi est ratée.
- Merkle–Damgård **big-endian** (longueur 64 bits BE, lecture des mots BE).
- 8 mots d'état, message schedule `W[64]`, fonctions Σ/σ, 64 constantes K.
- Digest 32 octets. `digest_size` porté par la struct → `output.c` s'adapte seul.
- Vérif : `./ft_ssl sha256 -s "42 is nice"` → `b7e44c7a…13b758f` (sujet p.8).

## Notes libft (CodeWithCharles/42_libft_full)
- Archive : `libftfull.a` ; headers dans `libft/include/` (`libft.h`, `ft_printf.h` non inclus par libft.h).
- Dispos utiles : `ft_printf`, `fd_printf(fd, ...)`, `%02x` (zero-pad OK), `t_list` + `ft_lst*`, `ft_strcmp`, `ft_putstr_fd`/`ft_putendl_fd`.

## Pas de norminette sur ce projet.
