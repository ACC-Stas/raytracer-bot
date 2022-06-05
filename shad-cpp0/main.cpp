#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/JSON/Parser.h>
#include "bot/telegram/bot.h"

int main() {
    StanislavushkaBot bot = StanislavushkaBot();
    bot.Run();
    return 0;
}