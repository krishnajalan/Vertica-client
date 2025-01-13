# Vertica-Client

This repository contains the `VerticaWriter` class, which provides an interface for interacting with a Vertica database using ODBC (Open Database Connectivity). The class handles database connections, query execution with retry logic, and error handling.

## Overview

The `VerticaWriter` class offers the following key functionalities:
- Connecting and disconnecting from a Vertica database.
- Executing SQL statements with retry logic.
- Handling ODBC errors and ensuring proper resource cleanup.

## Key Methods

### `initializeODBC()`
Initializes the ODBC environment and database connection handles.

### `connect()`
Establishes a connection to the Vertica database using the provided DSN, user ID, and password.

### `disconnect()`
Disconnects from the Vertica database and cleans up the connection state.

### `executeWithRetry(SQLHSTMT hdlStmt)`
Executes an SQL statement with a retry mechanism in case of connection errors. This method takes a statement handle as input.

### `executeWithRetry(SQLHSTMT hdlStmt, const char *query)`
Executes an SQL query string with a retry mechanism. This method takes both a statement handle and a query string as inputs.

### `handleODBCError(const char *message, SQLRETURN ret)`
Handles ODBC errors by logging the error message and throwing an exception.

### `handleSQLReturn(SQLRETURN ret, const char *message, SQLHSTMT hdlStmt)`
Checks the SQL return code and handles any errors by freeing the statement handle and logging the error.

### `cleanupODBC()`
Cleans up the ODBC environment and connection handles.

### `isConnectionError()`
Checks if the current connection is still valid. If the connection is dead, it returns true.

### `createInsertQuery(const char* tableName, const char* pattern, ...)`
Generates an SQL INSERT query string for a given table and value pattern.

## Usage

1. **Initialize ODBC Environment:**
   ```cpp
   VerticaWriter writer;
   writer.initializeODBC();

## Error Handling
The VerticaWriter class handles errors by logging the issue and cleaning up resources to prevent memory leaks. The retry mechanism ensures that temporary connection issues are handled gracefully.

## Dependencies
ODBC Driver for Vertica
Standard C++ Library

## License
This project is licensed under the MIT License. See the LICENSE file for details.
