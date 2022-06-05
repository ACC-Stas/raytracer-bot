#pragma once
#include "client.h"
#include <memory>

class IBot {
public:
    virtual void Run() = 0;

};

struct Message;

class StanislavushkaBot : public IBot {
public:
    StanislavushkaBot();

    void Run();

private:
    void ProcessMessage(const Message& message, bool* stop_cycle);

    std::unique_ptr<Client> client_;
    const std::string url_ = "https://api.telegram.org";
    const std::string token_ = "2137738749:AAGCJ1MwktQyJMwYncEcZLlu2297BxShI3g";
};
