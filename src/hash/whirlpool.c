/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   whirlpool.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 13:55:15 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/03 16:49:40 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"
#include "hash_const.h"

/* S-box generee a partir des mini-boxes du standard (voir docs/whirlpool.md). */
static uint8_t    g_wp_sbox[256];
/* Ligne de la matrice circulante MDS (theta / MixRows). */
static const uint8_t	g_wp_mds[8] = {1, 1, 4, 1, 8, 5, 2, 9};

static void	wp_build_sbox(void)
{
	static const int	e[16] = {0x1, 0xB, 0x9, 0xC, 0xD, 0x6, 0xF, 0x3,
	0xE, 0x8, 0x7, 0x4, 0xA, 0x2, 0x5, 0x0};
	static const int	r[16] = {0x7, 0xC, 0xB, 0xD, 0xE, 0x4, 0x9, 0xF,
	0x6, 0x3, 0x8, 0xA, 0x2, 0x5, 0x1, 0x0};
	static int			done;
	int					ei[16];
	int					x;
	int					u;
	int					v;
	int					t;

	if (done)
		return ;
	x = -1;
	while (++x < 16)
		ei[e[x]] = x;
	x = -1;
	while (++x < 256)
	{
		u = e[x >> 4];
		v = ei[x & 0xF];
		t = r[u ^ v];
		g_wp_sbox[x] = (uint8_t)((e[u ^ t] << 4) | ei[v ^ t]);
	}
	done = 1;
}

/* Multiplication dans GF(2^8) modulo 0x11d. */
static uint8_t  gf_mul(uint8_t a, uint8_t b)
{
	uint8_t	p;
	uint8_t	hi;
	int		i;

	p = 0;
	i = -1;
	while (++i < 8)
	{
		if (b & 1)
			p ^= a;
		hi = a & 0x80;
		a <<= 1;
		if (hi)
			a ^= 0x1D;
		b >>= 1;
	}
	return (p);
}

/* Une ronde sur la matrice 8x8 : gamma -> pi -> theta -> sigma(rk). */
static void wp_round(uint8_t st[8][8], const uint8_t rk[8][8])
{
	uint8_t	sc[8][8];
	uint8_t	acc;
	int		i;
	int		j;
	int		c;

	i = -1;
	while (++i < 8)
	{
		j = -1;
		while (++j < 8)
			sc[i][j] = g_wp_sbox[st[(i - j + 8) & 7][j]];
	}
	i = -1;
	while (++i < 8)
	{
		j = -1;
		while (++j < 8)
		{
			acc = 0;
			c = -1;
			while (++c < 8)
				acc ^= gf_mul(sc[i][c], g_wp_mds[(j - c + 8) & 7]);
			st[i][j] = acc ^ rk[i][j];
		}
	}
}

/* Chiffre "W" : whitening + 10 rondes, key schedule en lockstep. */
static void	wp_cipher(const uint8_t key[8][8], const uint8_t in[8][8], uint8_t out[8][8])
{
	uint8_t	k[8][8];
	uint8_t	rc[8][8];
	int		r;
	int		i;
	int		j;

	ft_memcpy(k, key, 64);
	i = -1;
	while (++i < 8)
	{
		j = -1;
		while (++j < 8)
			out[i][j] = in[i][j] ^ k[i][j];
	}
	r = 0;
	while (++r <= WHIRLPOOL_ROUNDS)
	{
		ft_bzero(rc, 64);
		j = -1;
		while (++j < 8)
			rc[0][j] = g_wp_sbox[8 * (r - 1) + j];
		wp_round(k, rc);
		wp_round(out, k);
	}
}

/* Miyaguchi-Preneel : H = W_H(m) ^ m ^ H. Lit/ecrit ctx->state (64 o). */
static void	wp_transform(t_hash_ctx *ctx, const uint8_t *block)
{
	uint8_t	h[8][8];
	uint8_t	m[8][8];
	uint8_t	enc[8][8];
	int		i;
	int		j;

	ft_memcpy(h, ctx->state, 64);
	ft_memcpy(m, block, 64);
	wp_cipher(h, m, enc);
	i = -1;
	while (++i < 8)
	{
		j = -1;
		while (++j < 8)
			ctx->state[i * 8 + j] = enc[i][j] ^ m[i][j] ^ h[i][j];
	}
}

/* Interface plugin (branchee sur md_core) */
static void	whirlpool_init(t_hash_ctx *ctx)
{
	wp_build_sbox();
	ft_bzero(ctx->state, WHIRLPOOL_DIGEST);
	ctx->buffer_len = 0;
	ctx->total_len = 0;
}


static void	whirlpool_update(t_hash_ctx *ctx, const uint8_t *data, size_t len)
{
	md_absorb(ctx, data, len, WHIRLPOOL_BLOCK, wp_transform);
}
static void	whirlpool_final(t_hash_ctx *ctx, uint8_t *digest)
{
	md_finalize(ctx, WHIRLPOOL_BLOCK, WHIRLPOOL_LEN, MD_BE, wp_transform);
	ft_memcpy(digest, ctx->state, WHIRLPOOL_DIGEST);
}

const t_hash_algo	g_whirlpool_algo = {
	"whirlpool", WHIRLPOOL_BLOCK, WHIRLPOOL_DIGEST,
	whirlpool_init, whirlpool_update, whirlpool_final
};