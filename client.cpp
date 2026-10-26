// Hello! Made by Ainselu, on the night of October 25th, 2026.
// Undergoing development. It's pretty cool but it's pretty mid.
// @TODO ­— Note on something to implement later.
// @NOTE — General comment or suggestion to look over.
// CTRL-F them if you want to see those.

#include "crow.h"
#include "nlohmann/json.hpp"
#include <cpr/api.h>
#include <crow/http_response.h>
#include <crow/websocket.h>
#include <nlohmann/json_fwd.hpp>
#include <cctype>
#include <argon2.h>
#include <openssl/rand.h>
#include <argon2.h>
#include <string>
#include "config.hpp"
#include "rgb.hpp"

using namespace std;
using namespace crow;

// All connected clients, and their usernames.
vector<pair<string, websocket::connection*>> conns;

// If accessing any of the above two things, lock this first!
mutex connections_mutex;

// Returns true if a username is entirely alphanumeric and within size limits.
bool is_uname_sanitary(string uname){
    if (uname.size() < 4 || uname.size() > 20)
        return false;

    for (auto c : uname)
        if (!isalnum(c))
            return false;
    return true;
}

int main(int argc, char *argv[]){
    if (argc != 4){
        cout << "HiveChat takes 3 arguments (port, bind address, and password)\n";
        return -1;
    }

    // Server password, required for entry (knock knock..!!)
    const char* PASSWORD = argv[3];

    SimpleApp app;

    // Main and sole route
    CROW_WEBSOCKET_ROUTE(app, "/hivep2p")

    // Called when the socket is accepted and they send a handshake, I guess.
    // Returns true if we proceed to the onopen (opening the websocket
    // connection), returns false if we drop it (not calling onclose.)
    .onaccept([&](const request& req, void**) -> bool {
        // Password should be stored here.
        string auth_header = req.get_header_value("Authorization");

        // Lock up tight, cuz we're accessing PASSWORD.
        lock_guard<mutex> lock(connections_mutex);

        // If the sizes aren't equal, don't bother proceeding.
        if (auth_header.size() != strlen(PASSWORD))
            return false;
        
        // Compare in constant time if they're equal.
        if (CRYPTO_memcmp(
            auth_header.data(),
            PASSWORD,
            strlen(PASSWORD)
        ) != 0)
            return false;
                
        return true;
    })
    
    // Websocket connection is opened and started, lesgo.
    .onopen([&](websocket::connection& conn) {
        // We use a random anonymous username.
        // We would assign a username earlier or here, but there
        // is no way to do so with only one endpoint, as it seems.
        random_device rd;
        mt19937 gen(rd());// Hello! Made by Ainselu, on the night of October 25th, 2026.
// Undergoing development. It's pretty cool but it's pretty mid.
// @TODO ­— Note on something to implement later.
// @NOTE — General comment or suggestion to look over.
// CTRL-F them if you want to see those.

#include "crow.h"
#include "nlohmann/json.hpp"
#include <cpr/api.h>
#include <crow/http_response.h>
#include <crow/websocket.h>
#include <nlohmann/json_fwd.hpp>
#include <cctype>
#include <argon2.h>
#include <openssl/rand.h>
#include <argon2.h>
#include <string>
#include "config.hpp"
#include "rgb.hpp"

using namespace std;
using namespace crow;

// All connected clients, and their usernames.
vector<pair<string, websocket::connection*>> conns;

// If accessing any of the above two things, lock this first!
mutex connections_mutex;

// Returns true if a username is entirely alphanumeric and within size limits.
bool is_uname_sanitary(string uname){
    if (uname.size() < 4 || uname.size() > 20)
        return false;

    for (auto c : uname)
        if (!isalnum(c))
            return false;
    return true;
}

int main(int argc, char *argv[]){
    if (argc != 4){
        cout << "HiveChat takes 3 arguments (port, bind address, and password)\n";
        return -1;
    }

    // Server password, required for entry (knock knock..!!)
    const char* PASSWORD = argv[3];

    SimpleApp app;

    // Main and sole route
    CROW_WEBSOCKET_ROUTE(app, "/hivep2p")

    // Called when the socket is accepted and they send a handshake, I guess.
    // Returns true if we proceed to the onopen (opening the websocket
    // connection), returns false if we drop it (not calling onclose.)
    .onaccept([&](const request& req, void**) -> bool {
        // Password should be stored here.
        string auth_header = req.get_header_value("Authorization");

        // Lock up tight, cuz we're accessing PASSWORD.
        lock_guard<mutex> lock(connections_mutex);

        // If the sizes aren't equal, don't bother proceeding.
        if (auth_header.size() != strlen(PASSWORD))
            return false;
        
        // Compare in constant time if they're equal.
        if (CRYPTO_memcmp(
            auth_header.data(),
            PASSWORD,
            strlen(PASSWORD)
        ) != 0)
            return false;
                
        return true;
    })
    
    // Websocket connection is opened and started, lesgo.
    .onopen([&](websocket::connection& conn) {
        // We use a random anonymous username.
        // We would assign a username earlier or here, but there
        // is no way to do so with only one endpoint, as it seems.
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> distrib(1000, 9999);
        string username = "Anonymous "+to_string(distrib(gen));
        
        // Because we're accessing conns.
        lock_guard<mutex> lock(connections_mutex);
        for (auto [uname, client] : conns)
            client->send_text("{\"username\": \"system\", \"message\": \"New user "+username+" has joined\"}");
        
        // Welcome aboard matey
        conns.push_back(pair<string, websocket::connection*>(username, &conn));

        std::hash<std::string> hasher;
        RGB rgb = hex_to_rgb(
            hasher(
                "system"
            ) % 0xFFFFFF
        );
        
        cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+"system"+"\033[0m]> " << "New user "+username+" has joined" << "\n";

    })

    .onmessage([](websocket::connection& conn, const string& data, bool is_binary) {
        // We don't want binary data.
        if (is_binary)
            conn.close("{\"error\": \"Invalid format\"}", websocket::CloseStatusCode::UnacceptableData);

        auto json_body = json::load(data);
        if (!json_body.has("message"))
            // We use message commands.
            conn.close("{\"error\": \"A message is required\"}", websocket::CloseStatusCode::UnacceptableData);
        
        // The actual message they're sending.
        string msg = static_cast<string>(json_body["message"].s());

        // Lock up tight because we're accessing conns, 
        lock_guard<mutex> lock(connections_mutex);

        // Command that gets all memebrs. This should generally
        // be used for the implementation of member lists in GUI clients.
        if (msg == ":members"){
            string mems = "{\"membercount\": [";
            for (auto [uname, sess] : conns)
                mems += "\""+uname+"\",";
            
            // This can fail if conns is empty, but this condition will never trigger.
            // As there's no way to send a message if yoy're not connected. I don't
            // care about this condition and won't check for it.
            mems[mems.size()-1] = ']';
            mems += "}";
            
            conn.send_text(mems);
            return;
        }

        // Client changes their username.
        else if (msg.starts_with(":username")){
            // Size of ":username " is 10, so it checks if there is
            // the command and a leading shitespace after the command.
            if (msg.size()<10){
                conn.send_text("{\"error\": \"No username given\"}");
                return;
            }

            // Since it's len 10, that minus 1 for the last index
            // would be 9.
            string username = msg.substr(9,msg.size()-1);

            // Server is reserved for serverside messages and client
            // is reserved for clientside messages.
            if (username == "server" || username == "client") {
                conn.send_text("{\"error\": \"Username reserved\"}");
                return;
            };

            // Check if the username is taken otherwise before assigning it.
            for (auto [uname, client] : conns)
                if (username == uname) {
                    conn.send_text("{\"error\": \"Username taken\"}");
                    return;
                }
            
            // We can't get the connection from the username, so we have to look it up
            // manually. Maybe later we could use a hash map or some shit? Could be more efficient. @NOTE
            for (auto& [uname, client] : conns){
                if (client == &conn) {
                    uname = msg.substr(10);
                    break;
                }
            }
        }
        // The name of the person who changed their name as a variable outside the loop.
        string username;
        
        for (auto& [uname, client] : conns) {
            if (client == &conn) {
                username = uname;
                break;
            }
        }       
        
        // Send the message, regardless of it's a command or not.
        for (auto& [uname, client] : conns) {
            client->send_text("{\"username\": \""+username+"\", \"message\": \""+msg+"\"}");
        }
        std::hash<std::string> hasher;
        RGB rgb = hex_to_rgb(
            hasher(
                username
            ) % 0xFFFFFF
        );
        
        cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+username+"\033[0m]> " << msg << "\n";
    })
    // When they leave or disconnect themselves. Or we disconnect them I think?
    .onclose([&](websocket::connection& conn, const string& reason, uint16_t status_code) {
        // Lock it up tight cuz we're accessing conns.
        lock_guard<mutex> lock(connections_mutex);
        
        // Username of the client.
        string username;
        bool found = false;

        for (auto& [uname, client] : conns) {
            if (client == &conn) {
                username = uname;
                found = true;
                break;
            }
        }

        if (!found) {
            return;
        }
        
        // Get them out of the list of clients.
        conns.erase(
            remove(
                conns.begin(),
                conns.end(),
                pair<
                    string,
                    websocket::connection*
                >(
                    username, &conn
                )
            ),
            conns.end()
        );
        // Hear ye, hear ye! The client has disconnected!
        for (auto [uname, client] : conns)
            client->send_text("{\"username\": \"system\", \"message\": \"Member "+username+" has left\"}");

        std::hash<std::string> hasher;
        RGB rgb = hex_to_rgb(
            hasher(
                "system"
            ) % 0xFFFFFF
        );
        
        cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+"system"+"\033[0m]> " << "Member "+username+" has left" << "\n";

    });

    auto shutup = async(launch::async, [&]() {
        string msg;

        std::hash<std::string> hasher;
        RGB rgb = hex_to_rgb(
            hasher(
                "system"
            ) % 0xFFFFFF
        );
        
        while (getline(cin, msg)) {
            nlohmann::json payload;
            payload["message"] = msg;
            payload["username"] = "system";

            string serialized = payload.dump();
            for (auto& [uname, client] : conns) {
                client->send_text(serialized);
            }
            
            cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+"system"+"\033[0m]> " << msg << "\n";
            if (msg.substr(0,6) == ":kick " && msg.size() > 7){
                bool found = false;
                for (auto& [uname, client] : conns) {
                    if (uname == msg.substr(6)){
                        client->close(
                            "{\"username\": \"system\", \"message\": \"You have been kicked from the server\"}",
                            websocket::CloseStatusCode::NormalClosure
                        );
                        found = true;
                        break;
                    }
                }
                if (!found)
                    // An abnormal condition here.
                    cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+"system"+"\033[0m]> " << "The requested user was not found." << "\n";
            }
            
        }
    });
    
    // @TODO Add support for TLS.
    std::string arg1(argv[1]); 
    app
        .port(stoi(arg1))
        #ifdef SSL_ON
        .ssl_file(SSL_CERT, SSL_KEY)
        #endif
        .bindaddr(argv[2])
        .multithreaded().run();
}
        uniform_int_distribution<int> distrib(1000, 9999);
        string username = "Anonymous "+to_string(distrib(gen));
        
        // Because we're accessing conns.
        lock_guard<mutex> lock(connections_mutex);
        for (auto [uname, client] : conns)
            client->send_text("{\"username\": \"system\", \"message\": \"New user "+username+" has joined\"}");
        
        // Welcome aboard matey
        conns.push_back(pair<string, websocket::connection*>(username, &conn));

        std::hash<std::string> hasher;
        RGB rgb = hex_to_rgb(
            hasher(
                "system"
            ) % 0xFFFFFF
        );
        
        cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+"system"+"\033[0m]> " << "New user "+username+" has joined" << "\n";

    })

    .onmessage([](websocket::connection& conn, const string& data, bool is_binary) {
        // We don't want binary data.
        if (is_binary)
            conn.close("{\"error\": \"Invalid format\"}", websocket::CloseStatusCode::UnacceptableData);

        auto json_body = json::load(data);
        if (!json_body.has("message"))
            // We use message commands.
            conn.close("{\"error\": \"A message is required\"}", websocket::CloseStatusCode::UnacceptableData);
        
        // The actual message they're sending.
        string msg = static_cast<string>(json_body["message"].s());

        // Lock up tight because we're accessing conns, 
        lock_guard<mutex> lock(connections_mutex);

        // Command that gets all memebrs. This should generally
        // be used for the implementation of member lists in GUI clients.
        if (msg == ":members"){
            string mems = "{\"membercount\": [";
            for (auto [uname, sess] : conns)
                mems += "\""+uname+"\",";
            
            // This can fail if conns is empty, but this condition will never trigger.
            // As there's no way to send a message if yoy're not connected. I don't
            // care about this condition and won't check for it.
            mems[mems.size()-1] = ']';
            mems += "}";
            
            conn.send_text(mems);
            return;
        }

        // Client changes their username.
        else if (msg.starts_with(":username")){
            // Size of ":username " is 10, so it checks if there is
            // the command and a leading shitespace after the command.
            if (msg.size()<10){
                conn.send_text("{\"error\": \"No username given\"}");
                return;
            }

            // Since it's len 10, that minus 1 for the last index
            // would be 9.
            string username = msg.substr(9,msg.size()-1);

            // Server is reserved for serverside messages and client
            // is reserved for clientside messages.
            if (username == "server" || username == "client") {
                conn.send_text("{\"error\": \"Username reserved\"}");
                return;
            };

            // Check if the username is taken otherwise before assigning it.
            for (auto [uname, client] : conns)
                if (username == uname) {
                    conn.send_text("{\"error\": \"Username taken\"}");
                    return;
                }
            
            // We can't get the connection from the username, so we have to look it up
            // manually. Maybe later we could use a hash map or some shit? Could be more efficient. @NOTE
            for (auto& [uname, client] : conns){
                if (client == &conn) {
                    uname = msg.substr(10);
                    break;
                }
            }
        }
        // The name of the person who changed their name as a variable outside the loop.
        string username;
        
        for (auto& [uname, client] : conns) {
            if (client == &conn) {
                username = uname;
                break;
            }
        }       
        
        // Send the message, regardless of it's a command or not.
        for (auto& [uname, client] : conns) {
            client->send_text("{\"username\": \""+username+"\", \"message\": \""+msg+"\"}");
        }
        std::hash<std::string> hasher;
        RGB rgb = hex_to_rgb(
            hasher(
                username
            ) % 0xFFFFFF
        );
        
        cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+username+"\033[0m]> " << msg << "\n";
    })
    // When they leave or disconnect themselves. Or we disconnect them I think?
    .onclose([&](websocket::connection& conn, const string& reason, uint16_t status_code) {
        // Lock it up tight cuz we're accessing conns.
        lock_guard<mutex> lock(connections_mutex);
        
        // Username of the client.
        string username;
        bool found = false;

        for (auto& [uname, client] : conns) {
            if (client == &conn) {
                username = uname;
                found = true;
                break;
            }
        }

        if (!found) {
            return;
        }
        
        // Get them out of the list of clients.
        conns.erase(
            remove(
                conns.begin(),
                conns.end(),
                pair<
                    string,
                    websocket::connection*
                >(
                    username, &conn
                )
            ),
            conns.end()
        );
        // Hear ye, hear ye! The client has disconnected!
        for (auto [uname, client] : conns)
            client->send_text("{\"username\": \"system\", \"message\": \"Member "+username+" has left\"}");

        std::hash<std::string> hasher;
        RGB rgb = hex_to_rgb(
            hasher(
                "system"
            ) % 0xFFFFFF
        );
        
        cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+"system"+"\033[0m]> " << "Member "+username+" has left" << "\n";

    });

    auto shutup = async(launch::async, [&]() {
        string msg;

        std::hash<std::string> hasher;
        RGB rgb = hex_to_rgb(
            hasher(
                "system"
            ) % 0xFFFFFF
        );
        
        while (getline(cin, msg)) {
            nlohmann::json payload;
            payload["message"] = msg;
            payload["username"] = "system";

            string serialized = payload.dump();
            for (auto& [uname, client] : conns) {
                client->send_text(serialized);
            }
            
            cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+"system"+"\033[0m]> " << msg << "\n";
            if (msg.substr(0,6) == ":kick " && msg.size() > 7){
                bool found = false;
                for (auto& [uname, client] : conns) {
                    if (uname == msg.substr(6)){
                        client->close(
                            "{\"username\": \"system\", \"message\": \"You have been kicked from the server\"}",
                            websocket::CloseStatusCode::NormalClosure
                        );
                        found = true;
                        break;
                    }
                }
                if (!found)
                    // An abnormal condition here.
                    cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+"system"+"\033[0m]> " << "The requested user was not found." << "\n";
            }
            
        }
    });
    
    // @TODO Add support for TLS.
    std::string arg1(argv[1]); 
    app
        .port(stoi(arg1))
        #ifdef SSL_ON
        .ssl_file(SSL_CERT, SSL_KEY)
        #endif
        .bindaddr(argv[2])
        .multithreaded().run();
}
