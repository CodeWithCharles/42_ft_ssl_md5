/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 18:02:58 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/03 00:59:11 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include    "ft_ssl.h"

int main(int argc, char **argv)
{
    t_ssl   ssl;
    int     status;
    
    ft_bzero(&ssl, sizeof(t_ssl));
    ssl.prog = "ft_ssl";
    if (argc < 2)
        return (ft_putendl_fd("usage: ft_ssl command [flags] [args...]", 2), 1);
    
    ssl.command = find_command(argv[1]);
    if (!ssl.command)
        return (print_digest(ssl.prog, argv[1]), 1);
	if (parse_arguments(&ssl, argv + 2))
		return (ft_lstclear(&ssl.sources, free), 1);
	status = run_sources(&ssl);
	ft_lstclear(&ssl.sources, free);
	return (status);
}