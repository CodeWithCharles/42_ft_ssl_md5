/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 18:16:34 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/02 18:28:18 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"

int add_source(t_ssl *ssl, t_source_kind kind, const char *value)
{
    t_source    *src;
    t_list      *node;

    src = malloc(sizeof(t_source));
    if (!src)
        return (1);
    src->kind = kind;
    src->value = value;
    node = ft_lstnew(src);
    if (!node)
    {
        free(src);
        return (1);
    }
    ft_lstadd_back(&ssl->sources, node);
    return (0);
}

static int  flag_error(t_ssl *ssl, const char *reason, char c)
{
    ft_putstr_fd((char *)ssl->prog, 2);
    ft_putstr_fd(": ", 2);
    ft_putstr_fd((char *)ssl->command->name, 2);
    ft_putstr_fd(": ", 2);
    ft_putstr_fd((char *)reason, 2);
    ft_putstr_fd(" -- ", 2);
    ft_putchar_fd(c, 2);
    ft_putchar_fd('\n', 2);
    return (1);
}

static const t_flag_spec    *find_flag(const t_flag_spec *flags, char c)
{
    int i;

    i = 0;
    while (flags[i].name)
    {
        if (flags[i].name == c)
            return (&flags[i]);
        ++i;
    }
    return (NULL);
}

/* Retour : -1 erreur, 0 aucun argv extra consomme, 1 un argv extra consomme. */
static int  apply_flags(t_ssl *ssl, const char *token, char *next)
{
    const t_flag_spec *spec;

    while (*token)
    {
        spec = find_flag(ssl->command->flags, *token);
        if (!spec)
            return (flag_error(ssl, "unknown flag", *token) * -1);
        if (spec->takes_value)
        {
            if (*(token + 1))
                return (spec->handler(ssl, token + 1) ? -1 : 0);
            if (!next)
                return (flag_error(ssl, "requires an argument", *token) * -1);
            return (spec->handler(ssl, next) ? -1 : 1);
        }
        if (spec->handler(ssl, NULL))
            return (-1);
        token++;
    }
    return (0);
}

static int is_flag(const char *arg)
{
    return (arg[0] == '-' && arg[1] != '\0');
}

int parse_arguments(t_ssl *ssl, char **args)
{
    int i;
    int consumed;

    i = 0;
    while (args[i])
    {
        if (!ssl->seen_operand && is_flag(args[i]))
        {
            consumed = apply_flags(ssl, args[i] + 1, args[i + 1]);
            if (consumed < 0)
                return (1);
            i += consumed;
        }
        else
        {
            if (add_source(ssl, SRC_FILE, args[i]))
                return (1);
            ssl->seen_operand = 1;
        }
        ++i;
    }
    if (!ssl->sources)
        return (add_source(ssl, SRC_STDIN, NULL));
    return (0);
}