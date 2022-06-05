#include "catch.hpp"
#include <telegram/client.h>
#include "telegram/fake.h"

TEST_CASE("Single getMe") {
    telegram::FakeServer fake("Single getMe");
    fake.Start();
    std::string host = fake.GetUrl();
    std::string token = "123";
    Client client(host, token);
    client.CheckToken("123");
    fake.StopAndCheckExpectations();
}

TEST_CASE("getMe error handling") {
    telegram::FakeServer fake("getMe error handling");
    fake.Start();
    std::string host = fake.GetUrl();
    std::string token = "123";

    try {
        Client client(host, token);
        client.CheckToken("123");
        REQUIRE(false);
    } catch (const std::exception& exception) {
        REQUIRE(true);
    }
    try {
        Client client(host, token);
        client.CheckToken("123");
        REQUIRE(false);
    } catch (const std::exception& exception) {
        REQUIRE(true);
    }
}

TEST_CASE("Single getUpdates and send messages") {
    telegram::FakeServer fake("Single getUpdates and send messages");
    fake.Start();
    std::string host = fake.GetUrl();
    std::string token = "123";
    Client client(host, token);

    std::vector<Message> messages = client.GetMessages();
    Message input;
    Message output;
    input = messages[0];
    output.chat_id = input.chat_id;
    output.text = "Hi!";
    client.SendMessage(output);

    input = messages[1];
    output.chat_id = input.chat_id;
    output.text = "Reply";
    output.message_id = input.message_id;
    client.SendMessage(output);
    client.SendMessage(output);

    fake.StopAndCheckExpectations();
}

TEST_CASE("Handle getUpdates offset") {
    telegram::FakeServer fake("Handle getUpdates offset");
    fake.Start();
    std::string host = fake.GetUrl();
    std::string token = "123";
    Client client(host, token);

    client.SetTimeout(5);
    std::vector<Message> messages = client.GetMessages();
    REQUIRE(messages.size() == 2);
    messages = client.GetMessages();  // offset is handled automatically
    REQUIRE(messages.empty());
    messages = client.GetMessages();
    REQUIRE(messages.size() == 1);
    fake.StopAndCheckExpectations();
}
