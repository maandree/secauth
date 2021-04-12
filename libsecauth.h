/* See LICENSE file for copyright and license details. */
#ifndef LIBSECAUTH_H
#define LIBSECAUTH_H

#include <stdint.h>
#include <stddef.h>


/* $secauth${$<prehash>$}${$<xferhash>$}$<client_rounds>$<server_rounds>${$<posthash>$}$<expected> */


struct libsecauth_spec {
	const char *prehash; /* secret if empty, all information is stored at client-side */
	const char *xferhash;
	const char *posthash;
	const char *expected;
	uint32_t client_rounds;
	uint32_t server_rounds;
};


int libsecauth_parse_spec(struct libsecauth_spec *spec, char *settings);
size_t libsecauth_format_spec(struct libsecauth_spec *spec, char *buffer, size_t buffer_size);

char *libsecauth_client_hash(const struct libsecauth_spec *spec, const char *password);
int libsecauth_server_hash(const struct libsecauth_spec *spec, const char *inhash, const char *pepper, char **resultp);


#endif
