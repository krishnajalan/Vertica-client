#ifndef VERTICA_H
#define VERTICA_H

#include <iostream>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <stdarg.h>


// Struct to hold the vertica Login details
struct VerticaLogin {
    std::string dsnName;
    std::string userid;
    std::string password;
};
static VerticaLogin vertica_login;

class VerticaWriter
{
protected:
    const char *m_dsnName;
    const char *m_userid;
    const char *m_password;
    SQLHDBC m_hdlDbc;
    SQLHENV m_hdlEnv;
    bool m_isConnected;

public:
    VerticaWriter(const char *dsnName, const char *userid, const char *password)
        : m_dsnName(dsnName), m_userid(userid), m_password(password), m_isConnected(false) {
        initializeODBC();
        connect();
    }

    ~VerticaWriter() {
        disconnect();
        cleanupODBC();
    }

    bool isConnected() const {
        return m_isConnected;
    }

    void reconnect() {
        disconnect();
        connect();
    }

    void close() {
        disconnect();
    }

    SQLRETURN executeWithRetry(SQLHSTMT hdlStmt);
    SQLRETURN executeWithRetry(SQLHSTMT hdlStmt, const char *query);

    void handleODBCError(const char *message, SQLRETURN ret = SQL_ERROR);

    void handleSQLReturn(SQLRETURN ret, const char *message = "", SQLHSTMT hdlStmt = SQL_NULL_HSTMT);

private:
    void initializeODBC();

    void cleanupODBC();

    void connect();

    void disconnect();

    bool isConnectionError();
};

/*
    * Function to create an INSERT query
    * @param tableName: Name of the table to insert into
    * @param pattern: Pattern for the insert query (e.g: `"(%d, %d, %d)", 1, 2, 3`)
    * @return: The generated INSERT query
*/
std::string createInsertQuery(const char* tableName, const char* pattern, ...);

#endif
