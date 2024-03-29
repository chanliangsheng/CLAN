#ifndef _BASE64_H_
#define _BASE64_H_

#include <string>
#include <vector>
typedef unsigned char BYTE;

std::string base64_encode(BYTE const* buf, unsigned int bufLen);
std::string base64_decode(std::string const&);

#endif
