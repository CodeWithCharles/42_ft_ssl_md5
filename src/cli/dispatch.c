/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dispatch.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 18:10:04 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/03 01:18:37 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"

/* TODO: Branche en Phase 2 avec md5 / Phase 4 pour sha256 */
const   t_command g_commands[] = {
    {"md5", &g_md5_algo, g_hash_flags},
    {"sha256", &g_sha256_algo, g_hash_flags},
    {"sha224", &g_sha224_algo, g_hash_flags},
    {NULL, NULL, NULL},
};

const t_command *find_command(const char *name)
{
    int i;

    i = 0;
    while (g_commands[i].name)
    {
        if (ft_strcmp(g_commands[i].name, name) == 0)
            return (&g_commands[i]);
        ++i;
    }
    return (NULL);
}

void    print_invalid_command(const char *prog, const char *name)
{
    int i;

    fd_printf(2, "%s: Error: '%s' is an invalid command.\n", prog, name);
    ft_putendl_fd("Commands:", 2);
    i = 0;
    while (g_commands[i].name)
        ft_putendl_fd((char *)g_commands[i++].name, 2);
    ft_putstr_fd("Flags:\n", 2);
    i = 0;
    while (g_hash_flags[i].name)
    {
        if (i > 0)
            ft_putchar_fd(' ', 2);
        ft_putchar_fd('-', 2);
        ft_putchar_fd(g_hash_flags[i].name, 2);
        i++;
    }
    ft_putchar_fd('\n', 2);
}