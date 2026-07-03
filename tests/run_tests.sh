#!/bin/bash
# ============================================================================ #
#  Suite de tests ft_ssl — mandatory (md5 + sha256)                            #
#  Compare ft_ssl aux outils de reference (md5sum / sha256sum) et aux          #
#  formats exacts du sujet. Verifie aussi les codes retour facon md5sum.       #
#                                                                              #
#  Usage : ./tests/run_tests.sh    (ou : make test)                           #
#  Sortie : liste des tests + total. Code retour != 0 si un test echoue.       #
# ============================================================================ #

cd "$(dirname "$0")/.." || exit 1
BIN=./ft_ssl
[ -x "$BIN" ] || { echo "Binaire $BIN introuvable — lance 'make' d'abord."; exit 1; }
command -v md5sum >/dev/null && command -v sha256sum >/dev/null \
	|| { echo "md5sum/sha256sum requis (coreutils)."; exit 1; }

pass=0
fail=0
if [ -t 1 ]; then G=$'\033[32m'; R=$'\033[31m'; B=$'\033[1m'; Z=$'\033[0m'; else G=; R=; B=; Z=; fi

eq () { # $1 label  $2 attendu  $3 obtenu
	if [ "$2" = "$3" ]; then
		pass=$((pass + 1))
		printf "  ${G}OK${Z}   %s\n" "$1"
	else
		fail=$((fail + 1))
		printf "  ${R}FAIL %s${Z}\n     attendu: [%s]\n     obtenu : [%s]\n" "$1" "$2" "$3"
	fi
}

rep () { printf 'a%.0s' $(seq 1 "$1"); }   # $1 fois le caractere 'a'

# ------------------------------- Correction -------------------------------- #
# Frontieres de padding Merkle-Damgard : 55/56 (bascule d'un bloc a deux),
# 64 (bloc plein), + tailles variees et gros buffer.

printf "${B}== MD5 vs md5sum ==${Z}\n"
for n in 0 1 3 55 56 63 64 65 128 1000; do
	s=$(rep "$n")
	eq "md5 len=$n" \
		"$(printf '%s' "$s" | md5sum | cut -d' ' -f1)" \
		"$(printf '%s' "$s" | $BIN md5 -q)"
done

printf "${B}== SHA-256 vs sha256sum ==${Z}\n"
for n in 0 1 3 55 56 63 64 65 128 1000; do
	s=$(rep "$n")
	eq "sha256 len=$n" \
		"$(printf '%s' "$s" | sha256sum | cut -d' ' -f1)" \
		"$(printf '%s' "$s" | $BIN sha256 -q)"
done

printf "${B}== SHA-224 vs sha224sum (bonus) ==${Z}\n"
for n in 0 1 3 55 56 63 64 65 128 1000; do
	s=$(rep "$n")
	eq "sha224 len=$n" \
		"$(printf '%s' "$s" | sha224sum | cut -d' ' -f1)" \
		"$(printf '%s' "$s" | $BIN sha224 -q)"
done

printf "${B}== SHA-512 vs sha512sum (bonus) ==${Z}\n"
# Frontieres du bloc 128 o : 111 tient, 112 bascule sur un bloc de plus, 128 plein.
for n in 0 1 3 110 111 112 127 128 129 1000; do
	s=$(rep "$n")
	eq "sha512 len=$n" \
		"$(printf '%s' "$s" | sha512sum | cut -d' ' -f1)" \
		"$(printf '%s' "$s" | $BIN sha512 -q)"
done

printf "${B}== SHA-384 vs sha384sum (bonus) ==${Z}\n"
for n in 0 1 3 111 112 128 1000; do
	s=$(rep "$n")
	eq "sha384 len=$n" \
		"$(printf '%s' "$s" | sha384sum | cut -d' ' -f1)" \
		"$(printf '%s' "$s" | $BIN sha384 -q)"
done

printf "${B}== Whirlpool (bonus, vecteurs officiels NESSIE) ==${Z}\n"
# Pas de whirlpoolsum dans coreutils : on compare aux vecteurs de reference.
eq "whirlpool empty" \
	"19fa61d75522a4669b44e39c1d2e1726c530232130d407f89afee0964997f7a73e83be698b288febcf88e3e03c4f0757ea8964e59b63d93708b138cc42a66eb3" \
	"$(printf '' | $BIN whirlpool -q)"
eq "whirlpool abc" \
	"4e2448a4c6f486bb16b6562c73b4020bf3043e3a731bce721ae1b303d97e6d4c7181eebdb6c57e277d0e34957114cbd6c797fc9d95d8b582d225292076d4eef5" \
	"$(printf 'abc' | $BIN whirlpool -q)"
eq "whirlpool message digest" \
	"378c84a4126e2dc6e56dcc7458377aac838d00032230f53ce1f5700c0ffb4d3b8421557659ef55c106b4b52ac5a4aaa692ed920052838f3362e86dbd37a8903e" \
	"$(printf 'message digest' | $BIN whirlpool -q)"
# Self-consistency -s / file / stdin sur les frontieres de padding (bloc 64 o,
# champ longueur 32 o : bascule autour de 31/32).
for n in 31 32 63 64 65 1000; do
	s=$(rep "$n")
	printf '%s' "$s" > .tmp_wp
	ref="$(printf '%s' "$s" | $BIN whirlpool -q)"
	eq "whirlpool len=$n : file==stdin" "$ref" "$($BIN whirlpool -q .tmp_wp)"
	eq "whirlpool len=$n : -s==stdin"   "$ref" "$($BIN whirlpool -q -s "$s")"
done
rm -f .tmp_wp

# Gros fichier binaire aleatoire
head -c 5000000 /dev/urandom > .tmp_big
eq "md5 gros fichier bin" \
	"$(md5sum .tmp_big | cut -d' ' -f1)" "$($BIN md5 -q .tmp_big)"
eq "sha256 gros fichier bin" \
	"$(sha256sum .tmp_big | cut -d' ' -f1)" "$($BIN sha256 -q .tmp_big)"
eq "sha512 gros fichier bin" \
	"$(sha512sum .tmp_big | cut -d' ' -f1)" "$($BIN sha512 -q .tmp_big)"
rm -f .tmp_big

# --------------------------- Formats du sujet ------------------------------ #
printf "${B}== Formats de sortie (sujet p.7-8) ==${Z}\n"
printf 'And above all,\n' > .tmp_file
eq "md5 stdin" \
	"(stdin)= 35f1d6de0302e2086a4e472266efb3a9" \
	"$(echo '42 is nice' | $BIN md5)"
eq "md5 -p" \
	'("42 is nice")= 35f1d6de0302e2086a4e472266efb3a9' \
	"$(echo '42 is nice' | $BIN md5 -p)"
eq "md5 file" \
	"MD5 (.tmp_file) = 53d53ea94217b259c11a5a2d104ec58a" \
	"$($BIN md5 .tmp_file)"
eq "md5 -r file" \
	"53d53ea94217b259c11a5a2d104ec58a .tmp_file" \
	"$($BIN md5 -r .tmp_file)"
eq "md5 -s" \
	'MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72' \
	"$($BIN md5 -s abc)"
eq "md5 -q -r stdin" \
	"e20c3b973f63482a778f3fd1869b7f25" \
	"$(echo 'Pity the living.' | $BIN md5 -q -r)"
eq "sha256 -s (sujet)" \
	'SHA256 ("42 is nice") = b7e44c7a40c5f80139f0a50f3650fb2bd8d00b0d24667c4c2ca32c88e13b758f' \
	"$($BIN sha256 -s '42 is nice')"
rm -f .tmp_file

# ------------------------------ Codes retour ------------------------------- #
# Convention md5sum : 0 si tout va bien, 1 si au moins une source echoue.
printf "${B}== Codes retour (facon md5sum) ==${Z}\n"
printf x > .tmp_ok
$BIN md5 .tmp_ok >/dev/null 2>&1;          eq "rc succes"            0 $?
$BIN md5 /does/not/exist >/dev/null 2>&1;  eq "rc fichier manquant"  1 $?
$BIN md5 .tmp_ok /nope >/dev/null 2>&1;    eq "rc mixte OK+erreur"   1 $?
$BIN badcmd >/dev/null 2>&1;               eq "rc commande invalide" 1 $?
$BIN </dev/null >/dev/null 2>&1;           eq "rc aucun arg (EOF)"   0 $?
rm -f .tmp_ok

# --------------------------- Mode interactif (bonus) ----------------------- #
# Pas de commande -> lecture des commandes sur STDIN facon openssl.
# stderr (prompt "ft_ssl> " + erreurs) masque : on compare le stdout au one-shot.
printf "${B}== Mode interactif (bonus) ==${Z}\n"
eq "interactif == one-shot" \
	"$($BIN md5 -s abc)" \
	"$(printf 'md5 -s abc\n' | $BIN 2>/dev/null)"
eq "interactif quote-aware" \
	"$($BIN md5 -s 'hello world')" \
	"$(printf 'md5 -s \"hello world\"\n' | $BIN 2>/dev/null)"
eq "interactif exit stoppe" \
	"$($BIN md5 -s a)" \
	"$(printf 'md5 -s a\nexit\nmd5 -s NEVER\n' | $BIN 2>/dev/null)"
eq "interactif cmd inconnue skip" \
	"$($BIN md5 -s ok)" \
	"$(printf 'nope\nmd5 -s ok\n' | $BIN 2>/dev/null)"
eq "interactif reset options entre lignes" \
	"$($BIN sha256 -s a)" \
	"$(printf 'md5 -q -s x\nsha256 -s a\n' | $BIN 2>/dev/null | tail -n1)"

# --------------------------------- Bilan ----------------------------------- #
printf "\n${B}Total :${Z} ${G}%d OK${Z}, ${R}%d FAIL${Z}\n" "$pass" "$fail"
[ "$fail" -eq 0 ]
