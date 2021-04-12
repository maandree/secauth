/* See LICENSE file for copyright and license details. */
#include "libsecauth.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>


size_t
libsecauth_format_spec(struct libsecauth_spec *spec, char *buffer, size_t buffer_size)
{
	char client_rounds[sizeof("4294967295")];
	char server_rounds[sizeof("4294967295")];
	int i, expected_with_dollars;
	client_rounds[0] = '\0';
	server_rounds[0] = '\0';
	if (spec->client_rounds)
		sprintf(client_rounds, "%"PRIu32, spec->client_rounds);
	if (spec->server_rounds)
		sprintf(server_rounds, "%"PRIu32, spec->server_rounds);
	expected_with_dollars = spec->expected && strchr(spec->expected, '$');
	i = snprintf(buffer, buffer_size, "$secauth$%s%s%s$%s%s%s$%s$%s$%s%s%s$%s%s%s",
	             spec->prehash  ? "{$" : "", spec->prehash  ? spec->prehash  : "", spec->prehash  ? "$}" : "",
	             spec->xferhash ? "{$" : "", spec->xferhash ? spec->xferhash : "", spec->xferhash ? "$}" : "",
	             client_rounds, server_rounds,
	             spec->posthash ? "{$" : "", spec->posthash ? spec->posthash : "", spec->posthash ? "$}" : "",
	             expected_with_dollars ? "{$" : "", spec->expected ? spec->expected : "" , expected_with_dollars ? "$}" : "");
	return i > 0 ? (size_t)i : 0;
}
