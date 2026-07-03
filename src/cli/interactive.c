/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   interactive.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/03 12:04:11 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/03 12:34:11 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"

/*
** Parcourt un token depuis line[*i], quote-aware ('...' et "...").
** Si dst != NULL, y copie les octets (quotes retirees). Avance *i jusqu'au
** separateur ou la fin. Retourne la longueur du token.
*/
static size_t   walk_token(const char *line, size_t *i, char *dst)
{
    size_t  len;
    char    quote;

    len = 0;
    while (line[*i] && !ft_isspace(line[*i]))
    {
        if (line[*i] == '\'' || line[*i] == '"')
        {
            quote = line[(*i)++];
            while (line[*i] && line[*i] != quote)
            {
                if (dst)
                    dst[len] = line[*i];
                len++;
                (*i)++;
            }
            if (line[*i] == quote)
                (*i)++;
        }
        else
        {
            if (dst)
                dst[len] = line[*i];
            len++;
            (*i)++;
        }
    }
    return (len);
}

static char *extract_token(const char *line, size_t *i)
{
    size_t  start;
    size_t  len;
    char    *tok;

    start = *i;
    len = walk_token(line, i, NULL);
    tok = malloc(len + 1);
    if (!tok)
        return (NULL);
    *i = start;
    walk_token(line, i, tok);
    tok[len] = '\0';
    return (tok);
}

static size_t   count_tokens(const char *line)
{
    size_t  i;
    size_t  n;

    i = 0;
    n = 0;
    while (line[i])
    {
        while(ft_isspace(line[i]))
            i++;
        if (!line[i])
            break ;
        walk_token(line, &i, NULL);
        n++;
    }
    return (n);
}

static char **tokenize(const char *line)
{
    char    **tokens;
    size_t  n;
    size_t  i;
    size_t  k;

    n = count_tokens(line);
    tokens = malloc((n + 1) * sizeof(char *));
    if (!tokens)
        return (NULL);
    i = 0;
    k = 0;
    while (k < n)
    {
        while (ft_isspace(line[i]))
            i++;
        tokens[k] = extract_token(line, &i);
        if (!tokens[k])
            return (tokens[k] = NULL, ft_free_split(&tokens), NULL);
        k++;
    }
    tokens[n] = NULL;
    return (tokens);
}

static int  is_quit(const char *cmd)
{
    return (ft_strcmp(cmd, "exit") == 0 || ft_strcmp(cmd, "quit") == 0);
}

/* Execute une ligne tokenisee (argv[0] = commande). N'interrompt jamais. */
static void run_line(t_ssl *ssl, char **argv)
{
    const t_command *cmd;

    cmd = find_command(argv[0]);
    if (!cmd)
    {
        print_invalid_command(ssl->prog, argv[0]);
        return ;
    }
    ssl->command = cmd;
    ft_bzero(&ssl->options, sizeof(t_options));
    ssl->sources = NULL;
    ssl->seen_operand = 0;
    if (parse_arguments(ssl, argv + 1) == 0)
        run_sources(ssl);
    ft_lstclear(&ssl->sources, free);
}

int interactive_mode(t_ssl *ssl)
{
    char    *line;
    char    **argv;

    fd_printf(2, "ft_ssl> ");
    line = get_next_line(0);
    while (line)
    {
        argv = tokenize(line);
        free(line);
        if (argv && argv[0])
        {
            if (is_quit(argv[0]))
                return(ft_free_split(&argv), 0);
            run_line(ssl, argv);
        }
        ft_free_split(&argv);
        fd_printf(2, "ft_ssl> ");
        line = get_next_line(0);
    }
    fd_printf(2, "\n");
    return (0);
}