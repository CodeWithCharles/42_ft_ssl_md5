/* -------------------------------------------------------------------------- */
/*                                  ft_ssl.h                                  */
/* -------------------------------------------------------------------------- */

#ifndef FT_SSL_H
# define FT_SSL_H

# include "libft.h"
# include "hash.h"

/* -------------------------------------------------------------------------- */
/*                                 CLI Options                                */
/* -------------------------------------------------------------------------- */

typedef struct  s_options
{
    int echo_stdin;     /* -p */
    int quiet;          /* -q */
    int reverse;        /* -r */
}   t_options;

/* -------------------------------------------------------------------------- */
/*                       Input sources (order preserved)                      */
/* -------------------------------------------------------------------------- */

typedef enum e_source_kind
{
    SRC_STDIN,      /* defaut, ou -p */
    SRC_STRING,     /* -s "..." */
    SRC_FILE,       /* file */
}   t_source_kind;

typedef struct  s_source
{
    t_source_kind   kind;
    const char      *value; /* STRING = contenu ; FILE = chemin ; STDIN = NULL */
}   t_source;

typedef struct  s_ssl   t_ssl;

/* -------------------------------------------------------------------------- */
/*                                 Flags table                                */
/* -------------------------------------------------------------------------- */

typedef int (*t_flag_handler)(t_ssl *ssl, const char *value);

typedef struct  s_flag_spec
{
    char            name;           /* 'p', 'q', 'r', 's' */
    int             takes_value;    /* 1 si le flag consomme l'arg suivant */
    t_flag_handler  handler;
}   t_flag_spec;

/* -------------------------------------------------------------------------- */
/*        Commande (md5, sha256) : porte son algo ET sa table de flags        */
/* -------------------------------------------------------------------------- */

typedef struct  s_command
{
    const char          *name;
    const t_hash_algo   *algo;
    const t_flag_spec   *flags; /* NULL terminated */
}   t_command;


/* -------------------------------------------------------------------------- */
/*                               Contexte global                              */
/* -------------------------------------------------------------------------- */

struct  s_ssl
{
    const char      *prog;          /* "ft_ssl", pour les messages */
    const t_command *command;       /* commande resolue */
    t_options       options;        /* -p -q -r */
    t_list          *sources;       /* liste ordonnee de (t_source *) */
    int             seen_operand;   /* switch flags -> fichiers */
};

/* -------------------------------------------------------------------------- */
/*                                 Prototypes                                 */
/* -------------------------------------------------------------------------- */

/* dispatch.c */
const t_command *find_command(const char *name);

/* parse.c */
int             parse_arguments(t_ssl *ssl, char **argv);

/* sources (parse.c) */
int             add_source(t_ssl *ssl, t_source_kind kind, const char *value);

extern const    t_flag_spec g_hash_flags[];

/* flags.c */
int             flag_echo(t_ssl *ssl, const char *value);
int             flag_quiet(t_ssl *ssl, const char *value);
int             flag_reverse(t_ssl *ssl, const char *value);
int             flag_string(t_ssl *ssl, const char *value);

#endif