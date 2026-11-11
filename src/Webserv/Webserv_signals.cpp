#include "Webserv.hpp"

volatile sig_atomic_t Webserv::_flag_sigint = 0;

void Webserv::handleSigint(int)
{
    Webserv::_flag_sigint = 1;
}