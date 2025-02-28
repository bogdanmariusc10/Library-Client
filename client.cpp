#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "nlohmann/json.hpp"
#include "helpers.hpp"
#include "requests.hpp"

using json = nlohmann::json;
using namespace std;

const string HOST = "34.246.184.49";
const int PORT = 8080;

string session_cookie = "";
string auth_token = "";

void register_user(int sockfd)
{
    string username, password;

    cout << "username=";
    cin.ignore();
    getline(cin,username);
    cout << "password=";
    getline(cin,password);

    if (username.find(' ') != string::npos)
    {
        cout << "ERROR - Numele de utilizator nu trebuie sa contina spatii!" << endl;
        return;
    }

    if (password.find(' ') != string::npos)
    {
        cout << "ERROR - Parola nu trebuie sa contina spatii!" << endl;
        return;
    }

    json payload = { {"username", username}, {"password", password} };
    char url[] = "/api/v1/tema/auth/register";
    char content_type[] = "application/json";
    
    const char *message = compute_post_request(HOST.c_str(), url, content_type, &payload, "");
    sockfd = open_connection(HOST.c_str(), PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    string response = receive_from_server(sockfd);

    if (response.find("400 Bad Request") == string::npos)
    {
        cout << "SUCCESS - Utilizatorul a fost inregistrat cu succes!" << endl;
    }
    else
    {
        cout << "ERROR - Numele de utilizator este deja folosit!" << endl;
    }

    close_connection(sockfd);
}

void login_user(int sockfd)
{
    string username, password;

    cout << "username=";
    cin.ignore();
    getline(cin,username);
    cout << "password=";
    getline(cin,password);

    if (username.find(' ') != string::npos)
    {
        cout << "ERROR - Numele de utilizator nu trebuie sa contina spatii!" << endl;
        return;
    }
    if (password.find(' ') != string::npos)
    {
        cout << "ERROR - Parola nu trebuie sa contina spatii!" << endl;
        return;
    }

    json payload = { {"username", username}, {"password", password} };
    char url[] = "/api/v1/tema/auth/login";
    char content_type[] = "application/json";
    const char *message = compute_post_request(HOST.c_str(), url, content_type, &payload, "");
    sockfd = open_connection(HOST.c_str(), PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    string response = receive_from_server(sockfd);

    size_t session_cookie_position = response.find("Set-Cookie:");
    if (session_cookie_position != string::npos)
    {
        size_t start_value = response.find("connect.sid=", session_cookie_position);
        if (start_value != string::npos)
        {
            size_t end_value = response.find("HttpOnly", start_value);
            if (end_value != string::npos)
            {
                session_cookie = response.substr(start_value, end_value - start_value + strlen("HttpOnly"));
            }
        }
    }

    if (response.find("400 Bad Request") == string::npos)
    {
        cout << "SUCCESS - Utilizatorul s-a logat cu succes!" << endl;
    }
    else
    {
        cout << "ERROR - Credentialele nu se potrivesc!" << endl;
    }

    close_connection(sockfd);
}

void enter_library(int sockfd)
{
    if (session_cookie.empty())
    {
        cout << "ERROR - Autentifica-te mai intai!" << endl;
        return;
    }

    char url[] = "/api/v1/tema/library/access";
    const char *message = compute_get_request(HOST.c_str(), url, nullptr, session_cookie, session_cookie.size(),"");
    sockfd = open_connection(HOST.c_str(), PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    string response = receive_from_server(sockfd);

    if (response.find("400 Bad Request") == string::npos)
    {
        cout << "SUCCESS - Utilizatorul are acces la biblioteca!" << endl;
    }
    else
    {
        cout << "ERROR - Nu s-a putut obtine acces la biblioteca!" << endl;
        return;
    }

    size_t auth_token_position = response.find("\"token\":\"");
    if (auth_token_position != string::npos)
    {
        size_t start_value = auth_token_position + strlen("\"token\":\"");
        size_t end_value = response.find("\"", start_value);
        if (end_value != string::npos)
        {
            auth_token = response.substr(start_value, end_value - start_value);
        }
    }

    close_connection(sockfd);
}

void get_books(int sockfd)
{
    if (auth_token.empty())
    {
        cout << "ERROR - Autentifica-te si obtine acces la biblioteca mai intai!" << endl;
        return;
    }

    char url[] = "/api/v1/tema/library/books";
    const char *message = compute_get_request(HOST.c_str(), url, nullptr, session_cookie, 0, auth_token);
    sockfd = open_connection(HOST.c_str(), PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    string response = receive_from_server(sockfd);

    size_t books_start = response.find("[");
    size_t books_end = response.rfind("]");
    if (books_start != string::npos && books_end != string::npos)
    {
        string books_str = response.substr(books_start, books_end - books_start + 1);
        try
        {
            auto books = json::parse(books_str);
            cout << books.dump(4) << endl;
        }
        catch (const json::parse_error& e)
        {
            cout << "ERROR - Nu s-a parsat JSON!" << e.what() << endl;
        }
    }
    else
    {
        cout << "ERROR - NU s-a gasit JSON!" << endl;
    }

    close_connection(sockfd);
}


void get_book(int sockfd)
{
    long book_id;
    cout << "id=";
    cin >> book_id;

    if (auth_token.empty())
    {
        cout << "ERROR - Autentifica-te si obtine acces la biblioteca mai intai!" << endl;
        return;
    }

    sockfd = open_connection(HOST.c_str(), PORT, AF_INET, SOCK_STREAM, 0);

    char url[50];
    sprintf(url, "/api/v1/tema/library/books/%ld", book_id);
    const char *message = compute_get_request(HOST.c_str(),url, nullptr, session_cookie, 0, auth_token);
    send_to_server(sockfd, message);
    string response = receive_from_server(sockfd);


    size_t book_start = response.find("{");
    if (book_start != string::npos)
    {
        string json_str = response.substr(book_start);
        try
        {
            json book = json::parse(json_str);
            cout << "{\n";
            cout << "   \tid: " << book["id"] << "\n";
            cout << "   \ttitle: " << book["title"].get<string>() << "\n";
            cout << "   \tauthor: " << book["author"].get<string>() << "\n";
            cout << "   \tpublisher: " << book["publisher"].get<string>() << "\n";
            cout << "   \tgenre: " << book["genre"].get<string>() << "\n";
            cout << "   \tpage_count: " << book["page_count"] << "\n";
            cout << "}\n";
        }
        catch (const json::parse_error& e)
        {
            cout << "ERROR - Nu s-a parsat JSON!" << e.what() << endl;
        }
    }
    else
    {
        cout << "ERROR - NU s-a gasit JSON!" << endl;
    }

    close_connection(sockfd);
}

void add_book(int sockfd)
{
    string title, author, genre, publisher, page_count;

    cout << "title=";
    cin.ignore();
    getline(cin, title);
    cout << "author=";
    getline(cin, author);
    cout << "genre=";
    getline(cin, genre);
    cout << "publisher=";
    getline(cin, publisher);
    cout << "page_count=";
    getline(cin, page_count);

    if (auth_token.empty())
    {
        cout << "ERROR - Autentifica-te si obtine acces la biblioteca mai intai!" << endl;
        return;
    }

    if (title.empty())
    {
        cout << "ERROR - Titlul nu poate fi gol!" << endl;
        return;
    }
    else if (author.empty())
    {
        cout << "ERROR - Autorul nu poate fi gol!" << endl;
        return;
    }
    else if (genre.empty())
    {
        cout << "ERROR - Genul nu poate fi gol!" << endl;
        return;
    }
    else if (publisher.empty())
    {
        cout << "ERROR - Publicatia nu poate fi goala!" << endl;
        return;
    }

    int page_count_int;
    try
    {
        page_count_int = stoi(page_count);
        if (page_count_int <= 0)
        {
            throw invalid_argument("ERROR - Numarul de pagini trebuie sa fie un numar intreg pozitiv!");
        }
    } catch (invalid_argument& e)
    {
        cout << "ERROR - Numarul de pagini trebuie sa fie un numar intreg pozitiv!" << endl;
        return;
    }

    json payload = {
        {"title", title},
        {"author", author},
        {"genre", genre},
        {"page_count", page_count_int},
        {"publisher", publisher}
    };

    char url[] = "/api/v1/tema/library/books";
    char content_type[] = "application/json";
    const char *message = compute_post_request(HOST.c_str(), url, content_type, &payload, auth_token);
    sockfd = open_connection(HOST.c_str(), PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    string response = receive_from_server(sockfd);

    if(response.find("400 Bad Request") == string::npos)
    {
        cout << "SUCCESS - Cartea a fost adaugata cu succes!" << endl;
    }
    else
    {
        cout<< "ERROR -  Cartea nu a putut fi adaugata!" << endl;
    }

    close_connection(sockfd);
}

void delete_book(int sockfd)
{
    string book_id;
    cout << "id=";
    cin >> book_id;

    if (auth_token.empty())
    {
        cout << "ERROR - Autentifica-te si obtine acces la biblioteca mai intai!" << endl;
        return;
    }

    string url = "/api/v1/tema/library/books/" + book_id;
    const char *message = compute_delete_request(HOST.c_str(), url.c_str(),"", session_cookie, auth_token);
    sockfd = open_connection(HOST.c_str(), PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    string response = receive_from_server(sockfd);

    if(response.find("400 Bad Request") == string::npos)
    {
        cout << "SUCCESS - Cartea a fost stearsa cu succes!" << endl;
    }
    else
    {
        cout<< "ERROR - Cartea nu a putut fi stearsa!" << endl;
    }

    close_connection(sockfd);
}

void logout_user(int sockfd)
{

    if (session_cookie.empty())
    {
        cout << "ERROR - Autentifica-te mai intai!" << endl;
        return;
    }

    char url[] = "/api/v1/tema/auth/logout";
    const char *message = compute_get_request(HOST.c_str(), url, nullptr, session_cookie, 0, "");
    sockfd = open_connection(HOST.c_str(), PORT, AF_INET, SOCK_STREAM, 0);
    send_to_server(sockfd, message);
    string response = receive_from_server(sockfd);


    if (response.find("400 Bad Request") == string::npos)
    {
        session_cookie.clear();
        auth_token.clear();
        cout << "SUCCESS - Utilizatorul a fost delogat cu succes!" << endl;
    }
    else
    {
        cout << "ERROR - Nu s-a putut realiza delogarea!" << endl;
    }

    close_connection(sockfd);
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR - Nu s-a creat socket-ul!");
        exit(EXIT_FAILURE);
    }

    server = gethostbyname(HOST.c_str());
    if (server == NULL)
    {
        cerr << "ERROR - Nu exista host-ul!" << endl;
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    server_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("ERROR - Nu s-a putut realiza conectarea la server!");
        exit(EXIT_FAILURE);
    }

    while (true) {
        cout << "Comenzi:\n";
        cout << "register\n";
        cout << "login\n";
        cout << "enter_library\n";
        cout << "get_books\n";
        cout << "get_book\n";
        cout << "add_book\n";
        cout << "delete_book\n";
        cout << "logout\n";
        cout << "exit\n";

        string option;
        cout << "Introdu comanda: " << endl;
        cin >> option;

        if (option == "register")
        {
            register_user(sockfd);
        }
        else if (option == "login")
        {
            login_user(sockfd);
        }
        else if (option == "enter_library")
        {
            enter_library(sockfd);
        }
        else if (option == "get_books")
        {
            get_books(sockfd);
        }
        else if (option == "get_book")
        {
            get_book(sockfd);
        }
        else if (option == "add_book")
        {
            add_book(sockfd);
        }
        else if (option == "delete_book")
        {
            delete_book(sockfd);
        }
        else if (option == "logout")
        {
            logout_user(sockfd);
        }
        else if (option == "exit")
        {
            exit(0);
        }
        else
        {
            cout << "Opțiune invalidă. Vă rugăm să încercați din nou." << endl;
            continue;
        }
        cout << endl;
    }

    return 0;
}
