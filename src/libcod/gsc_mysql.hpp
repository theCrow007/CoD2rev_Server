#ifndef GSC_MYSQL_HPP
#define GSC_MYSQL_HPP
#include "gsc.hpp"
#if LIBCOD_COMPILE_MYSQL == 1

// Original rev functions
void gsc_mysql_init();
void gsc_mysql_real_connect();
void gsc_mysql_close();
void gsc_mysql_query();
void gsc_mysql_errno();
void gsc_mysql_error();
void gsc_mysql_affected_rows();
void gsc_mysql_store_result();
void gsc_mysql_num_rows();
void gsc_mysql_num_fields();
void gsc_mysql_field_seek();
void gsc_mysql_fetch_field();
void gsc_mysql_fetch_row();
void gsc_mysql_free_result();
void gsc_mysql_real_escape_string();

// zk-style wrapper
void gsc_mysql_initialize();

// Async functions (original rev names)
void gsc_mysql_async_create_query();
void gsc_mysql_async_create_query_nosave();
void gsc_mysql_async_getdone_list();
void gsc_mysql_async_getresult_and_free();
void gsc_mysql_async_initializer();
void gsc_mysql_reuse_connection();

// Async function aliases for gsc.cpp (prefix gsc_async_mysql_*)
void gsc_async_mysql_initialize();
void gsc_async_mysql_close();
void gsc_async_mysql_create_query();
void gsc_async_mysql_create_query_nosave();
void gsc_async_mysql_checkdone();
void gsc_async_mysql_errno();
void gsc_async_mysql_error();
void gsc_async_mysql_affected_rows();
void gsc_async_mysql_num_rows();
void gsc_async_mysql_num_fields();
void gsc_async_mysql_field_seek();
void gsc_async_mysql_fetch_field();
void gsc_async_mysql_fetch_row();
void gsc_async_mysql_free_task();
void gsc_async_mysql_real_escape_string();

#endif

#endif