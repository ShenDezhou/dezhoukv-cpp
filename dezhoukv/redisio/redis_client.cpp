#include "time.h"
#include "stdlib.h" 
#include "redis_client.h"

//带密码验证的连接建立
int RedisAdapter::Open()
{
	connected = false;
	if (pRedisContext != NULL)
	{
		redisFree(pRedisContext);
		pRedisContext = NULL;
	}
	struct timeval timeout = {2, 0}; 
	pRedisContext = (redisContext*)redisConnectWithTimeout(address.c_str(), port, timeout);
	if ( (NULL == pRedisContext) || (pRedisContext->err)  )
	{
		redisFree(pRedisContext);
		pRedisContext = NULL;
		return -1;
	}
  
//	redisReply *pRedisReply = (redisReply*)redisCommand(pRedisContext, "AUTH %s", password.c_str() ); 
//	if (pRedisReply == NULL)
//		return -1;
//	int ret = -1;
//	if (pRedisReply->type == REDIS_REPLY_ERROR	&& strcmp(pRedisReply->str, "ERR Client sent AUTH, but no password is set") == 0)
//	{
//		ret = 0;
//		connected = true;
//	}

//	else if (pRedisReply->type != REDIS_REPLY_STATUS)
//	{
//		ret = -1;
//	}
//	else if (strcmp(pRedisReply->str, "OK") == 0)
//	{
//		ret = 0;
//		connected = true;
//	}
//	else
//	{
//		ret = -1;
//	}
  
//	freeReplyObject(pRedisReply);
//  if (strlen(password.c_str()) ==0) {
//   connected = true;
//    return 0;
//  }
	return 0;
}

int RedisAdapter::Close()
{
	if (pRedisContext != NULL)
	{
		redisFree(pRedisContext);
		pRedisContext = NULL;
	}
	connected = false;
	return 0;
}

int RedisAdapter::Set(std::string key, std::string value)
{
	if (!pRedisContext) return -1;
	redisReply *pRedisReply = (redisReply*)redisCommand(pRedisContext, "SET %b %b", key.c_str(), key.size(), value.c_str(), value.size()); 
	if (pRedisReply == NULL)
		return -1;
	int ret = -1;
	if (pRedisReply->type != REDIS_REPLY_STATUS) {
      ret = -1;
  }
  else if (strcmp(pRedisReply->str, "OK") == 0) {
      ret = 1;
  }
  else {
      ret = -1;
  }
	freeReplyObject(pRedisReply);
	return ret;
}

// int RedisAdapter::Set(std::string key, std::string value)
// {
//     if (!pRedisContext) return -1;  
//     std::string command="SET ";
//     std::string parameter = key;
//     char *commandStr = new char[value.length()+1];
//     memset(commandStr, 0, value.length() + 1);
//     memcpy(commandStr, value.c_str(), value.length());
//     const char **commandList = new const char*[3];
//     commandList[0] = command.c_str();
//     commandList[1] = parameter.c_str();
//     commandList[2] = commandStr;
//     size_t *lengthList = new size_t[3];
//     lengthList[0]=3;
//     lengthList[1]=key.length();
//     lengthList[2]=value.length();
    
//     redisReply *pRedisReply = (redisReply*)redisCommandArgv(pRedisContext, 3, (const char**)commandList, (const size_t*)lengthList);
        
//     //fprintf(stderr, "RedisAdapter::Set valueis[%s] length2[%d]\n", SS_TO_ASCIIX((bchar_t*)commandList[2]), lengthList[2]);
//     if (pRedisReply == NULL){
//         fprintf(stderr, "RedisAdapter::Set NULL!\n");
//         delete [] commandStr;
//         commandStr = NULL;
//         delete [] commandList;
//         commandList = NULL;
//         delete [] lengthList;
//         lengthList = NULL;
//             return -1;
//     }
//     int ret = -1;
//     if (pRedisReply->type != REDIS_REPLY_STATUS)
//     {
//         ret = -1;
//     }
//     else if (strcmp(pRedisReply->str, "OK") == 0)
//     {
//         ret = 1;
//     }
//     else
//     {
//         ret = -1;
//     }
//     freeReplyObject(pRedisReply);
//     delete [] commandStr;
//     commandStr = NULL;
//     delete [] commandList;
//     commandList = NULL;
//     delete [] lengthList;
//     lengthList = NULL;
//     return ret;
// }

int RedisAdapter::Get(std::string key, std::string &value)
{
	if (!pRedisContext) return -1;
	redisReply *pRedisReply = (redisReply*)redisCommand(pRedisContext, "GET %b", key.c_str(), key.size() ); 
	if (pRedisReply == NULL)
		return -1;
	int ret = -1;
	if (pRedisReply->type == REDIS_REPLY_NIL)
	{
		ret = 0;
	}
	else if (pRedisReply->type == REDIS_REPLY_STRING)
	{
		value.assign(pRedisReply->str,  pRedisReply->len);
		ret = 1;
	}	
	else
	{
		ret = -1;
	}
	freeReplyObject(pRedisReply);
	return ret;
}

int RedisAdapter::Delete(std::string key)
{
	if (!pRedisContext) 
		return -1;
		redisReply *pRedisReply = (redisReply*)redisCommand(pRedisContext, "DEL %s", key.c_str() ); 
	if (pRedisReply == NULL)
		return -1;
	int ret = -1;
		if (pRedisReply->type == REDIS_REPLY_NIL)
		{
			ret = 0;
		}
		else if (pRedisReply->type == REDIS_REPLY_INTEGER)
		{
			ret = pRedisReply->integer;
		}		
		else
		{
			ret = -1;
		}
	freeReplyObject(pRedisReply);
	return ret;

}

/*int main()
{
	RedisAdapter *redis = new RedisAdapter("127.0.0.1", 6379);
	int ret = redis->Connect();
	std::cout << ret << std::endl;
	std::string value;
	ret = redis->Set("test", "yy");
	std::cout << ret << std::endl;
	ret = redis->Set("test", "yy9");
	std::cout << ret << std::endl;
	ret = redis->Get("test", value);
	std::cout << ret << std::endl;
	std::cout << value << std::endl;
	ret = redis->Delete("test");
	std::cout << ret << std::endl;
	redis->Close();
	return 0;
}*/
