/* See LICENSE file for copyright and license details. */
#include "libsecauth.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PEPPER "pepper"


static char *
server_lookup_hash(void)
{
	char *settings;
	size_t size, len;
	ssize_t r;
	int fd;

	fd = open("democonfig", O_RDONLY);
	if (fd < 0) {
		perror("open");
		return NULL;
	}
	settings = NULL;
	size = 0;
	for (len = 0;; len += (size_t)r) {
		if (len == size) {
			settings = realloc(settings, size += 128);
			if (!settings) {
				perror("realloc");
				return NULL;
			}
		}
		r = read(fd, &settings[len], size - len);
		if (r <= 0) {
			if (!r)
				break;
			perror("read");
			return NULL;
		}
	}
	close(fd);
	if (!len || settings[len - 1] != '\n') {
		fprintf(stderr, "./democonfig is corrupt\n");
		return NULL;
	}
	settings[len - 1] = '\0';

	return settings;
}

int
main(int argc, char *argv[])
{
	struct libsecauth_spec spec;
	char *message, *settings, *expected;
	size_t size, off;
	int fd;
	ssize_t w;
	int r;

	if (argc != 2) {
		fprintf(stderr, "usage: %s password", argv[0]);
		return 1;
	}

	/* -- SERVER -- */

	settings = server_lookup_hash();
	if (!settings)
		return 1;
	if (libsecauth_parse_spec(&spec, settings)) {
		perror("libsecauth_parse_spec");
		return 1;
	}
	spec.posthash = NULL;
	spec.expected = NULL;
	spec.client_rounds -= spec.client_rounds > 0;
	spec.server_rounds = 0;

	size = libsecauth_format_spec(&spec, NULL, 0);
	message = malloc(size);
	if (!message) {
		perror("malloc");
		return 1;
	}
	libsecauth_format_spec(&spec, message, size);
	free(settings);

	/* -- CLIENT -- */

	settings = message;
	if (libsecauth_parse_spec(&spec, settings)) {
		perror("libsecauth_parse_spec");
		return 1;
	}
	message = libsecauth_client_hash(&spec, argv[1]);
	if (!message) {
		perror("libsecauth_client_hash");
		return 1;
	}
	free(settings);

	/* -- SERVER -- */

	settings = server_lookup_hash();
	if (!settings)
		return 1;
	if (libsecauth_parse_spec(&spec, settings)) {
		perror("libsecauth_parse_spec");
		return 1;
	}
	spec.server_rounds += spec.client_rounds > 0;
	spec.client_rounds -= spec.client_rounds > 0;
	r = libsecauth_server_hash(&spec, message, PEPPER, NULL);
	if (r < 0) {
		perror("libsecauth_server_hash");
		return 1;
	} else if (!r) {
		free(settings);
		free(message);
		printf("Incorrect password\n");
		return 0;
	}
	printf("Login OK\n");

	expected = NULL;
	if (!spec.client_rounds) {
		spec.xferhash = "$1$newsalt1$";
		spec.posthash = "$6$rounds=1000$newsalt2$";
		spec.expected = NULL;
		spec.server_rounds = 100;
		if (libsecauth_server_hash(&spec, message, PEPPER, &expected) < 0) {
			perror("libsecauth_server_hash");
			return 1;
		}
		spec.client_rounds = spec.server_rounds;
		spec.server_rounds = 0;
		spec.expected = expected;
		printf("password rehashed\n");
	}
	free(message);
	size = libsecauth_format_spec(&spec, NULL, 0);
	message = malloc(size);
	if (!message) {
		perror("malloc");
		return 1;
	}
	libsecauth_format_spec(&spec, message, size);
	free(settings);
	free(expected);

	message[size - 1] = '\n';
	fd = open("democonfig", O_WRONLY | O_TRUNC, 0600);
	if (fd < 0) {
		perror("open");
		return 1;
	}
	for (off = 0; off < size; off += (size_t)w) {
		w = write(fd, &message[off], size - off);
		if (w <= 0) {
			perror("write");
			return 1;
		}
	}
	close(fd);

	free(message);

	return 0;
}
