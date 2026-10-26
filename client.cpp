// Hello! Made by Ainselu, on the night of October 25th, 2026.
// Undergoing development. It's pretty cool but it's pretty mid.
// @TODO ­— Note on something to implement later.
// @NOTE — General comment or suggestion to look over.
// CTRL-F them if you want to see those.


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <cstdlib>
#include <future>
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include "rgb.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace asio = boost::asio;
using tcp = boost::asio::ip::tcp;
using namespace std;

int main(int argc, char *argv[]) {
    try {
        if (argc != 4){
            cout << "HiveChat takes 3 arguments (port, address, and password)\n";
            return -1;
        }
        string const port = string(argv[1]);
        string const host = string(argv[2]);
        string const password = string(argv[3]);

        asio::io_context ioc;

        tcp::resolver resolver(ioc);
        websocket::stream<tcp::socket> ws(ioc);

        auto const results = resolver.resolve(host, port);

        asio::connect(ws.next_layer(), results.begin(), results.end());

        // ToHive is the name of the simple client.
        ws.set_option(websocket::stream_base::decorator(
            [password](websocket::request_type& req) {
                req.set(http::field::user_agent, "ToHive");
                req.set(http::field::authorization, password);
            }));
        boost::system::error_code ec;
        websocket::response_type response;

        ws.handshake(response, host, "/hivep2p", ec);

        cout << "Handshake message: " << ec.message() << '\n';
        cout << "HTTP status: "
                << response.result_int() << ' '
                << response.reason() << '\n';
        
        if (!response.body().empty())
            cout << "Response body: " << response.body() << '\n';

        if (ec) return EXIT_FAILURE;

        cout << "[client]> You have joined the server.\n";
        // These are only named shutup because otherwise clangd will
        // highlight every line of the function in blue. That's annoying.
        // It's not directed at you, the reader of my fine code, but my
        // dumb ass language server.
        auto shutup = async(launch::async, [&](){
            while (true){
                beast::flat_buffer buffer;

                ws.read(buffer);
                // @TODO Display error messages and user membership if requested.
                if (ws.got_text()){
                    auto json = nlohmann::json::parse(
                        beast::buffers_to_string(buffer.data())
                    );
                    if (json.contains("message") && json.contains("username")){
                        std::hash<std::string> hasher;
                        RGB rgb = hex_to_rgb(
                            hasher(
                                json["username"].get<string>()
                            ) % 0xFFFFFF
                        );
                        
                        cout << "[\033[38;2;"+to_string(rgb.r)+";"+to_string(rgb.g)+";"+to_string(rgb.b)+"m"+json["username"].get<string>()+"\033[0m]> " << json["message"].get<string>() << "\n";
                    }

                    if (json.contains("error"))
                        cout << "[\033[31merror\033[0m]> " << json["error"].get<string>() << "\n";
                }
            }
        });

        auto shutup2 = async(launch::async, [&]() {
            string msg;

            while (getline(cin, msg)) {
                nlohmann::json payload;
                payload["message"] = msg;

                string serialized = payload.dump();
                ws.write(asio::buffer(serialized));
            }
        });

    } catch (exception const& e) {
        cerr << "Error: " << e.what() << endl;

        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

