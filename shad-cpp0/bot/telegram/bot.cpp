#include <Poco/URI.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/JSON/Parser.h>
#include <memory>
#include <string>
#include "client.h"
#include "bot.h"

void StanislavushkaBot::Run() {
    std::unique_ptr<bool> need_to_stop(new bool);
    *need_to_stop = false;
    while (true) {
        std::vector<Message> messages = client_->GetMessages();
        for (const auto& message : messages) {
            ProcessMessage(message, need_to_stop.get());
            if (*need_to_stop) {
                break;
            }
        }
        if (*need_to_stop) {
            break;
        }
    }
}

StanislavushkaBot::StanislavushkaBot() {
    client_ = std::unique_ptr<Client>(
        new Client(url_, token_, "/home/stanislav/shad-cpp0/bot/telegram/offset.txt"));
    client_->CheckToken(token_);
    client_->SetTimeout(30);
}

void StanislavushkaBot::ProcessMessage(const Message& message, bool* stop_cycle) {
    Message output;
    output.chat_id = message.chat_id;

    if (message.type == "input_file") {
        output.text = client_->SaveFile(message);
        client_->SendMessage(output);
        return;
    }

    if (message.type != "bot_command") {
        output.text = "is not a command";

    } else if (message.command == "/random") {
        output.text = "42";

    } else if (message.command == "/weather") {
        output.text = "Winter Is Coming";

    } else if (message.command == "/styleguide") {
        output.text = "Some funny joke about style guide";
    } else if (message.command == "/crash") {
        throw std::runtime_error("crash will of user");
    } else if (message.command == "/stop") {
        output.text = "It is time to stop";
        *stop_cycle = true;
    } else if (message.command == "/render") {
        output.text = client_->UploadImage(message);
    } else {
        output.text = "Don't know such command";
    }
    client_->SendMessage(output);
}
