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

# Gros fichier binaire aleatoire
head -c 5000000 /dev/urandom > .tmp_big
eq "md5 gros fichier bin" \
	"$(md5sum .tmp_big | cut -d' ' -f1)" "$($BIN md5 -q .tmp_big)"
eq "sha256 gros fichier bin" \
	"$(sha256sum .tmp_big | cut -d' ' -f1)" "$($BIN sha256 -q .tmp_big)"
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
$BIN >/dev/null 2>&1;                       eq "rc aucun argument"    1 $?
rm -f .tmp_ok

# --------------------------------- Bilan ----------------------------------- #
printf "\n${B}Total :${Z} ${G}%d OK${Z}, ${R}%d FAIL${Z}\n" "$pass" "$fail"
[ "$fail" -eq 0 ]
