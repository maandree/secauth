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
	char *pepperedhash = NULL;
	uint32_t rounds;
	size_t i = 0;

	if (resultp)
		*resultp = NULL;
	memset(hashbuf, 0, sizeof(hashbuf));

	for (i = 0, rounds = spec->server_rounds; rounds--; i ^= 1) {
		hash = crypt_r(hash, spec->xferhash, &hashbuf[i]);
		if (!hash)
			return -1;
	}

	if (pepper) {
		pepperedhash = malloc(strlen(pepper) + strlen(hash) + 1);
		if (!pepperedhash)
			return -1;
		stpcpy(stpcpy(pepperedhash, pepper), hash);
		hash = pepperedhash;
	}

	hash = crypt_r(hash, spec->posthash, &hashbuf[i]);
	free(pepperedhash);
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
