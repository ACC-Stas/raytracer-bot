#pragma once
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/JSON/JSON.h>
#include <vector>
#include <memory>

struct Message {
public:
    int chat_id;
    int update_id;
    int message_id = -1;
    std::string type;
    std::string text;
    std::string command;
    std::string filename;
};

class Client {
public:
    Client(const std::string& url, const std::string& token);
    Client(const std::string& url, const std::string& token, const std::string& offset_filename);
    std::vector<Message> GetMessages();
    void SendMessage(const Message& message);
    void SetTimeout(int timeout);
    void SetOffset(int offset);
    void CheckToken(const std::string& token);
    std::string SaveFile(const Message& message);
    std::string UploadImage(const Message& message);
private:
    Message ParseMessage(const Poco::Dynamic::Var& input);
    void SaveOffset();
    std::string path_;
    int offset_ = -1;
    int timeout_ = -1;
    std::string offset_filename_;
    std::unique_ptr<Poco::Net::HTTPClientSession> session_;
};
