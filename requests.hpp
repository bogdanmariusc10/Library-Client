#ifndef _REQUESTS_
#define _REQUESTS_

#include "nlohmann/json.hpp"

using namespace std;
using json_t = nlohmann::json;

// Computes and returns a DELETE request.
char *compute_delete_request(string host, string url, string query_params,
                            const string& cookie, const string& JWT_token);

// Computes and returns a GET request string (query_params
// and cookies can be set to NULL if not needed)
char *compute_get_request(const char *host, char *url, char *query_params,
							const string &cookies, int cookies_count, const string& token);

// Computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(const char *host, char *url, char* content_type, json_t *json, const string& token);

#endif