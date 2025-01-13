#include "vertica.h"

using std::endl;
using std::cout;

SQLRETURN VerticaWriter::executeWithRetry(SQLHSTMT hdlStmt) {
    const int maxRetries = 2;
    int retries = 0;

    while (retries < maxRetries) {
        SQLRETURN ret = SQL_ERROR;
        try{
            ret = SQLExecute(hdlStmt);
        }catch (...){
            ret = SQL_ERROR;
        }
        if (SQL_SUCCEEDED(ret)) {
            return ret; // Success
        }
        else if (isConnectionError()) {
            // Handle connection error, retry
            cout << "Connection error, retrying" << endl;
            reconnect();
            retries++;
        }
        else {
            // Handle other errors
            SQLFreeHandle(SQL_HANDLE_STMT, hdlStmt);
            handleODBCError("Error executing statement", ret);
            return ret;
        }
    }
    // Max retries reached, handle error
    SQLFreeHandle(SQL_HANDLE_STMT, hdlStmt);
    handleODBCError("Max retries reached, could not execute statement", SQL_ERROR);
    return SQL_ERROR;
}

SQLRETURN VerticaWriter::executeWithRetry(SQLHSTMT hdlStmt, const char *query) {
    const int maxRetries = 2;
    int retries = 0;

    while (retries < maxRetries) {
        SQLRETURN ret = SQL_ERROR;
        try{
            ret = SQLExecDirect(hdlStmt, (SQLTCHAR *)query, SQL_NTS);
        }catch (...){
            ret = SQL_ERROR;
        }
        if (SQL_SUCCEEDED(ret)) {
            return ret; // Success
        }
        else if (isConnectionError()) {
            // Handle connection error, retry
            reconnect();
            retries++;
        }
        else {
            // Handle other errors
            SQLFreeHandle(SQL_HANDLE_STMT, hdlStmt);
            handleODBCError("Error executing statement", ret);
            return ret;
        }
    }

    // Max retries reached, handle error
    SQLFreeHandle(SQL_HANDLE_STMT, hdlStmt);
    handleODBCError("Max retries reached, could not execute statement", SQL_ERROR);
    return SQL_ERROR;
}

void VerticaWriter::handleODBCError(const char *message, SQLRETURN ret) {
    std::cerr << message << " ret code: " << ret << std::endl;
    throw(ret);
    // Additional error handling code can be added here, such as logging or raising exceptions.
}

void VerticaWriter::handleSQLReturn(SQLRETURN ret, const char *message, SQLHSTMT hdlStmt) {
    if (!SQL_SUCCEEDED(ret)) {
        // free the statement handle
        if (hdlStmt != SQL_NULL_HSTMT) {
            SQLFreeHandle(SQL_HANDLE_STMT, hdlStmt);
        }
        handleODBCError(message, ret);
    }
}

void VerticaWriter::initializeODBC() {
    SQLRETURN ret;
    ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hdlEnv);
    handleSQLReturn(ret, "Could not allocate a handle.");

    ret = SQLSetEnvAttr(m_hdlEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_UINTEGER);
    handleSQLReturn(ret, "Could not set application version to ODBC3.");

    ret = SQLAllocHandle(SQL_HANDLE_DBC, m_hdlEnv, &m_hdlDbc);
    handleSQLReturn(ret, "Could not allocate database handle.");
}

void VerticaWriter::cleanupODBC() {
    if (m_hdlDbc != SQL_NULL_HDBC) {
        SQLFreeHandle(SQL_HANDLE_DBC, m_hdlDbc);
    }
    if (m_hdlEnv != SQL_NULL_HENV) {
        SQLFreeHandle(SQL_HANDLE_ENV, m_hdlEnv);
    }
}

void VerticaWriter::connect() {
    SQLRETURN ret;

    // Disable AUTOCOMMIT
    ret = SQLSetConnectAttr(m_hdlDbc, SQL_ATTR_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF, SQL_NTS);
    handleSQLReturn(ret, "Could not disable autocommit.");

    ret = SQLSetConnectAttr(m_hdlDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
    handleSQLReturn(ret, "Could not set login timeout.");

    ret = SQLConnect(m_hdlDbc, (SQLCHAR *)m_dsnName, SQL_NTS, (SQLCHAR *)m_userid, SQL_NTS, (SQLCHAR *)m_password, SQL_NTS);
    m_isConnected = SQL_SUCCEEDED(ret);
    handleSQLReturn(ret, "Could not connect to the database.");
}

void VerticaWriter::disconnect() {
    if (m_isConnected) {
        SQLDisconnect(m_hdlDbc);
        m_isConnected = false;
    }
}

bool VerticaWriter::isConnectionError() {
    // check connection is open
    SQLUINTEGER connectionDead;
    SQLRETURN ret = SQLGetConnectAttr(m_hdlDbc, SQL_ATTR_CONNECTION_DEAD, &connectionDead, 0, NULL);
    if (SQL_SUCCEEDED(ret) && connectionDead == SQL_CD_TRUE) {
        m_isConnected = false;
        return true;
    }
    return false;
}

/*
    * Function to create an INSERT query
    * @param tableName: Name of the table to insert into
    * @param pattern: Pattern for the insert query (e.g: `"(%d, %d, %d)", 1, 2, 3`)
    * @return: The generated INSERT query
*/
std::string createInsertQuery(const char* tableName, const char* pattern, ...) {
    char qry_buff[1000]; // Adjust the buffer size as needed
    va_list args;
    va_start(args, pattern);

    // Create the INSERT query with variable values
    vsnprintf(qry_buff, sizeof(qry_buff), pattern, args);

    va_end(args);

    // Combine the table name and the generated query
    std::string insertQuery = "INSERT INTO " + std::string(tableName) + " VALUES " + qry_buff;
    return insertQuery;
}

