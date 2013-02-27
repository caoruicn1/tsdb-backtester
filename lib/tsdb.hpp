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
 * ------------------------------------------------------------------------------
 *
 * tsdb.hpp
 *
 * Design Overview:
 *
 * Provides a simple read-only mySQL database interface for loading time series 
 * data into tsdb::TimeSeries<T> and tsdb::DataFrame<T> objects. Interface objects 
 * are non-copyable and non- assignable for security reasons.
 *
 * Notes:
 *
 * Requires linking the mysqlclient and mysql connector/c++ libraries. The MySQL 
 * Connector/C++ libs have to be built from source using a C++11 compliant compiler. 
 * This means fiddling a bit with the make files that come with source distributions. 
 * Host and database are currently hard-coded for testing purposes.
 *  
 */

#ifndef backtester_tsdb_hpp
#define backtester_tsdb_hpp

// MySQL Connector/C++
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/metadata.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/exception.h>
#include <cppconn/warning.h>

// STL & Boost
#include <cstdlib>
#include <memory>
#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/static_assert.hpp>

// Backtester
#include "utilities.hpp"
#include "timeseries.hpp"

namespace ts  = timeseries;

#define HOST "tcp://127.0.0.1:3306"
#define DATABASE "tsdb"


namespace tsdb {
    
    
    //EXCEPTIONS
    
    class TSDBInterfaceException: public std::exception {
        
    public:
        TSDBInterfaceException(unsigned short code):error_code(code){};
        ~TSDBInterfaceException() throw(){};
        
        virtual const char* what() const throw() {
            try {
                return (base_msg + messages.at(error_code)).c_str();
            }
            catch( std::out_of_range& e ){
                return (base_msg + messages.at(0)).c_str();
            }
        }
        
    private:
        unsigned short error_code;
        static const std::string base_msg;
        static const std::map<int,string> messages;
    };
    

    // TSDB INTERFACE CLASS
    
    class Interface: private utilities::Uncopyable {
        
    public:
        
        //CONSTRUCTORS
        
        Interface(const std::string&, const std::string&); // throws
        ~Interface(); //no throw
                
        // CONNECT/DISCONNECT
        
        void connect();     // throws
        void disconnect();  // no throw
        bool isConnected(); // no throw
        
        // ACCESSORS
        
        bool has_table(const std::string&);
        std::vector<std::string> get_column_names( const std::string& );
        std::map<std::string,int> get_table_dimensions( const std::string& );
        
        // META INFORMATION PRINTERS
        
        void print_connection_info(); // no throw
        void print_metadata();        // no throw

        // LOAD

        template<typename T> void load(ts::TimeSeries<T>& series,
                                       const std::string& table,
                                       bpt::ptime start = bpt::ptime(),
                                       bpt::ptime end = bpt::ptime(),
                                       bool print_meta = false)         // throws
        {
            BOOST_STATIC_ASSERT((boost::is_base_of< dp::DataPoint, T>::value));
            
            if( !isConnected() )
                connect();
            
            if( !has_table(table) )
                throw TSDBInterfaceException(2);
            
            if( !_columns_match_type<T>(table) )
                throw TSDBInterfaceException(5);
            
            if( start > end )
                throw TSDBInterfaceException(4);
            
            try{

                std::string cols = boost::algorithm::join(dp::dp_names<T>(), ", ");
                std::string query = "SELECT date_time, "+cols+" FROM "+table;
                unique_ptr<sql::PreparedStatement> pstmt(_con->prepareStatement(query));
                
                if( !start.is_not_a_date_time() && !end.is_not_a_date_time() )
                {
                    query += " WHERE date_time BETWEEN (?) and (?);";
                    pstmt.reset( _con->prepareStatement(query) );
                    pstmt->setDateTime(1, utilities::bpt_to_str(start));
                    pstmt->setDateTime(2, utilities::bpt_to_str(end));
                }
                else if( !start.is_not_a_date_time() && end.is_not_a_date_time() )
                {
                    query += " WHERE date_time >= (?);";
                    pstmt.reset( _con->prepareStatement(query) );
                    pstmt->setDateTime(1, utilities::bpt_to_str(start));
                }
                else if( start.is_not_a_date_time() && !end.is_not_a_date_time() )
                {
                    query += " WHERE date_time <= (?);";
                    pstmt.reset( _con->prepareStatement(query) );
                	pstmt->setDateTime(1, utilities::bpt_to_str(end));
                }
              
                std::unique_ptr<sql::ResultSet> rset( pstmt->executeQuery() );
                sql::ResultSetMetaData* rset_meta( rset->getMetaData() );
                
                if( print_meta )
                    _print_loading_MetaData( rset_meta );
                
                int num_cols = rset_meta->getColumnCount();
                std::vector<double> row;
                row.reserve( num_cols-1 );
                
                while( rset->next() ){
                    
                    for( int i =  2; i <= num_cols; ++i) // MySQL Conn doesn't allow accessing entire row at once
                        row.push_back(rset->getDouble(i));
                    
                    series.insert( utilities::str_to_time_t(rset->getString(1)), T(row) ); //move insert
                    row.clear();
                }
            }
            catch( sql::SQLException& ex ) {
                _print_SQLException(ex);
                throw TSDBInterfaceException(3);
            }
            
        } //load
             
    private:
        
        const std::string _user;
        const std::string _password;
        std::string _session_tz;
        static const std::string _database;
        static const std::string _host;

        sql::Driver* _drv;
        std::unique_ptr< sql::Connection > _con;
        
        
        // HELPERS
        
        // tests if the columns of TSDB 'table' match the datapoint type T
        // returns false if 'table' does not have the columns necessary for required datatype
        template<typename T> bool _columns_match_type(const std::string& table)
        {
            BOOST_STATIC_ASSERT((boost::is_base_of< dp::DataPoint, T>::value));
            
            std::vector<std::string> columns = get_column_names( table );
            std::vector<std::string> t_columns = dp::dp_names<T>();
            
            std::sort(columns.begin(),columns.end());
            std::sort(t_columns.begin(),t_columns.end());
            
            if( !includes(columns.begin(), columns.end(), t_columns.begin(), t_columns.end() ) )
                return false;
            
            return true;
        }
        
        
        void _print_SQLException(sql::SQLException&e ){
            
            std::cout << "ERROR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << ")" << std::endl;
            
            if (e.getErrorCode() == 1047) {
                std::cout << "\nSQL server does not seem to support prepared statements. MYSQL > 5.1 required. ";
            }
        }
        
        
        void _print_loading_MetaData(sql::ResultSetMetaData* meta){
            
            int numcols = meta->getColumnCount();
            
            std::cout << "\nLoading table \"" << meta->getTableName(1) << "\" from schema \"" << meta->getSchemaName(1);
            std::cout << "\" into TimeSeries ..." << std::endl;
            std::cout << "\nNumber of columns in result set: " << numcols << std::endl << std::endl;
            
            cout.width(20); std::cout << "Column Name/Label";
            cout.width(20); std::cout << "Column Type";
            cout.width(20); std::cout << "Column Size" << std::endl;
            
            for (int i = 0; i < numcols; ++i) {
                std::cout.width(20);
                std::cout << meta -> getColumnLabel (i+1);
                
                std::cout.width(20);
                std::cout << meta -> getColumnTypeName (i+1);
                
                std::cout.width(20);
                std::cout << meta -> getColumnDisplaySize (i+1) << std::endl;
            }
            
            std::cout << "Loading ... \n" << std::endl;
        }

    
    };

} //namespace tsdb
        

#endif





