/* See LICENSE file for copyright and license details. */
#include "libsecauth.h"

#include <crypt.h>
#include <string.h>


char *
libsecauth_client_hash(const struct libsecauth_spec *spec, const char *password)
{
	struct crypt_data hashbuf[2];
	const char *hash = password;
	uint32_t rounds;
	size_t i;

	memset(hashbuf, 0, sizeof(hashbuf));

	if (spec->prehash && *spec->prehash) {
		hash = crypt_r(password, spec->prehash, &hashbuf[1]);
		if (!hash)
			return NULL;
	}

	for (i = 0, rounds = spec->client_rounds; rounds--; i ^= 1) {
		hash = crypt_r(hash, spec->xferhash, &hashbuf[i]);
		if (!hash)
			return NULL;
	}

	return strdup(hash);
}
