/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   md_core.c                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 23:21:12 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/02 23:21:12 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "hash.h"

/* Bufferise les octets, déclenche transform à chaque bloc plein. */
void    md_absorb(t_hash_ctx *ctx, const uint8_t *data, size_t len,
            size_t block_size, t_md_transform transform)
{
    size_t  i;

    ctx->total_len += len;
    i = 0;
    while (i < len)
    {
        ctx->buffer[ctx->buffer_len++] = data[i++];
        if (ctx->buffer_len == block_size)
        {
            transform(ctx, ctx->buffer);
            ctx->buffer_len = 0;
        }
    }
}

/* Ecrit les 64 bits de longueur dans le champ (len_bytes octets),
    LE (md5) ou BE (sha). Le reste du champ (sha512) est déjà à zéro. */
static void write_length(t_hash_ctx *ctx, size_t block_size, size_t len_bytes, t_md_endian endian)
{
    uint64_t    bits;
    size_t      pad_to;
    size_t      i;

    bits = ctx->total_len * 8;
    pad_to = block_size - len_bytes;
    i = -1;
    while (++i < 8)
    {
        if (endian == MD_LE)
            ctx->buffer[pad_to + i] = (uint8_t)(bits >> (8 * i));
        else
            ctx->buffer[block_size - 1 - i] = (uint8_t)(bits >> (8 * i));
    }
}

/* Padding standard : 0x80, zéros, longueur en bits, flush final */
void    md_finalize(t_hash_ctx *ctx, size_t block_size, size_t len_bytes, t_md_endian endian, t_md_transform transform)
{
    size_t  pad_to;

    pad_to = block_size - len_bytes;
    ctx->buffer[ctx->buffer_len++] = 0x80;
    if (ctx->buffer_len > pad_to)
    {
        while (ctx->buffer_len < block_size)
            ctx->buffer[ctx->buffer_len++] = 0x00;
        transform(ctx, ctx->buffer);
        ctx->buffer_len = 0;
    }
    while (ctx->buffer_len < pad_to)
        ctx->buffer[ctx->buffer_len++] = 0x00;
    write_length(ctx, block_size, len_bytes, endian);
    transform(ctx, ctx->buffer);
}

/* Sérialise nwords mots de 32 bits de l'état vers out (LE=md5, BE=sha256). */
void    md_serialize32(const t_hash_ctx *ctx, uint8_t *out, size_t nwords, t_md_endian endian)
{
    const uint32_t  *h;
    size_t          i;
    size_t          j;

    h = (const uint32_t *)ctx->state;
    i = -1;
    while (++i < nwords)
    {
        j = -1;
        while (++j < 4)
        {
            if (endian == MD_LE)
                out[i * 4 + j] = (uint8_t)(h[i] >> (8 * j));
            else
                out[i * 4 + j] = (uint8_t)(h[i] >> (8 * (3 - j)));
        }
    }
}