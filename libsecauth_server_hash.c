/* See LICENSE file for copyright and license details. */
#include "libsecauth.h"

#include <crypt.h>
#include <stdlib.h>
#include <string.h>


int
libsecauth_server_hash(const struct libsecauth_spec *spec, const char *inhash, const char *pepper, char **resultp)
{
	struct crypt_data hashbuf[2];
	const char *hash = inhash, *result;
	char *posthash = NULL, *p;
	uint32_t rounds;
	size_t i = 0;

	*resultp = NULL;
	memset(hashbuf, 0, sizeof(hashbuf));

	for (i = 0, rounds = spec->server_rounds; rounds--; i ^= 1) {
		hash = crypt_r(hash, spec->xferhash, &hashbuf[i]);
		if (!hash)
			return -1;
	}

	if (pepper) {
		posthash = malloc(strlen(spec->posthash) + strlen(pepper) + 2);
		if (!posthash)
			return -1;
		p = stpcpy(posthash, spec->posthash);
		if (*posthash && p[-1] == '$')
			p -= 1;
		stpcpy(p, pepper);
	}

	hash = crypt_r(hash, posthash ? posthash : spec->posthash, &hashbuf[i]);
	free(posthash);
	if (!hash)
		return -1;

	result = strrchr(hash, '$');
	result = result ? &result[1] : hash;

	if (resultp) {
		*resultp = strdup(result);
		if (!*resultp)
			return -1;
	}

	if (!spec->expected || !*spec->expected)
		return 0;

	return !strcmp(result, spec->expected);
}
