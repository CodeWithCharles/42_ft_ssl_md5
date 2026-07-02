/* ************************************************************************** */
/*                                   exec.c                                   */
/* ************************************************************************** */

#include "ft_ssl.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define READ_CHUNK 65536

/* Etat d'echo -p : retient un eventuel '\n' final pour le stripper en flux. */
typedef struct s_echo
{
	int	pending_nl;
}	t_echo;

/* Recopie un chunk vers stdout en retenant le dernier '\n' du flux global. */
static void	echo_chunk(t_echo *e, const uint8_t *buf, size_t n)
{
	if (e->pending_nl && n > 0)
	{
		write(1, "\n", 1);
		e->pending_nl = 0;
	}
	if (n > 0 && buf[n - 1] == '\n')
	{
		e->pending_nl = 1;
		n--;
	}
	if (n > 0)
		write(1, buf, n);
}

/* Lit un fd par chunks -> update. echo != NULL : recopie (-p, strip \n final). */
static int	hash_fd(const t_hash_algo *algo, t_hash_ctx *ctx, int fd,
		t_echo *echo)
{
	uint8_t	buf[READ_CHUNK];
	ssize_t	n;

	algo->init(ctx);
	n = read(fd, buf, READ_CHUNK);
	while (n > 0)
	{
		if (echo)
			echo_chunk(echo, buf, (size_t)n);
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
	if (hash_fd(ssl->command->algo, &ctx, fd, NULL))
		return (close(fd), 1);
	ssl->command->algo->final(&ctx, digest);
	close(fd);
	return (0);
}

static int	hash_stdin_plain(t_ssl *ssl, uint8_t *digest)
{
	t_hash_ctx	ctx;

	if (hash_fd(ssl->command->algo, &ctx, 0, NULL))
		return (1);
	ssl->command->algo->final(&ctx, digest);
	return (0);
}

/* -p sur stdin : echo (strip \n) encadre par ("...")= / mode quiet. */
static int	run_p_stdin(t_ssl *ssl, uint8_t *digest)
{
	t_hash_ctx	ctx;
	t_echo		echo;

	echo.pending_nl = 0;
	print_p_open(ssl);
	if (hash_fd(ssl->command->algo, &ctx, 0, &echo))
		return (1);
	ssl->command->algo->final(&ctx, digest);
	print_p_close(ssl, digest);
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
		if (src->kind == SRC_STDIN && ssl->options.echo_stdin)
			rc = run_p_stdin(ssl, digest);
		else
		{
			if (src->kind == SRC_STRING)
				rc = hash_string(ssl, src, digest);
			else if (src->kind == SRC_FILE)
				rc = hash_file(ssl, src, digest);
			else
				rc = hash_stdin_plain(ssl, digest);
			if (rc == 0)
				print_digest(ssl, src, digest);
		}
		if (rc)
			status = 1;
		node = node->next;
	}
	return (status);
}