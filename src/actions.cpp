#include "actions.hpp"

using namespace package;
using namespace std;

#include <vector>
#include <arpa/inet.h>
#include <stdlib.h>
#include "sql.hpp"
#include "common/StringTools.hpp"

/*
 * 在已经通过认证的情况下进行的操作
 * 将数据存入数据库
 */
void action::authen_response_action(const package::authentication_response &ar)
{
    
}