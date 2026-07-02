/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   flags.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 18:12:46 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/02 18:30:53 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"

int flag_echo(t_ssl *ssl, const char *value)
{
    (void)value;
    ssl->options.echo_stdin = 1;
    return (add_source(ssl, SRC_STDIN, NULL));
}

int flag_quiet(t_ssl *ssl, const char *value)
{
    (void)value;
    ssl->options.quiet = 1;
    return (0);
}

int flag_reverse(t_ssl *ssl, const char *value)
{
    (void)value;
    ssl->options.reverse = 1;
    return (0);
}

int flag_string(t_ssl *ssl, const char *value)
{
    return (add_source(ssl, SRC_STRING, value));
}

const t_flag_spec   g_hash_flags[] = {
    {'p', 0, flag_echo},
    {'q', 0, flag_quiet},
    {'r', 0, flag_reverse},
    {'s', 1, flag_string},
    {0, 0, NULL},
};