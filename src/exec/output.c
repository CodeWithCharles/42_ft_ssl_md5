/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   output.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 21:20:09 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/02 21:20:09 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"

/* "md5" -> "MD5" sur stdout (tag des formats fichier / -s). */
static void put_tag(const char *name)
{
	while (*name)
		ft_printf("%c", ft_toupper((unsigned char)*name++));
}

static void	put_hex(const uint8_t *digest, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
		ft_printf("%02x", digest[i++]);
}

/* -r : HASH puis le "sujet". Guillemets conserves pour -s (cf. `acbd... "foo"`). */
static void	print_reverse(t_source *src, const uint8_t *digest, size_t n)
{
	put_hex(digest, n);
	if (src->kind == SRC_STDIN)
		ft_printf(" (stdin)");
	else if (src->kind == SRC_FILE)
		ft_printf(" %s", src->value);
	else
		ft_printf(" \"%s\"", src->value);
	ft_printf("\n");
}

/* Attention a l'asymetrie du sujet : "(stdin)= " sans espace avant '=', "MD5 (x) = " avec espace avec '='. */
static void	print_normal(t_ssl *ssl, t_source *src, const uint8_t *digest, size_t n)
{
	if (src->kind == SRC_STDIN)
		ft_printf("(stdin)= ");
	else
	{
		put_tag(ssl->command->name);
		if (src->kind == SRC_FILE)
			ft_printf(" (%s) = ", src->value);
		else
			ft_printf(" (\"%s\") = ", src->value);
	}
	put_hex(digest, n);
	ft_printf("\n");
}

void	print_digest(t_ssl *ssl, t_source *src, const uint8_t *digest)
{
	size_t	n;

	n = ssl->command->algo->digest_size;
	if (ssl->options.quiet)
	{
		put_hex(digest, n);
		ft_printf("\n");
	}
	else if (ssl->options.reverse)
		print_reverse(src, digest, n);
	else
		print_normal(ssl, src, digest, n);
}

/* -p sur stdin. Le hash porte sur le flux COMPLET (\n inclus), mais l'echo affiche le contenu sans son '\n' final -> ("42 is nice")= ... sur 1 ligne */
void	print_p_open(t_ssl *ssl)
{
	if (!ssl->options.quiet)
		ft_printf("(\"");
}

void	print_p_close(t_ssl *ssl, const uint8_t *digest)
{
	if (ssl->options.quiet)
		ft_printf("\n");				/* Sépare l'echo brut du hash */
	else
		ft_printf("\")= ");
	put_hex(digest, ssl->command->algo->digest_size);
	ft_printf("\n");
}