/* See LICENSE file for copyright and license details. */
#include "libsecauth.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PEPPER "pepper"


int
main(int argc, char *argv[])
{
	struct libsecauth_spec spec;
	char *message, *hash;
	size_t size, off;
	ssize_t r;
	int fd;

	if (argc != 2) {
		fprintf(stderr, "usage: %s password", argv[0]);
		return 1;
	}

	/* -- SERVER -- */

	memset(&spec, 0, sizeof(spec));
	spec.prehash = "$6$rounds=2000$salt1$";
	spec.xferhash = "$1$salt2$";
	spec.client_rounds = 100;
	spec.server_rounds = 0;

	size = libsecauth_format_spec(&spec, NULL, 0);
	message = malloc(size);
	if (!message) {
		perror("malloc");
		return 1;
	}
	libsecauth_format_spec(&spec, message, size);

	/* -- CLIENT -- */

	memset(&spec, 0, sizeof(spec));
	if (libsecauth_parse_spec(&spec, message)) {
		perror("libsecauth_parse_spec");
		return 1;
	}

	hash = libsecauth_client_hash(&spec, argv[1]);
	if (!hash) {
		perror("libsecauth_client_hash");
		return 1;
	}
	free(message);
	message = hash;

	/* -- SERVER -- */

	memset(&spec, 0, sizeof(spec));
	spec.prehash = "$6$rounds=2000$salt1$";
	spec.xferhash = "$1$salt2$";
	spec.client_rounds = 100;
	spec.server_rounds = 0;
	spec.posthash = "$6$rounds=1000$salt3$";
	if (libsecauth_server_hash(&spec, message, PEPPER, &hash) < 0) {
		perror("libsecauth_server_hash");
		return 1;
	}
	spec.expected = hash;
	free(message);
	size = libsecauth_format_spec(&spec, NULL, 0);
	message = malloc(size);
	if (!message) {
		perror("malloc");
		return 1;
	}
	libsecauth_format_spec(&spec, message, size);
	free(hash);

	message[size - 1] = '\n';
	fd = open("democonfig", O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	for (off = 0; off < size; off += (size_t)r) {
		r = write(fd, &message[off], size - off);
		if (r <= 0) {
			perror("write");
			return 1;
		}
	}
	close(fd);
	free(message);

	return 0;
}
