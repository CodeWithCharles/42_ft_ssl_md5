/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: cpoulain <cpoulain@student.42lehavre.fr    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/02 19:05:23 by cpoulain          #+#    #+#             */
/*   Updated: 2026/07/02 19:09:31 by cpoulain         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ft_ssl.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define READ_CHUNK 65536

/* Digest en hexa minuscule (sortie brute ; le format viendra en Phase 3). */
static void	print_hex(const uint8_t *digest, size_t n)
{
	size_t	i;

	i = 0;
	while (i < n)
		ft_printf("%02x", digest[i++]);
	ft_printf("\n");
}

/* Lit un fd par chunks -> update. echo = 1 : recopie brute vers stdout (-p). */
static int	hash_fd(const t_hash_algo *algo, t_hash_ctx *ctx, int fd, int echo)
{
	uint8_t		buf[READ_CHUNK];
	ssize_t		n;

	algo->init(ctx);
	n = read(fd, buf, READ_CHUNK);
	while (n > 0)
	{
		if (echo)
			write(1, buf, (size_t)n);
		algo->update(ctx, buf, (size_t)n);
		n = read(fd, buf, READ_CHUNK);
	}
	return (n < 0);
}

static int	hash_string(t_ssl *ssl, t_source *src, uint8_t *digest)
{
	t_hash_ctx	ctx;

	ssl->command->algo->init(&ctx);
	ssl->command->algo->update(&ctx, (const uint8_t *)src->value,
		ft_strlen(src->value));
	ssl->command->algo->final(&ctx, digest);
	return (0);
}

static int	hash_file(t_ssl *ssl, t_source *src, uint8_t *digest)
{
	t_hash_ctx	ctx;
	int			fd;

	fd = open(src->value, O_RDONLY);
	if (fd < 0)
	{
		fd_printf(2, "%s: %s: %s: %s\n", ssl->prog, ssl->command->name,
			src->value, strerror(errno));
		return (1);
	}
	if (hash_fd(ssl->command->algo, &ctx, fd, 0))
		return (close(fd), 1);
	ssl->command->algo->final(&ctx, digest);
	close(fd);
	return (0);
}

static int	hash_stdin(t_ssl *ssl, uint8_t *digest)
{
	t_hash_ctx	ctx;

	if (hash_fd(ssl->command->algo, &ctx, 0, ssl->options.echo_stdin))
		return (1);
	ssl->command->algo->final(&ctx, digest);
	return (0);
}

int	run_sources(t_ssl *ssl)
{
	t_list		*node;
	t_source	*src;
	uint8_t		digest[FT_SSL_MAX_DIGEST];
	int			status;
	int			rc;

	status = 0;
	node = ssl->sources;
	while (node)
	{
		src = (t_source *)node->content;
		if (src->kind == SRC_STRING)
			rc = hash_string(ssl, src, digest);
		else if (src->kind == SRC_FILE)
			rc = hash_file(ssl, src, digest);
		else
			rc = hash_stdin(ssl, digest);
		if (rc == 0)
			print_hex(digest, ssl->command->algo->digest_size);
		else
			status = 1;
		node = node->next;
	}
	return (status);
}