/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   hash_const.h                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 23:44:22 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/03 13:55:05 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HASH_CONST_H
# define HASH_CONST_H

/* Taille du champ de longueur du padding Merkle-Damgard (en octets). */
# define    MD_LEN64    		8	/* longueur sur 64 bits	: md5, sha256 */
# define    MD_LEN128   		16	/* longueur sur 128 bits : sha512 (bonus) */

/* ----------------------------------- MD5 ---------------------------------- */

# define	MD5_BLOCK			64	/* Taille de bloc (octets) */
# define	MD5_DIGEST			16	/* Taille du digest (octets) */
# define	MD5_WORDS			4	/* mots d'état de 32 bits */
# define	MD5_ROUNDS			64	/* rondes de compression */

/* --------------------------------- SHA-256 -------------------------------- */

# define	SHA256_BLOCK		64
# define	SHA256_DIGEST		32
# define	SHA256_WORDS		8
# define	SHA256_ROUNDS		64

/* --------------------------------- SHA-224 -------------------------------- */
/* Meme moteur que SHA-256 : IV differents + digest tronque a 7 mots (224 bits).
   Reutilise SHA256_BLOCK / SHA256_WORDS / SHA256_ROUNDS pour l'etat interne. */

# define	SHA224_DIGEST		28
# define	SHA224_WORDS		7

/* --------------------------------- SHA-512 -------------------------------- */
/* Moteur 64 bits : mots de 64 bits, bloc 128 o, 80 rondes, longueur sur 128 bits. */

# define	SHA512_BLOCK		128
# define	SHA512_DIGEST		64
# define	SHA512_WORDS		8
# define	SHA512_ROUNDS		80

/* --------------------------------- SHA-384 -------------------------------- */
/* SHA-512 tronque : meme moteur, IV differents, digest sur 6 mots (384 bits). */

# define	SHA384_DIGEST		48
# define	SHA384_WORDS		6

/* -------------------------------- Whirlpool ------------------------------- */

# define	WHIRLPOOL_BLOCK		64
# define	WHIRLPOOL_DIGEST	64
# define	WHIRLPOOL_ROUNDS	10
# define	WHIRLPOOL_LEN		32

#endif