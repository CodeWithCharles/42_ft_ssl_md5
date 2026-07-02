# ft_ssl — état de session

_Dernière mise à jour : 2026-07-02_

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
  - Sortie **brute** `hexdigest\n` (via `ft_printf("%02x")`) — formats du sujet PAS encore faits.
  - Erreurs façon OpenSSL via `fd_printf(2, ...)`, on continue les sources suivantes, exit 1.

## Tests passés
7/7 vecteurs vs md5sum (vide, abc, 55/56/64 octets, stdin, gros fichier), valgrind clean.

## Prochain pas — Phase 3 : formatage de sortie
Implémenter les 4 formats page 7 du sujet + combos `-r -q -p`, au pixel :
- défaut stdin : `(stdin)= <hash>`
- `-p` : `("<contenu stdin>")= <hash>`
- fichier : `MD5 (file) = <hash>`
- `-r` fichier : `<hash> file`
- `-s` : `MD5 ("string") = <hash>`
- `-q` : hash seul (mais `-p -q` echo quand même le stdin en clair avant)

Ça ne doit toucher QUE la couche sortie (`print_hex`/`run_sources`), pas le hashing.

## Notes libft (CodeWithCharles/42_libft_full)
- Archive : `libftfull.a` ; headers dans `libft/include/` (`libft.h`, `ft_printf.h` non inclus par libft.h).
- Dispos utiles : `ft_printf`, `fd_printf(fd, ...)`, `%02x` (zero-pad OK), `t_list` + `ft_lst*`, `ft_strcmp`, `ft_putstr_fd`/`ft_putendl_fd`.

## Pas de norminette sur ce projet.
