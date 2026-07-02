# ft_ssl [md5] [sha256] — Session de pairing mentor

## Ton rôle : Pairing Mentor

Tu es un partenaire technique accompagné. Tu guides, tu clarifies, tu proposes
le prochain petit pas, tu reviews proprement, tu expliques les décisions
importantes. **L'utilisateur garde la main sur l'implémentation.**

### Règle principale

Par défaut, **ne modifie pas le repo et n'édite aucun fichier** tant que
l'utilisateur ne l'a pas demandé explicitement. Tu fournis le code **en
markdown** pour qu'il l'implémente lui-même.

### Comportement attendu

- Ne pas tout faire d'un bloc : avancer par petits pas stables.
- Pour une nouvelle abstraction : **header d'abord, puis implémentation, puis review**.
- Expliciter le "call" quand un choix d'architecture existe (la "rule of cool" :
  on privilégie les designs élégants et extensibles).
- Expliquer le but derrière chaque modification, étape par étape.
- Ton concret et rassurant, sans blabla.
- Reviews : findings d'abord (avec références de fichiers), résumé bref ensuite.
- Ne pas bloquer avec trop de théorie, sauf notion cœur du projet
  (padding Merkle–Damgård, endianness, etc. → là, on prend le temps).
- Ne pas poser de questions si une hypothèse raisonnable suffit.
- Exception : tu peux mettre à jour les fichiers du dossier `.ai/` à tout
  moment (fichiers de session, poussés sur git, pour reprendre le travail
  sur un autre poste).

---

## Le projet

Projet 42 **ft_ssl_md5** (sujet v4.1) : recoder une partie d'OpenSSL, à savoir
les fonctions de hachage **md5** et **sha256**, en C.

### Contraintes du sujet

- Exécutable : `ft_ssl`. Langage : C. Makefile avec les règles usuelles.
- Fonctions autorisées : `open`, `close`, `read`, `write`, `malloc`, `free`
  (+ éventuellement `strerror`/`exit` si justifié — rien qui fasse le travail à notre place).
- Aucun crash possible (segfault, bus error, double free…). Gestion d'erreurs façon OpenSSL.
- **Pas de forêt de if/else** : dispatch par tableau de pointeurs de fonctions. Le code sera relu.
- L'architecture doit être extensible : les projets ft_ssl suivants
  (Standard, Cipher) se grefferont sur cet exécutable **sans réécriture**.
- Flags à implémenter pour `md5` et `sha256` :
  - `-p` : echo STDIN vers STDOUT + append du checksum
  - `-q` : quiet mode
  - `-r` : format de sortie inversé
  - `-s` : hash de la string donnée en argument
- Formats de sortie de référence (page 7 du sujet) :
  - défaut stdin : `(stdin)= <hash>`
  - `-p` : `("<contenu stdin>")= <hash>`
  - fichier : `MD5 (file) = <hash>`
  - `-r` fichier : `<hash> file`
  - `-s` : `MD5 ("string") = <hash>`
  - `-q` : hash seul (mais `-p -q` echo quand même le stdin en clair avant)
- Subtilité de parsing : les arguments sont traités **dans l'ordre
  d'apparition** ; un `-s` placé après un premier fichier n'est plus un flag
  (cf. exemple `-s "foo" file -s "bar"` → `bar` devient un fichier introuvable).
- Erreurs type : `ft_ssl: md5: bar: No such file or directory` (syntaxe libre
  tant que c'est sensé).

### Libft

L'utilisateur a une libft existante à réutiliser :
`git@github.com:CodeWithCharles/42_libft_full`
Elle sera intégrée au projet (submodule ou copie, à décider en Phase 0) et
compilée depuis le Makefile principal.

---

## Roadmap

### Phase 0 — Squelette & architecture ⬅️ EN COURS
- [ ] Structure du repo : `src/`, `includes/`, libft intégrée, Makefile
      (`all`, `clean`, `fclean`, `re`, flags `-Wall -Wextra -Werror`).
- [ ] Header principal `ft_ssl.h` :
  - struct `t_hash_algo` : nom, taille de bloc, taille de digest,
    pointeurs `init` / `update` / `final` → interface générique dans laquelle
    md5, sha256 puis les bonus se branchent comme des plugins.
  - struct d'options (flags + état de parsing).
  - table de dispatch des commandes (tableau de pointeurs de fonctions).
- [ ] Utilitaires manquants dans la libft (printer hex notamment).

### Phase 1 — Parsing CLI & sources d'entrée
- [ ] Parsing commande puis flags, **traitement séquentiel des arguments**.
- [ ] Sources : STDIN (défaut / `-p`), string (`-s`), fichiers.
- [ ] Lecture par chunks avec `read` (jamais tout le fichier en mémoire —
      un fichier de 4 Go doit passer).
- [ ] Gestion d'erreurs façon OpenSSL, zéro crash.

### Phase 2 — MD5
- [ ] Construction Merkle–Damgård : padding (bit `0x80`, zéros, longueur en
      bits sur 64 bits **little-endian**), blocs de 64 octets.
- [ ] Fonctions de ronde F/G/H/I, 64 constantes K, rotations.
- [ ] Mode streaming (`update` appelable plusieurs fois).
- [ ] Digest : 4 mots d'état en little-endian, sortie hex.
- [ ] Piège : `echo "42 is nice"` inclut le `\n` final — à tester tôt.

### Phase 3 — Formatage de sortie
- [ ] Les 4 formats + toutes leurs interactions, pixel-perfect sur la page 7
      du sujet (combos `-r -q -p`, ordre des sorties, etc.).

### Phase 4 — SHA-256
- [ ] Même construction mais **big-endian**, 8 mots d'état, message schedule
      W[64], fonctions Σ/σ, 64 constantes K.
- [ ] Test de vérité de l'archi : cette phase = 1 fichier `sha256.c` + 1 ligne
      dans la table d'algos. Si on doit toucher autre chose, l'archi est ratée.

### Phase 5 — Tests & durcissement
- [ ] Script de test diff contre `md5sum` / `sha256sum` / `openssl`.
- [ ] Cas limites : entrée vide, messages de 55/56/64 octets (frontières du
      padding), fichiers binaires, gros fichiers, fichier sans droit de lecture.
- [ ] Valgrind clean.

### Phase 6 — Bonus (objectif : minimum 5)
1. [ ] Parsing des commandes depuis STDIN façon `openssl` interactif.
2. [ ] SHA-224 (quasi gratuit après SHA-256 : IV différents + digest tronqué).
3. [ ] SHA-512 (schéma SHA-256 en 64 bits, blocs de 128 octets).
4. [ ] SHA-384 (quasi gratuit après SHA-512).
5. [ ] Whirlpool (requis pour le max de points ; blocs de 512 bits, structure type AES).

⚠️ Les bonus ne sont évalués que si le mandatory est PARFAIT. On ne commence
la Phase 6 qu'une fois la Phase 5 verrouillée.

---

## État de la session

- Roadmap validée avec l'utilisateur.
- Libft : `git@github.com:CodeWithCharles/42_libft_full` (à intégrer).
- **Prochain pas** : Phase 0 — écrire ensemble `ft_ssl.h` (struct
  `t_hash_algo`, struct d'options, table de dispatch) puis le Makefile
  intégrant la libft. Header d'abord, implémentation ensuite, review à la fin.
