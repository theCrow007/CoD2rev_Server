#include "gsc_mysql.hpp"

#if LIBCOD_COMPILE_MYSQL == 1

#include "mysql/include/mysql.h"
#include <cstring>
#include <cstdlib>

// Default connection for zk-style calls (without handle)
static MYSQL *default_zk_connection = NULL;

struct mysql_async_task
{
	mysql_async_task *prev;
	mysql_async_task *next;
	int id;
	MYSQL_RES *result;
	bool done;
	bool started;
	bool save;
	char query[COD2_MAX_STRINGLENGTH + 1];
};

struct mysql_async_connection
{
	mysql_async_connection *prev;
	mysql_async_connection *next;
	mysql_async_task* task;
	MYSQL *connection;
};

mysql_async_connection *first_async_connection = NULL;
mysql_async_task *first_async_task = NULL;
MYSQL *cod_mysql_connection = NULL;

void *mysql_async_execute_query(void *input_c) //cannot be called from gsc, is threaded.
{
	mysql_async_connection *c = (mysql_async_connection *) input_c;
	int res = mysql_query(c->connection, c->task->query);
	if(!res && c->task->save)
		c->task->result = mysql_store_result(c->connection);
	else if(res)
	{
		//mysql show error here?
	}
	c->task->done = true;
	c->task = NULL;
	return NULL;
}

void *mysql_async_query_handler(void* input_nothing) //is threaded after initialize
{
	static bool started = false;
	if(started)
	{
		Com_DPrintf("mysql_async_query_handler() async handler already started. Returning\n");
		return NULL;
	}
	started = true;
	mysql_async_connection *c = first_async_connection;
	if(c == NULL)
	{
		Com_DPrintf("mysql_async_query_handler() async handler started before any connection was initialized\n"); //this should never happen
		started = false;
		return NULL;
	}
	mysql_async_task *q;
	while(true)
	{
		Sys_EnterCriticalSection(CRITSECT_MYSQL);
		q = first_async_task;
		c = first_async_connection;
		while(q != NULL)
		{
			if(!q->started)
			{
				while(c != NULL && c->task != NULL)
					c = c->next;
				if(c == NULL)
				{
					//out of free connections
					break;
				}
				q->started = true;
				c->task = q;
				threadid_t query_doer;
				Sys_CreateNewThread(mysql_async_execute_query, &query_doer, c);
				c = c->next;
			}
			q = q->next;
		}
		Sys_LeaveCriticalSection(CRITSECT_MYSQL);
		Sys_SleepMSec(10);
	}
	return NULL;
}

int mysql_async_query_initializer(const char *sql, bool save) //cannot be called from gsc, helper function
{
	static int id = 0;
	id++;
	Sys_EnterCriticalSection(CRITSECT_MYSQL);
	mysql_async_task *current = first_async_task;
	while(current != NULL && current->next != NULL)
		current = current->next;
	mysql_async_task *newtask = new mysql_async_task;
	newtask->id = id;
	strncpy(newtask->query, sql, COD2_MAX_STRINGLENGTH - 1);
	newtask->query[COD2_MAX_STRINGLENGTH - 1] = '\0';
	newtask->prev = current;
	newtask->result = NULL;
	newtask->save = save;
	newtask->done = false;
	newtask->next = NULL;
	newtask->started = false;
	if(current != NULL)
		current->next = newtask;
	else
		first_async_task = newtask;
	Sys_LeaveCriticalSection(CRITSECT_MYSQL);
	return id;
}

void gsc_mysql_async_create_query_nosave()
{
	const char *query;
	if ( ! stackGetParams("s", &query))
	{
		stackError("gsc_mysql_async_create_query_nosave() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}
	int id = mysql_async_query_initializer(query, false);
	stackPushInt(id);
	return;
}

void gsc_mysql_async_create_query()
{
	const char *query;
	if ( ! stackGetParams("s", &query))
	{
		stackError("gsc_mysql_async_create_query() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}
	int id = mysql_async_query_initializer(query, true);
	stackPushInt(id);
	return;
}

void gsc_mysql_async_getdone_list()
{
	Sys_EnterCriticalSection(CRITSECT_MYSQL);
	mysql_async_task *current = first_async_task;
	stackPushArray();
	while(current != NULL)
	{
		if(current->done)
		{
			stackPushInt((int)current->id);
			stackPushArrayLast();
		}
		current = current->next;
	}
	Sys_LeaveCriticalSection(CRITSECT_MYSQL);
}

void gsc_mysql_async_getresult_and_free() //same as above, but takes the id of a function instead and returns 0 (not done), undefined (not found) or the mem address of result
{
	int id;
	if(!stackGetParams("i", &id))
	{
		stackError("gsc_mysql_async_getresult_and_free() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}
	Sys_EnterCriticalSection(CRITSECT_MYSQL);
	mysql_async_task *c = first_async_task;
	if(c != NULL)
	{
		while(c != NULL && c->id != id)
			c = c->next;
	}
	if(c != NULL)
	{
		if(!c->done)
		{
			stackPushUndefined(); //should never happend, query not done yet
			Sys_LeaveCriticalSection(CRITSECT_MYSQL);
			return;
		}
		if(c->next != NULL)
			c->next->prev = c->prev;
		if(c->prev != NULL)
			c->prev->next = c->next;
		else
			first_async_task = c->next;
		if(c->save)
		{
			intptr_t ret = (intptr_t)c->result;
			stackPushInt(ret);
		}
		else
			stackPushInt(0);
		delete c;
		Sys_LeaveCriticalSection(CRITSECT_MYSQL);
		return;
	}
	else
	{
		stackError("gsc_mysql_async_getresult_and_free() mysql async query id not found");
		stackPushUndefined();
		Sys_LeaveCriticalSection(CRITSECT_MYSQL);
		return;
	}
}

void gsc_mysql_async_initializer()//returns array with mysql connection handlers
{
	if(first_async_connection != NULL)
	{
		Com_DPrintf("gsc_mysql_async_initializer() async mysql already initialized. Returning before adding additional connections\n");
		stackPushUndefined();
		return;
	}

	int port, connection_count;
	const char *host, *user, *pass, *db;

	if ( ! stackGetParams("ssssii", &host, &user, &pass, &db, &port, &connection_count))
	{
		stackError("gsc_mysql_async_initializer() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}
	if(connection_count <= 0)
	{
		stackError("gsc_mysql_async_initializer() need a positive connection_count in mysql_async_initializer");
		stackPushUndefined();
		return;
	}
	int i;
	stackPushArray();
	mysql_async_connection *current = first_async_connection;
	for(i = 0; i < connection_count; i++)
	{
		mysql_async_connection *newconnection = new mysql_async_connection;
		newconnection->next = NULL;
		newconnection->connection = mysql_init(NULL);
		newconnection->connection = mysql_real_connect((MYSQL*)newconnection->connection, host, user, pass, db, port, NULL, 0);
		bool reconnect = true;
		mysql_options(newconnection->connection, MYSQL_OPT_RECONNECT, &reconnect);
		newconnection->task = NULL;
		if(current == NULL)
		{
			newconnection->prev = NULL;
			first_async_connection = newconnection;
		}
		else
		{
			while(current->next != NULL)
				current = current->next;
			current->next = newconnection;
			newconnection->prev = current;
		}
		current = newconnection;
		stackPushInt((intptr_t)newconnection->connection);
		stackPushArrayLast();
	}
	threadid_t async_handler;
	Sys_CreateNewThread(mysql_async_query_handler, &async_handler, NULL);
}

// ============================================================
// NEW: zk-style initialization function
// ============================================================
void gsc_mysql_initialize()
{
	const char *host, *user, *pass, *db;
	int port;
	
	if (!stackGetParams("ssssi", &host, &user, &pass, &db, &port))
	{
		stackError("mysql_initialize(host, user, pass, db, port)");
		stackPushUndefined();
		return;
	}
	
	if (default_zk_connection == NULL)
	{
		MYSQL *my = mysql_init(NULL);
		int reconnect = 1;
		mysql_options(my, MYSQL_OPT_RECONNECT, &reconnect);
		
		if (!mysql_real_connect(my, host, user, pass, db, port, NULL, 0))
		{
			stackError(mysql_error(my));
			mysql_close(my);
			stackPushUndefined();
			return;
		}
		
		default_zk_connection = my;
		cod_mysql_connection = my;
	}
	
	stackPushInt((intptr_t)default_zk_connection);
}

// Original rev functions (preserved)
void gsc_mysql_init()
{
	MYSQL *my = mysql_init(NULL);
	stackPushInt((intptr_t)my);
}

void gsc_mysql_reuse_connection()
{
	if(cod_mysql_connection == NULL)
	{
		stackPushUndefined();
		return;
	}
	else
	{
		stackPushInt((intptr_t)cod_mysql_connection);
		return;
	}
}

void gsc_mysql_real_connect()
{
	intptr_t mysql, port;
	const char *host, *user, *pass, *db;

	if ( ! stackGetParams("issssi", &mysql, &host, &user, &pass, &db, &port))
	{
		stackError("gsc_mysql_real_connect() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	mysql = (intptr_t)mysql_real_connect((MYSQL *)mysql, host, user, pass, db, port, NULL, 0);
	bool reconnect = true;
	mysql_options((MYSQL*)mysql, MYSQL_OPT_RECONNECT, &reconnect);
	if(cod_mysql_connection == NULL)
		cod_mysql_connection = (MYSQL*) mysql;
	if(default_zk_connection == NULL)
		default_zk_connection = (MYSQL*) mysql;
	stackPushInt(mysql);
}

void gsc_mysql_close()
{
	int mysql;

	if ( ! stackGetParams("i", &mysql))
	{
		stackError("gsc_mysql_close() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	mysql_close((MYSQL *)mysql);
	if(default_zk_connection == (MYSQL *)mysql)
		default_zk_connection = NULL;
	if(cod_mysql_connection == (MYSQL *)mysql)
		cod_mysql_connection = NULL;
	stackPushInt(0);
}

// Modified: supports both mysql_query(handle, query) AND mysql_query(query)
void gsc_mysql_query()
{
	int num_params = Scr_GetNumParam();
	
	// zk-style: mysql_query(query) - uses default connection
	if (num_params == 1)
	{
		const char *query;
		if (!stackGetParamString(0, &query))
		{
			stackError("mysql_query(query) - string expected");
			stackPushUndefined();
			return;
		}
		
		if (default_zk_connection == NULL)
		{
			stackError("mysql_query(query) - no default connection. Call mysql_initialize() or mysql_real_connect() first.");
			stackPushUndefined();
			return;
		}
		
		stackPushInt(mysql_query(default_zk_connection, query));
		return;
	}
	
	// Original rev style: mysql_query(handle, query)
	if (num_params == 2)
	{
		int mysql;
		const char *query;
		
		if ( ! stackGetParams("is", &mysql, &query))
		{
			stackError("gsc_mysql_query(handle, query)");
			stackPushUndefined();
			return;
		}
		
		int ret = mysql_query((MYSQL *)mysql, query);
		stackPushInt(ret);
		return;
	}
	
	stackError("gsc_mysql_query() expects 1 or 2 arguments");
	stackPushUndefined();
}

void gsc_mysql_errno()
{
	int mysql;

	if ( ! stackGetParams("i", &mysql))
	{
		stackError("gsc_mysql_errno() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	int ret = mysql_errno((MYSQL *)mysql);
	stackPushInt(ret);
}

// Modified: supports both mysql_error(handle) AND mysql_error()
void gsc_mysql_error()
{
	int num_params = Scr_GetNumParam();
	
	// zk-style: mysql_error() - uses default connection
	if (num_params == 0)
	{
		if (default_zk_connection == NULL)
		{
			stackError("mysql_error() - no default connection");
			stackPushUndefined();
			return;
		}
		
		stackPushString((char *)mysql_error(default_zk_connection));
		return;
	}
	
	// Original rev style: mysql_error(handle)
	if (num_params == 1)
	{
		int mysql;
		if ( ! stackGetParams("i", &mysql))
		{
			stackError("gsc_mysql_error(handle)");
			stackPushUndefined();
			return;
		}
		
		char *ret = (char *)mysql_error((MYSQL *)mysql);
		stackPushString(ret);
		return;
	}
	
	stackError("gsc_mysql_error() expects 0 or 1 arguments");
	stackPushUndefined();
}

// Modified: supports both mysql_affected_rows(handle) AND mysql_affected_rows()
void gsc_mysql_affected_rows()
{
	int num_params = Scr_GetNumParam();
	
	// zk-style: mysql_affected_rows() - uses default connection
	if (num_params == 0)
	{
		if (default_zk_connection == NULL)
		{
			stackError("mysql_affected_rows() - no default connection");
			stackPushUndefined();
			return;
		}
		
		stackPushInt(mysql_affected_rows(default_zk_connection));
		return;
	}
	
	// Original rev style: mysql_affected_rows(handle)
	if (num_params == 1)
	{
		int mysql;
		if ( ! stackGetParams("i", &mysql))
		{
			stackError("gsc_mysql_affected_rows(handle)");
			stackPushUndefined();
			return;
		}
		
		int ret = mysql_affected_rows((MYSQL *)mysql);
		stackPushInt(ret);
		return;
	}
	
	stackError("gsc_mysql_affected_rows() expects 0 or 1 arguments");
	stackPushUndefined();
}

// Modified: supports both mysql_store_result(handle) AND mysql_store_result()
void gsc_mysql_store_result()
{
	int num_params = Scr_GetNumParam();
	
	// zk-style: mysql_store_result() - uses default connection
	if (num_params == 0)
	{
		if (default_zk_connection == NULL)
		{
			stackError("mysql_store_result() - no default connection");
			stackPushUndefined();
			return;
		}
		
		MYSQL_RES *result = mysql_store_result(default_zk_connection);
		stackPushInt((intptr_t)result);
		return;
	}
	
	// Original rev style: mysql_store_result(handle)
	if (num_params == 1)
	{
		int mysql;
		if ( ! stackGetParams("i", &mysql))
		{
			stackError("gsc_mysql_store_result(handle)");
			stackPushUndefined();
			return;
		}
		
		MYSQL_RES *result = mysql_store_result((MYSQL *)mysql);
		stackPushInt((intptr_t)result);
		return;
	}
	
	stackError("gsc_mysql_store_result() expects 0 or 1 arguments");
	stackPushUndefined();
}

void gsc_mysql_num_rows()
{
	int result;

	if ( ! stackGetParams("i", &result))
	{
		stackError("gsc_mysql_num_rows() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	int ret = mysql_num_rows((MYSQL_RES *)result);
	stackPushInt(ret);
}

void gsc_mysql_num_fields()
{
	int result;

	if ( ! stackGetParams("i", &result))
	{
		stackError("gsc_mysql_num_fields() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	int ret = mysql_num_fields((MYSQL_RES *)result);
	stackPushInt(ret);
}

void gsc_mysql_field_seek()
{
	int result;
	int offset;

	if ( ! stackGetParams("ii", &result, &offset))
	{
		stackError("gsc_mysql_field_seek() one or more arguments is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	int ret = mysql_field_seek((MYSQL_RES *)result, offset);
	stackPushInt(ret);
}

void gsc_mysql_fetch_field()
{
	int result;

	if ( ! stackGetParams("i", &result))
	{
		stackError("gsc_mysql_fetch_field() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	MYSQL_FIELD *field = mysql_fetch_field((MYSQL_RES *)result);
	if (field == NULL)
	{
		stackPushUndefined();
		return;
	}
	char *ret = field->name;
	stackPushString(ret);
}

void gsc_mysql_fetch_row()
{
	int result;

	if ( ! stackGetParams("i", &result))
	{
		stackError("gsc_mysql_fetch_row() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}

	MYSQL_ROW row = mysql_fetch_row((MYSQL_RES *)result);
	if (!row)
	{
		stackPushUndefined();
		return;
	}

	stackPushArray();

	int numfields = mysql_num_fields((MYSQL_RES *)result);
	for (int i=0; i<numfields; i++)
	{
		if (row[i] == NULL)
			stackPushUndefined();
		else
			stackPushString(row[i]);

		stackPushArrayLast();
	}
}

void gsc_mysql_free_result()
{
	int result;

	if ( ! stackGetParams("i", &result))
	{
		stackError("gsc_mysql_free_result() argument is undefined or has a wrong type");
		stackPushUndefined();
		return;
	}
	if(result == 0)
	{
		stackError("mysql_free_result() input is a NULL-pointer");
		stackPushUndefined();
		return;
	}
	mysql_free_result((MYSQL_RES *)result);
	stackPushUndefined();
}

// Modified: supports both mysql_real_escape_string(handle, str) AND mysql_real_escape_string(str)
void gsc_mysql_real_escape_string()
{
	int num_params = Scr_GetNumParam();
	
	// zk-style: mysql_real_escape_string(str) - uses default connection
	if (num_params == 1)
	{
		const char *str;
		if ( ! stackGetParams("s", &str))
		{
			stackError("mysql_real_escape_string(str)");
			stackPushUndefined();
			return;
		}
		
		if (default_zk_connection == NULL)
		{
			stackError("mysql_real_escape_string(str) - no default connection");
			stackPushUndefined();
			return;
		}
		
		char *to = (char *) malloc(strlen(str) * 2 + 1);
		mysql_real_escape_string(default_zk_connection, to, str, strlen(str));
		stackPushString(to);
		free(to);
		return;
	}
	
	// Original rev style: mysql_real_escape_string(handle, str)
	if (num_params == 2)
	{
		int mysql;
		const char *str;
		
		if ( ! stackGetParams("is", &mysql, &str))
		{
			stackError("gsc_mysql_real_escape_string(handle, str)");
			stackPushUndefined();
			return;
		}
		
		char *to = (char *) malloc(strlen(str) * 2 + 1);
		mysql_real_escape_string((MYSQL *)mysql, to, str, strlen(str));
		stackPushString(to);
		free(to);
		return;
	}
	
	stackError("gsc_mysql_real_escape_string() expects 1 or 2 arguments");
	stackPushUndefined();
}

#endif
