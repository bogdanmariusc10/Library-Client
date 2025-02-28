#include <cstdlib>
#include <string>
#include "helpers.hpp"
#include "requests.hpp"
#include "nlohmann/json.hpp"

using namespace std;
using json_t = nlohmann::json;

char *compute_delete_request(string host, string url, string query_params,
                            const string& cookie, const string& JWT_token)
{
    char *message = (char *)calloc(BUFLEN, sizeof(char));
    char *line = (char *)calloc(LINELEN, sizeof(char));

    if (query_params != "") {
        sprintf(line, "DELETE %s?%s HTTP/1.1", url.c_str(), query_params.c_str());
    } else {
        sprintf(line, "DELETE %s HTTP/1.1", url.c_str());
    }

    compute_message(message, line);

    sprintf(line, "Host: %s", host.c_str());
    compute_message(message, line);    

    if (!cookie.empty()) {
        sprintf(line, "Cookie: %s", cookie.c_str());
        
        compute_message(message, line);
    }

    if (!JWT_token.empty()) {
        sprintf(line, "Authorization: Bearer %s", JWT_token.c_str());
        compute_message(message, line); 
    }

    compute_message(message, "");
    return message;
}

char * compute_get_request(const char * host, char * url, char * query_params, const string & cookies, int cookies_count, const string & token)
{
    char *message = (char *) calloc(BUFLEN, sizeof(char));
    char *line = (char *) calloc(LINELEN, sizeof(char));

    if (query_params != nullptr) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }

    compute_message(message, line);

    memset(line, 0, sizeof(char) * LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    if (!cookies.empty()) {
        memset(line, 0, sizeof(char) * LINELEN);
        sprintf(line, "Cookie: %s", cookies.c_str());
        compute_message(message, line);
    }

    if (!token.empty()) {
        memset(line, 0, sizeof(char) * LINELEN);
        sprintf(line, "Authorization: Bearer %s", token.c_str());
        compute_message(message, line);
    }

    compute_message(message, "");
    free(line);
    return message;
}

char *compute_post_request(const char *host, char *url, char* content_type, json_t *json, const string& token)
{
    char *message = (char *) calloc(BUFLEN, sizeof(char));
    char *line = (char *) calloc(LINELEN, sizeof(char));

    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);
    
    memset(line, 0, sizeof(char) * LINELEN);
    sprintf(line, "Host: %s", host);
    compute_message(message, line);

    auto payload = json->dump(4);

    memset(line, 0, sizeof(char) * LINELEN);
    sprintf(line, "Content-Type: %s", content_type);
    compute_message(message, line);

    memset(line, 0, sizeof(char) * LINELEN);
    sprintf(line, "Content-Length: %ld", strlen(payload.c_str()));
    compute_message(message, line);

    if (!token.empty()) {
        memset(line, 0, sizeof(char) * LINELEN);
        sprintf(line, "Authorization: Bearer %s", token.c_str());
        compute_message(message, line);
    }

    compute_message(message, "");

    memset(line, 0, LINELEN);
    strcat(message, payload.c_str());

    free(line);
    return message;
}
