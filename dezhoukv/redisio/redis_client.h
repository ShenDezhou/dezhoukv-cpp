#ifndef _REDIS_TOOL_H
#define _REDIS_TOOL_H

#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <sys/time.h>
#include <hiredis/hiredis.h>

class RedisAdapter{
public:
	
	RedisAdapter(std::string Address, int Port)
	{
		address = Address;
		port = Port;
		pRedisContext = NULL;
		connected = false;
		// password = code;
	}

	inline void SetCode(std::string code) {
		password = code;
	}
		
	int Open();
	int Connect();
	int Close();
	
	// key value function
	int Set(std::string key, std::string value);
	int Get(std::string key, std::string &value);
	
	//int Get(gDocID_t *key, size_t keySize, PageData &value);	
	// golbal function
	int Delete(std::string key);      

private:
	std::string address;
	int port;
	redisContext *pRedisContext;  
	bool connected;
	std::string password;
};

#endif

