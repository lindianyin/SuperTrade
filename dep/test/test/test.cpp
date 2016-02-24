#if 1
#include <winsock.h>
#include "mysql.h"
#include <cstdio>
int main(int argc, char* argv[])
{
	MYSQL *pMysql = new MYSQL();
	mysql_init(pMysql);
	mysql_real_connect(pMysql,"183.136.221.50","root","gywl2013","booktest",3306,NULL,0);
	printf(mysql_error(pMysql));
 	return 0;
}
#endif
