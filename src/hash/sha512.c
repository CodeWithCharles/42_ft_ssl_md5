/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   sha512.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 01:36:22 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/03 01:46:13 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hash.h"
#include "hash_const.h"

# define    ROTR(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
# define    CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
# define    MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
# define    BSIG0(x) (ROTR(x, 28) ^ ROTR(x, 34) ^ ROTR(x, 39))
# define    BSIG1(x) (ROTR(x, 14) ^ ROTR(x, 18) ^ ROTR(x, 41))
# define    SSIG0(x) (ROTR(x, 1) ^ ROTR(x, 8) ^ ((x) >> 7))
# define    SSIG1(x) (ROTR(x, 19) ^ ROTR(x, 61) ^ ((x) >> 6))

/* IV = 64 premiers bits des racines carrées des 8 premiers nombres premiers. */
static const uint64_t   g_sha512_iv[SHA512_WORDS] = {
    0x6a09e667f3bcc908, 0xbb67ae8584caa73b,
    0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
    0x510e527fade682d1, 0x9b05688c2b3e6c1f,
    0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
};

/* K = 64 premiers bits des racines cubiques des 80 premiers nombres premiers. */
static const uint64_t   g_sha512_k[SHA512_ROUNDS] = {
    0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
    0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
    0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
    0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
    0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
    0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
    0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
    0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
    0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
    0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
    0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
    0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
    0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
    0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
    0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
    0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
    0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
    0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
    0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
    0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

static uint64_t read_be64(const uint8_t *p)
{
    return (((uint64_t)p[0] << 56) | ((uint64_t)p[1] << 48)
        | ((uint64_t)p[2] << 40) | ((uint64_t)p[3] << 32)
        | ((uint64_t)p[4] << 24) | ((uint64_t)p[5] << 16)
        | ((uint64_t)p[6] << 8) | (uint64_t)p[7]);
}

/* Prépare les 80 mots du message schedule W. */
static void sha512_schedule(uint64_t *w, const uint8_t *block)
{
    int i;

    i = -1;
    while (++i < 16)
        w[i] = read_be64(block + i * 8);
    i = 15;
    while (++i < SHA512_ROUNDS)
        w[i] = SSIG1(w[i - 2]) + w[i - 7] + SSIG0(w[i - 15]) + w[i - 16];
}

/* v[] = a, b, c, d, e, f, g, h */
static void sha512_transform(t_hash_ctx *ctx, const uint8_t *block)
{
    uint64_t    *h;
    uint64_t    w[SHA512_ROUNDS];
    uint64_t    v[SHA512_WORDS];
    uint64_t    t1;
    int         i;

    h = (uint64_t *)ctx->state;
    sha512_schedule(w, block);
    i = -1;
    while (++i < SHA512_WORDS)
        v[i] = h[i];
    i = -1;
    while (++i < SHA512_ROUNDS)
    {
        t1 = v[7] + BSIG1(v[4]) + CH(v[4], v[5], v[6]) + g_sha512_k[i] + w[i];
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
    while (++i < SHA512_WORDS)
        h[i] += v[i];
}

static void sha512_init(t_hash_ctx *ctx)
{
    uint64_t    *h;
    int         i;

    h = (uint64_t *)ctx->state;
    i = -1;
    while (++i < SHA512_WORDS)
        h[i] = g_sha512_iv[i];
    ctx->buffer_len = 0;
    ctx->total_len = 0;
}

static void sha512_update(t_hash_ctx *ctx, const uint8_t *data, size_t len)
{
    md_absorb(ctx, data, len, SHA512_BLOCK, sha512_transform);
}

static void sha512_final(t_hash_ctx *ctx, uint8_t *digest)
{
    md_finalize(ctx, SHA512_BLOCK, MD_LEN128, MD_BE, sha512_transform);
    md_serialize64(ctx, digest, SHA512_WORDS, MD_BE);
}

const t_hash_algo   g_sha512_algo = {
    "sha512", SHA512_BLOCK, SHA512_DIGEST, sha512_init, sha512_update,
    sha512_final
};

/* ------------------------------- SHA-384 ---------------------------------- */
/* SHA-512 tronque : meme moteur, IV differents, digest sur 6 mots (384 bits). */

static const uint64_t   g_sha384_iv[SHA512_WORDS] = {
    0xcbbb9d5dc1059ed8, 0x629a292a367cd507,
    0x9159015a3070dd17, 0x152fecd8f70e5939,
    0x67332667ffc00b31, 0x8eb44a8768581511,
    0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4
};

static void sha384_init(t_hash_ctx *ctx)
{
    uint64_t    *h;
    int         i;

    h = (uint64_t *)ctx->state;
    i = -1;
    while (++i < SHA512_WORDS)
        h[i] = g_sha384_iv[i];
    ctx->buffer_len = 0;
    ctx->total_len = 0;
}

static void sha384_final(t_hash_ctx *ctx, uint8_t *digest)
{
    md_finalize(ctx, SHA512_BLOCK, MD_LEN128, MD_BE, sha512_transform);
    md_serialize64(ctx, digest, SHA384_WORDS, MD_BE);
}

const t_hash_algo   g_sha384_algo = {
    "sha384", SHA512_BLOCK, SHA384_DIGEST, sha384_init, sha512_update,
    sha384_final
};