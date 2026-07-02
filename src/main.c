/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 18:02:58 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/02 18:26:44 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include    "ft_ssl.h"

/* TODO: Remplacer par l'executeur reel */
static void print_recap(t_ssl *ssl)
{
    t_list      *n;
    t_source    *s;
    const char  *kinds[] = {"STDIN", "STRING", "FILE"};

    ft_printf("command: %s | p=%d q=%d r=%d\n", ssl->command->name,
    ssl->options.echo_stdin, ssl->options.quiet, ssl->options.reverse);
    n = ssl->sources;

    while (n)
    {
        s = (t_source *)n->content;
        ft_printf(" [%s] %s\n", kinds[s->kind], s->value ? s->value : "(stdin)");
        n = n->next;
    }
}

int main(int argc, char **argv)
{
    t_ssl   ssl;
    
    ft_bzero(&ssl, sizeof(t_ssl));
    ssl.prog = "ft_ssl";
    if (argc < 2)
        return (ft_putendl_fd("usage: ft_ssl command [flags] [args...]", 2), 1);
    
    ssl.command = find_command(argv[1]);
    if (!ssl.command)
    {
        ft_putstr_fd("ft_ssl: Error: '", 2);
        ft_putstr_fd(argv[1], 2);
        ft_putendl_fd("' is an invalid command.", 2);
        return (1);
    }
    if (parse_arguments(&ssl, argv + 2))
        return (ft_lstclear(&ssl.sources, free), 1);
    print_recap(&ssl);
    ft_lstclear(&ssl.sources, free);
    return (0);
}