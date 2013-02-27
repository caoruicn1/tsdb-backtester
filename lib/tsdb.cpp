/*
 * TSDB-Backtester - C/C++ framework for algorithmic trading strategy backtests
 *
 * The MIT License
 *
 * Copyright (c) 2013 Andreas Fragner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */


#include "tsdb.hpp"

using namespace tsdb;

// STATICS

const std::string Interface::_database = DATABASE;
const std::string Interface::_host = HOST;
const std::string TSDBInterfaceException::base_msg = "TSDB Interface Exception: ";

const std::map<int,std::string> TSDBInterfaceException::messages {
    {0, "Unknown TSDBInterface exception."},
    {1, "Could not connect to TSDB server."},
    {2, "Invalid table name."},
    {3, "Failed to load TSDB table."},
    {4, "Invalid date range request."},
    {5, "Column mismatch."},
    {6, "Failed to set session time zone."},
    {7, "MySQL server error."}};

    
// CONSTRUCTORS, DESTRUCTORS

Interface::Interface(const std::string& user, const std::string& password)
:   _user(user),
    _password(password),
    _drv(),
    _con(),
    _session_tz()
{
    try{
        _drv = get_driver_instance();
    }
    catch (sql::SQLException &e) { 
        _print_SQLException(e);
    }
}


Interface::~Interface(){    //no throw
    disconnect();
}


// CONNECT, DISCONNECT

void Interface::connect() {
    
    try {
        
        _con.reset( _drv->connect( _host, _user, _password) );
        _con->setSchema( _database );
        _con->setAutoCommit(false);
        
        std::unique_ptr< sql::Statement > stmt( _con->createStatement() );
		stmt->execute("SET time_zone='+00:00'"); //set server session timezone to UTC by default
        
        std::unique_ptr< sql::ResultSet > rset( stmt->executeQuery("SELECT @@session.time_zone;") );

        rset->next();
        _session_tz = rset->getString("@@session.time_zone");
        
        if( !rset->rowsCount() || _session_tz != "+00:00" )
            throw TSDBInterfaceException(6);

    }
    catch(sql::SQLException& e){
        _print_SQLException(e);
        throw TSDBInterfaceException(1);
    }
}


void Interface::disconnect() {  //no throw
    
    if( _con.get()==NULL )
        return;
    
    try {
        _con->close();
        _con.reset();
        _session_tz = "";
    }
    catch(sql::SQLException& e){
        _print_SQLException(e);
    }
}


bool Interface::isConnected() {
    if ( _con.get() == NULL )
        return false;
    return !( _con->isClosed() );
}


// ACCESSORS

bool Interface::has_table( const std::string& name){    //no throw
    
    size_t rows = 0;
    string query = "SELECT table_name FROM information_schema.tables WHERE table_name = '"+name+"';";

    try{
        
        if( !isConnected())
            connect();
        
        std::unique_ptr< sql::Statement > stmt( _con->createStatement());
        std::unique_ptr< sql::ResultSet > res(stmt->executeQuery(query));
        rows = res->rowsCount();
        
        if( !rows )
            return false;
    }
    catch( sql::SQLException &e){
        _print_SQLException(e);
        return false;
    }
    
    return rows;
}


vector<string> Interface::get_column_names( const string& table_name ) //throws
{
    if( !isConnected())
        connect();
    
    if( !has_table(table_name) )
        throw TSDBInterfaceException(2);
    
    try {
        
        unique_ptr< sql::Statement > stmt( _con->createStatement() );
        unique_ptr< sql::ResultSet > rset( stmt->executeQuery("SHOW COLUMNS FROM "+ table_name+";"));
        
        vector<string> cols;
        cols.reserve( rset->rowsCount() );
        
        while( rset->next() )
            cols.push_back( rset->getString("Field") );
        
        return cols;
        
    }
    catch(sql::SQLException& e){
        
        _print_SQLException(e);
        throw TSDBInterfaceException(7);
    }
}


// META INFORMATION PRINTERS

void Interface::print_connection_info() //nothrow
{
    std::cout << std::boolalpha;
    std::cout << "\nConnected: " << isConnected() << std::endl;
    
    if( !isConnected() ){
        std::cout << "Interface is not connected to TSDB." << std::endl;
        return;
    }
    
    try { 
        std::cout << "Host: " << _host << std::endl;
        std::cout << "DB Schema: " << _con->getSchema() << std::endl;
        std::cout << "User: " << _con->getMetaData()->getUserName() << std::endl;
        std::cout << "Driver name: " << _con->getMetaData()->getDriverName() << std::endl;
        std::cout << "Driver version: " << _con->getMetaData()->getDriverVersion() << std::endl;
        std::cout << "Session timezone: " << _session_tz << std::endl;
    }
    catch(sql::SQLException& e){
        _print_SQLException(e);
    }
}


void Interface::print_metadata() //nothrow
{
    
    if( !isConnected() ){
        std::cout << "\nMetaData information not available. Interface not connected to TSDB." << std::endl;
        return;
    }
    
    try{
        std::cout << "\nTSDB Database Metadata" << std::endl;
        std::cout << "--------------------------" << std::endl;
    
        std::cout << std::boolalpha;
    
        sql::DatabaseMetaData *meta = _con -> getMetaData();
    
        std::cout << "Database Product Name: " << meta -> getDatabaseProductName() << std::endl;
        std::cout << "Database Product Version: " << meta -> getDatabaseProductVersion() << std::endl;
        std::cout << "Database User Name: " << meta -> getUserName() << std::endl << std::endl;
    
        std::cout << "Driver name: " << meta -> getDriverName() << std::endl;
        std::cout << "Driver version: " << meta -> getDriverVersion() << std::endl << std::endl;
        
        std::cout << "Database in Read-Only Mode?: " << meta -> isReadOnly() << std::endl;
        std::cout << "Supports Transactions?: " << meta -> supportsTransactions() << std::endl;
        std::cout << "Supports DML Transactions only?: " << meta -> supportsDataManipulationTransactionsOnly() << std::endl;
        std::cout << "Supports Batch Updates?: " << meta -> supportsBatchUpdates() << std::endl;
        std::cout << "Supports Outer Joins?: " << meta -> supportsOuterJoins() << std::endl;
        std::cout << "Supports Multiple Transactions?: " << meta -> supportsMultipleTransactions() << std::endl;
        std::cout << "Supports Named Parameters?: " << meta -> supportsNamedParameters() << std::endl;
        std::cout << "Supports Statement Pooling?: " << meta -> supportsStatementPooling() << std::endl;
        std::cout << "Supports Stored Procedures?: " << meta -> supportsStoredProcedures() << std::endl;
        std::cout << "Supports Union?: " << meta -> supportsUnion() << std::endl << std::endl;
        
        std::cout << "Maximum Connections: " << meta -> getMaxConnections() << std::endl;
        std::cout << "Maximum Columns per Table: " << meta -> getMaxColumnsInTable() << std::endl;
        std::cout << "Maximum Columns per Index: " << meta -> getMaxColumnsInIndex() << std::endl;
        std::cout << "Maximum Row Size per Table: " << meta -> getMaxRowSize() << " bytes" << std::endl;
        
        std::cout << std::endl << std::endl;
    }
    catch( sql::SQLException& e){
        _print_SQLException(e);
    }
    
}


