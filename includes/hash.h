/* -------------------------------------------------------------------------- */
/*                                   hash.h                                   */
/* -------------------------------------------------------------------------- */

#ifndef HASH_H
# define HASH_H

# include <stddef.h>
# include <stdint.h>

/*
** Tailles max : dimensionnent les buffers du contexte générique.
**   MD5    : état 16o (4x u32), bloc 64o
**   SHA256 : état 32o (8x u32), bloc 64o
**   Marge prise pour SHA-512 (bonus) : état 64o, bloc 128o.
*/

# define    FT_SSL_MAX_STATE    64
# define    FT_SSL_MAX_BLOCK    128
# define    FT_SSL_MAX_DIGEST   64

typedef struct  s_hash_ctx
{
    uint8_t state[FT_SSL_MAX_STATE];    /* mots d'etat, format natif algo */
    uint8_t buffer[FT_SSL_MAX_BLOCK];   /* bloc partiel en attente */
    size_t  buffer_len;                 /* octets presents dans buffer */
    uint64_t    total_len;              /* total absorbe, en octets */
} t_hash_ctx;

/* Un algo = un plugin. Les tailles voyagent avec lui. */
typedef struct s_hash_algo
{
    const char  *name;
    size_t      block_size;
    size_t      digest_size;
    void        (*init)(t_hash_ctx *ctx);
    void        (*update)(t_hash_ctx *ctx, const uint8_t *data, size_t len);
    void        (*final)(t_hash_ctx *ctx, uint8_t *digest);
} t_hash_algo;

#endif