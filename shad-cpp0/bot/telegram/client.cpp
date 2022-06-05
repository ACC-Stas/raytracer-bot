#include <Poco/URI.h>
#include "Poco/URIStreamOpener.h"
#include <Poco/JSON/Parser.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include <Poco/Net/FTPStreamFactory.h>
#include <Poco/Net/FilePartSource.h>
#include <memory>
#include <string>
#include <filesystem>
#include <stdexcept>
#include <fstream>
#include "client.h"
#include "exceptions.h"
#include "../../raytracer/raytracer.h"

struct RaytracerInput {
    bool valid = true;
    CameraOptions camera_options = CameraOptions(640, 640);
    RenderOptions render_options;
    std::string filename;
};

RaytracerInput ParseRaytracerInput(const Message& message) {
    RaytracerInput result;
    std::regex raytracer(R"(^\/render\s+(.+\.obj)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(-?\d+\.?\d*)\s+(\d+)\s+(.+))");
    std::smatch match;
    std::string current_name;

    result.valid = false;
    if (std::regex_search(message.text, match, raytracer)) {
        result.valid = true;
        result.filename = match[1];
        result.camera_options.look_from = std::array<double, 3>{std::stod(match[2]), std::stod(match[3]), std::stod(match[4])};
        result.camera_options.look_to = std::array<double, 3>{std::stod(match[5]), std::stod(match[6]), std::stod(match[7])};
        result.render_options.depth = std::stoi(match[8]);

        if (match[9] == "full") {
            result.render_options.mode = RenderMode::kFull;
        } else if (match[9] == "norm") {
            result.render_options.mode = RenderMode::kNormal;
        } else if (match[9] == "depth") {
            result.render_options.mode = RenderMode::kDepth;
        } else {
            result.valid = false;
        }
    }
    return result;
}

Client::Client(const std::string& url, const std::string& token) {
    path_ = "/bot" + token + "/";
    Poco::URI uri(url);
    if (uri.getScheme() == "http") {
        session_ = std::make_unique<Poco::Net::HTTPClientSession>(uri.getHost(), uri.getPort());
    } else if (uri.getScheme() == "https") {
        session_ = std::unique_ptr<Poco::Net::HTTPClientSession>(
            new Poco::Net::HTTPSClientSession(uri.getHost(), uri.getPort()));
    } else {
        throw std::runtime_error("There is no such scheme as" + uri.getHost());
    }

    Poco::Net::HTTPSStreamFactory::registerFactory(); // Must register the HTTP factory to stream using HTTP
    Poco::Net::FTPStreamFactory::registerFactory(); // Must register the FTP factory to stream using FTP
}

Client::Client(const std::string& url, const std::string& token, const std::string& offset_filename)
    : Client(url, token) {
    offset_filename_ = offset_filename;
    std::ifstream input(offset_filename);
    input >> offset_;
    input.close();
}

void Client::CheckToken(const std::string& token) {
    Poco::URI uri(path_ + "getMe");

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.toString());
    session_->sendRequest(request);
    Poco::Net::HTTPResponse response;
    std::istream& input_body_stream = session_->receiveResponse(response);

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {
        throw HTTPError(response.getStatus(),
                        std::to_string(response.getStatus()) + " " + response.getReason());
    }

    Poco::JSON::Parser parser;
    Poco::Dynamic::Var parsed_result = parser.parse(input_body_stream);
    std::string string = parsed_result.toString();
    auto result_condition = parsed_result.extract<Poco::JSON::Object::Ptr>()->get("ok");
    if (result_condition.toString() != "true") {
        throw InvalidTokenError(token, token + " is invalid");
    }
}

std::vector<Message> Client::GetMessages() {
    Poco::URI uri(path_ + "getUpdates");
    if (offset_ > 0) {
        uri.addQueryParameter("offset", std::to_string(offset_));
    }
    if (timeout_ > 0) {
        uri.addQueryParameter("timeout", std::to_string(timeout_));
    }

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.toString());
    session_->sendRequest(request);
    Poco::Net::HTTPResponse response;
    std::istream& input_body_stream = session_->receiveResponse(response);

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {
        throw HTTPError(response.getStatus(),
                        std::to_string(response.getStatus()) + " " + response.getReason());
    }

    Poco::JSON::Parser parser;
    Poco::Dynamic::Var parsed_result = parser.parse(input_body_stream);
    auto result_condition = parsed_result.extract<Poco::JSON::Object::Ptr>()->get("ok");
    if (result_condition.toString() != "true") {
        throw std::runtime_error("message error");
    }

    auto raw_messages = parsed_result.extract<Poco::JSON::Object::Ptr>()->getArray("result");
    std::vector<Message> messages;
    for (const auto& raw_message : *raw_messages) {
        std::string temp = raw_message.toString();
        std::cout << temp << "\n\n\n";
        messages.push_back(ParseMessage(raw_message));
    }
    if (messages.empty()) {
        return messages;
    }
    offset_ = messages.back().update_id + 1;
    SaveOffset();
    return messages;
}

Message Client::ParseMessage(const Poco::Dynamic::Var& input) {
    Message message;
    message.update_id = input.extract<Poco::JSON::Object::Ptr>()->getValue<int>("update_id");
    if (!input.extract<Poco::JSON::Object::Ptr>()->has("message")) {
        return message;
    }

    Poco::Dynamic::Var raw_message = input.extract<Poco::JSON::Object::Ptr>()->get("message");
    if (raw_message.extract<Poco::JSON::Object::Ptr>()->has("text")) {
        message.text = raw_message.extract<Poco::JSON::Object::Ptr>()->get("text").toString();
    }
    if (raw_message.extract<Poco::JSON::Object::Ptr>()->has("message_id")) {
        message.message_id =
            raw_message.extract<Poco::JSON::Object::Ptr>()->getValue<int>("message_id");
    }

    if (raw_message.extract<Poco::JSON::Object::Ptr>()->has("entities")) {
        auto raw_entities = raw_message.extract<Poco::JSON::Object::Ptr>()->getArray("entities");
        auto raw_entity = (*raw_entities).get(0);
        int offset = raw_entity.extract<Poco::JSON::Object::Ptr>()->getValue<int>("offset");
        int length = raw_entity.extract<Poco::JSON::Object::Ptr>()->getValue<int>("length");
        message.type = raw_entity.extract<Poco::JSON::Object::Ptr>()->get("type").toString();
        message.command = message.text.substr(offset, length);
    } else {
        message.type = "regular_message";
    }

    if (raw_message.extract<Poco::JSON::Object::Ptr>()->has("document")) {
        auto raw_document = raw_message.extract<Poco::JSON::Object::Ptr>()->get("document");
        message.type = "input_file";
        message.text = raw_document.extract<Poco::JSON::Object::Ptr>()->getValue<std::string>("file_id");
        message.filename = raw_document.extract<Poco::JSON::Object::Ptr>()->getValue<std::string>("file_name");
    }

    Poco::Dynamic::Var raw_chat = raw_message.extract<Poco::JSON::Object::Ptr>()->get("chat");
    message.chat_id = raw_chat.extract<Poco::JSON::Object::Ptr>()->getValue<int>("id");
    return message;
}

void Client::SendMessage(const Message& message) {
    Poco::URI uri(path_ + "sendMessage");

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery());
    Poco::JSON::Object obj;
    obj.set("chat_id", std::to_string(message.chat_id));
    obj.set("text", message.text);
    if (message.message_id > 0) {
        obj.set("reply_to_message_id", std::to_string(message.message_id));
    }
    std::stringstream ss;
    obj.stringify(ss);

    request.setContentType("application/json");
    request.setContentLength(ss.str().size());

    std::ostream& my_o_stream = session_->sendRequest(request);
    obj.stringify(my_o_stream);

    Poco::Net::HTTPResponse response;
    std::istream& input_body_stream = session_->receiveResponse(response);

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {

        Poco::JSON::Parser parser;
        Poco::Dynamic::Var parsed_result = parser.parse(input_body_stream);

        std::cout << parsed_result.toString() << "\n\n\n";

//        throw HTTPError(response.getStatus(),
//                        std::to_string(response.getStatus()) + " " + response.getReason());
    }
}

void Client::SetTimeout(int timeout) {
    if (timeout < 0) {
        throw std::runtime_error("invalid timeout");
    }
    timeout_ = timeout;
}

void Client::SaveOffset() {
    if (offset_filename_.empty()) {
        return;
    }
    std::ofstream output(offset_filename_);
    output << offset_;
    output.close();
}

void Client::SetOffset(int offset) {
    offset_ = offset;
}

std::string Client::SaveFile(const Message& message) {
    Poco::URI uri(path_ + "getFile?file_id=" + message.text);
    if (offset_ > 0) {
        uri.addQueryParameter("offset", std::to_string(offset_));
    }
    if (timeout_ > 0) {
        uri.addQueryParameter("timeout", std::to_string(timeout_));
    }

    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.toString());
    session_->sendRequest(request);
    Poco::Net::HTTPResponse response;
    std::istream& input_body_stream = session_->receiveResponse(response);

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {
        throw HTTPError(response.getStatus(),
                        std::to_string(response.getStatus()) + " " + response.getReason());
    }

    Poco::JSON::Parser parser;
    Poco::Dynamic::Var parsed_result = parser.parse(input_body_stream);

    std::cout << parsed_result.toString() << "\n\n\n";

    auto result_condition = parsed_result.extract<Poco::JSON::Object::Ptr>()->get("ok");
    if (result_condition.toString() != "true") {
        throw std::runtime_error("message error");
    }

    auto raw_message = parsed_result.extract<Poco::JSON::Object::Ptr>()->get("result");
    std::string temp = raw_message.extract<Poco::JSON::Object::Ptr>()->getValue<std::string>("file_path");
    std::cout << temp << "\n\n\n";

    std::string url = "https://api.telegram.org/file";
    url += path_;
    url += temp;
    std::string file_path = std::filesystem::current_path().string() + "/" + message.filename;
    std::cout << file_path << "\n";

    // Create and open a file stream
    std::ofstream file_stream;
    file_stream.open(file_path, std::ios::out | std::ios::trunc | std::ios::binary);

    // Create the URI from the URL to the file.
    std::cout << url << "\n";
    std::cout << std::filesystem::current_path() << "\n";
    Poco::URI file_uri(url);

    // Open the stream and copy the data to the file.
    try {
        std::unique_ptr<std::istream> p_str(Poco::URIStreamOpener::defaultOpener().open(file_uri));
        Poco::StreamCopier::copyStream(*p_str.get(), file_stream);
    } catch (Poco::ExistsException& e) {
        return "File already loaded";
    }

    file_stream.close();

    return "Loaded";
}


std::string Client::UploadImage(const Message& message) {
    Poco::URI uri(path_ + "sendPhoto");
    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, uri.getPathAndQuery());

    Poco::Net::HTMLForm form;
    form.setEncoding(Poco::Net::HTMLForm::ENCODING_MULTIPART);
    form.set("chat_id", std::to_string(message.chat_id));

    RaytracerInput input = ParseRaytracerInput(message);
    if (!input.valid) {
        return "Invalid input";
    }

    Image result = Render(input.filename, input.camera_options, input.render_options);


    result.Write(std::filesystem::current_path().string() + "/DATA/" + "result.png");

    form.addPart("photo", new Poco::Net::FilePartSource(std::filesystem::current_path().string() + "/DATA/" + "result.png"));
    form.prepareSubmit(request);
    form.write(session_->sendRequest(request));

    Poco::Net::HTTPResponse response;
    std::istream& input_body_stream = session_->receiveResponse(response);

    if (response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_OK) {
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var parsed_result = parser.parse(input_body_stream);

        std::cout << parsed_result.toString() << "\n\n\n";
        throw HTTPError(response.getStatus(),
                        std::to_string(response.getStatus()) + " " + response.getReason());
    }

    return "Uploaded";
}
