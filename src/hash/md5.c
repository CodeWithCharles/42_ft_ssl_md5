/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   md5.c                                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 18:43:35 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/02 23:50:49 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hash.h"
#include "hash_const.h"

static const uint32_t	g_md5_iv[MD5_WORDS] = {
	0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476
};

/* 64 constantes K = floor(2^32 * |sin(i+1)|). */
static const uint32_t	g_md5_k[64] = {
	0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
	0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
	0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
	0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
	0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
	0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
	0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
	0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
	0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
	0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
	0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
	0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
	0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
	0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
	0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
	0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

/* Rotations gauche par ronde (4 valeurs repetees). */
static const uint32_t	g_md5_s[64] = {
	7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
	5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
	4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
	6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

static uint32_t rotl(uint32_t x, uint32_t c)
{
    return ((x << c) | (x >> (32 - c)));
}

/* Lits 4 octets en little-endian -> un mot de 32 bits. */
static uint32_t read_le32(const uint8_t *p)
{
    return ((uint32_t)p[0] | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24));
}

/* Compression d'un bloc de 64 octets dans l'etat h[0..3]. */
static void md5_transform(t_hash_ctx *ctx, const uint8_t *block)
{
	uint32_t	*h;
	uint32_t	m[16];
	uint32_t	a, b, c, d;
	uint32_t	f, g;
	int			i;

	h = (uint32_t *)ctx->state;
	i = -1;
	while (++i < 16)
		m[i] = read_le32(block + i * 4);
	a = h[0];
	b = h[1];
	c = h[2];
	d = h[3];
	i = -1;
	while (++i < MD5_ROUNDS)
	{
		if (i < 16)
		{
			f = (b & c) | (~b & d);
			g = i;
		}
		else if (i < 32)
		{
			f = (d & b) | (~d & c);
			g = (5 * i + 1) & 15;
		}
		else if (i < 48)
		{
			f = b ^ c ^ d;
			g = (3 * i + 5) & 15;
		}
		else
		{
			f = c ^ (b | ~d);
			g = (7 * i) & 15;
		}
		f = f + a + g_md5_k[i] + m[g];
		a = d;
		d = c;
		c = b;
		b = b + rotl(f, g_md5_s[i]);
	}
	h[0] += a;
	h[1] += b;
	h[2] += c;
	h[3] += d;
}

/* -------------------------- Interface t_hash_algo ------------------------- */

static void md5_init(t_hash_ctx *ctx)
{
	uint32_t	*h;
	int			i;

	h = (uint32_t *)ctx->state;
	i = -1;
	while (++i < MD5_WORDS)
		h[i] = g_md5_iv[i];
	ctx->buffer_len = 0;
	ctx->total_len = 0;
}

static void md5_update(t_hash_ctx *ctx, const uint8_t *data, size_t len)
{
    md_absorb(ctx, data, len, MD5_BLOCK, md5_transform);
}

static void md5_final(t_hash_ctx *ctx, uint8_t *digest)
{
    md_finalize(ctx, MD5_BLOCK, MD_LEN64, MD_LE, md5_transform);
    md_serialize32(ctx, digest, MD5_WORDS, MD_LE);
}

const t_hash_algo   g_md5_algo = {
    "md5", MD5_BLOCK, MD5_DIGEST, md5_init, md5_update, md5_final
};