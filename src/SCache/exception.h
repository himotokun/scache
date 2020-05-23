#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

class Exception{
	std::string msg;
public:
	Exception(std::string _msg):
		msg(_msg)
	{}
	std::string GetMessage() const{ return msg; }
};

#endif
