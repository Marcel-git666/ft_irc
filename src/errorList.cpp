#include "../inc/Server.hpp"

std::string Server::GetErrorStr(const std::string &args, int errorNumber) {
  switch (errorNumber) {
  case (401):
    return (":server 401 * " + args + " :No such nick/channel\r\n");
  case (403):
    return (":server 403 * " + args + " :No such channel\r\n");
  case (433):
    return (":server 433 * " + args + " :Nickname is already in use\r\n");
  case (432):
    return (":server 432 * " + args + " :Erroneus nickname\r\n");
  case (441):
    return (":server 441 " + args + " :They aren't on that channel\r\n");
  case (442):
    return (":server 442 * " + args + " :You're not on that channel\r\n");
  case (443):
    return (":server 443 " + args + " :is already on channel\r\n");
  case (464):
    return (":server 464 * :Password incorrect\r\n");
  case (461):
    return (":server 461 " + args + " :Not enough parameters\r\n");
  case (462):
    return (":server 462 * :You may not reregister\r\n");
  case (471):
    return (":server 471 * " + args + " :Cannot join channel (+l)\r\n");
  case (472):
    return (":server 472 " + args + " :is unknown mode char to me\r\n");
  case (473):
    return (":server 473 * " + args + " :Cannot join channel (+i)\r\n");
  case (475):
    return (":server 475 * " + args + " :Cannot join channel (+k)\r\n");
  case (482):
    return (":server 482 * " + args + " :You're not channel operator\r\n");
  }
  return ("");
}
