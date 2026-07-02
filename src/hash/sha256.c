/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sha256.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 00:12:40 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/03 01:18:03 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hash.h"
#include "hash_const.h"

# define    ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
# define    CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
# define    MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
# define    BSIG0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
# define    BSIG1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
# define    SSIG0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ ((x) >> 3))
# define    SSIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

/* IV = 32 premiers bits des racines carrées des 8 premiers nombres premiers */
static const uint32_t   g_sha256_iv[SHA256_WORDS] = {
	0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
	0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

/* K = 32 premiers bits des racines cubiques des 64 premiers nombres premiers. */
static const uint32_t	g_sha256_k[SHA256_ROUNDS] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static uint32_t read_be32(const uint8_t *p)
{
    return (((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)
        | ((uint32_t)p[2] << 8) | (uint32_t)p[3]);
}

/* Prépare les 64 mots du message schedule W. */
static void sha256_schedule(uint32_t *w, const uint8_t *block)
{
    int i;

    i = -1;
    while (++i < 16)
        w[i] = read_be32(block + i * 4);
    i = 15;
    while (++i < SHA256_ROUNDS)
        w[i] = SSIG1(w[i - 2]) + w[i - 7] + SSIG0(w[i - 15]) + w[i - 16];
}

/* v[] = a, b, c, d, e, f, g, h */
static void sha256_transform(t_hash_ctx *ctx, const uint8_t *block)
{
    uint32_t    *h;
    uint32_t    w[SHA256_ROUNDS];
    uint32_t    v[SHA256_WORDS];
    uint32_t    t1;
    int         i;

    h = (uint32_t *)ctx->state;
    sha256_schedule(w, block);
    i = -1;
    while (++i < SHA256_WORDS)
        v[i] = h[i];
    i = -1;
    while (++i < SHA256_ROUNDS)
    {
        t1 = v[7] + BSIG1(v[4]) + CH(v[4], v[5], v[6]) + g_sha256_k[i] + w[i];
        v[7] = v[6];
        v[6] = v[5];
        v[5] = v[4];
        v[4] = v[3] + t1;
        v[3] = v[2];
        v[2] = v[1];
        v[1] = v[0];
        v[0] = t1 + BSIG0(v[1]) + MAJ(v[1], v[2], v[3]);
    }
    i = -1;
    while (++i < SHA256_WORDS)
        h[i] += v[i];
}

static void sha256_init(t_hash_ctx *ctx)
{
    uint32_t    *h;
    int         i;

    h = (uint32_t *)ctx->state;
    i = -1;
    while (++i < SHA256_WORDS)
        h[i] = g_sha256_iv[i];
    ctx->buffer_len = 0;
    ctx->total_len = 0;
}

static void sha256_update(t_hash_ctx *ctx, const uint8_t *data, size_t len)
{
    md_absorb(ctx, data, len, SHA256_BLOCK, sha256_transform);
}

static void sha256_final(t_hash_ctx *ctx, uint8_t *digest)
{
    md_finalize(ctx, SHA256_BLOCK, MD_LEN64, MD_BE, sha256_transform);
    md_serialize32(ctx, digest, SHA256_WORDS, MD_BE);
}

const t_hash_algo   g_sha256_algo = {
    "sha256", SHA256_BLOCK, SHA256_DIGEST, sha256_init, sha256_update, sha256_final
};

/* ------------------------------- SHA-224 ---------------------------------- */
/* Meme moteur que SHA-256 (transform / schedule / update). Seuls changent
   l'IV et la troncature du digest a 7 mots (224 bits). */

static const uint32_t   g_sha224_iv[SHA256_WORDS] = {
    0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,
    0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4
};

static void sha224_init(t_hash_ctx *ctx)
{
    uint32_t    *h;
    int         i;

    h = (uint32_t *)ctx->state;
    i = -1;
    while (++i < SHA256_WORDS)
        h[i] = g_sha224_iv[i];
    ctx->buffer_len = 0;
    ctx->total_len = 0;
}

static void sha224_final(t_hash_ctx *ctx, uint8_t *digest)
{
    md_finalize(ctx, SHA256_BLOCK, MD_LEN64, MD_BE, sha256_transform);
    md_serialize32(ctx, digest, SHA224_WORDS, MD_BE);
}

const t_hash_algo   g_sha224_algo = {
    "sha224", SHA256_BLOCK, SHA224_DIGEST, sha224_init, sha256_update,
    sha224_final
};