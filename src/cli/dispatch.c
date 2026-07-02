/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dispatch.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 18:10:04 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/03 00:27:03 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"

/* TODO: Branche en Phase 2 avec md5 / Phase 4 pour sha256 */
const   t_command g_commands[] = {
    {"md5", &g_md5_algo, g_hash_flags},
    {"sha256", &g_sha256_algo, g_hash_flags},
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