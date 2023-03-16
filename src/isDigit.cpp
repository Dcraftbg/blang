#pragma once
#include <string>
/*
Stolen from stack overflow:
https://stackoverflow.com/questions/4654636/how-to-determine-if-a-string-is-a-number-with-c
*/
bool mis_number(const std::string s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}