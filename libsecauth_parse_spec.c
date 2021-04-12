/* See LICENSE file for copyright and license details. */
#include "libsecauth.h"

#include <ctype.h>
#include <errno.h>
#include <string.h>


static char *
get_subhash(char *s, char **endp)
{
	size_t depth = 0;
	char *ret;

	if (s[0] == '{' && s[1] == '$') {
		s = ret = &s[2];
	} else {
		ret = s;
	}
	for (; s[0]; s++) {
		if (s[0] == '{' && s[1] == '$' && s[-1] == '$') {
			depth += 1;
		} else if (s[0] == '}' && s[1] == '$' && s[-1] == '$') {
			if (!depth)
				return NULL;
			if (!--depth) {
				*s++ = '\0';
				*s++ = '\0';
				return ret;
			}
		} else if (s[0] == '$' && !depth) {
			*s++ = '\0';
			return ret;
		}
	}

	return NULL;
}

static int
strtou32(const char *s, uint32_t *valp)
{
	for (*valp = 0; isdigit(*s); s++) {
		if (*valp > (UINT32_MAX - (uint32_t)(*s & 15)) / 10)
			return -1;
		*valp = *valp * 10 + (uint32_t)(*s & 15);
	}
	return *s ? -1 : 0;
}

int
libsecauth_parse_spec(struct libsecauth_spec *spec, char *s)
{
	const char *client_rounds, *server_rounds;
	size_t slen = strlen(s);

	memset(spec, 0, sizeof(*spec));

	if (strncmp(s, "$secauth$", sizeof("$secauth$") - 1))
		goto invalid;
	s = &s[sizeof("$secauth$") - 1];

	spec->prehash  = get_subhash(s, &s);
	spec->xferhash = get_subhash(s, &s);
	client_rounds  = get_subhash(s, &s);
	server_rounds  = get_subhash(s, &s);
	spec->posthash = get_subhash(s, &s);
	spec->expected = get_subhash(s, &s);

	if (client_rounds && strtou32(client_rounds, &spec->client_rounds))
		goto invalid;
	if (server_rounds && strtou32(server_rounds, &spec->server_rounds))
		goto invalid;

	if (*s)
		goto invalid;

	return 0;

invalid:
	memset(s, 0, slen);
	errno = EINVAL;
	return -1;
}
