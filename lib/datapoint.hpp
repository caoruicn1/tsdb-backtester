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
 * datapoint.hpp
 *
 * Design Overview:
 *
 * Simple POD structures for representing labeled non-tick data. Used
 * for internal price data representation in timeseries template class,
 * which restricts template parameters to classes derived from DataPoint,
 * see timeseries.hpp.
 *
 * Design Objectives:
 *
 * Minimize memory and copy footprint of the datapoint structures while
 * at the same time exposing member variables in an intuitive way and
 * therefore reducing implementation risk.
 *
 *
 */


#ifndef backtester_datapoint_hpp
#define backtester_datapoint_hpp

#include <exception>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/static_assert.hpp>

namespace bpt = boost::posix_time;

namespace datapoint {
    
    // EXCEPTIONS
    
    class DataPointException: public std::exception {
        
    public:
        DataPointException(unsigned short code):error_code(code){};
        ~DataPointException() throw(){};
            
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
        static const std::map<int,std::string> messages;
    };
    
    
    // ABSTRACT BASE
    
    struct DataPoint {
        virtual ~DataPoint() = 0;
    };
    
    
    // DERIVED CLASSES
    // Note: explicitly specify default move operators bc of current compiler inconsistencies
    
    struct OHLC: public DataPoint {
        
        OHLC(double o, double h, double l, double c);
        OHLC(const std::vector<double>& init);

        OHLC( const OHLC& ) = default;
        OHLC& operator=( OHLC&& ) = default;
        OHLC& operator=( const OHLC& ) = default;

        double open, high, low, close;
    };
    
    
    struct OHLCV: public DataPoint {
        
        OHLCV(double o, double h, double l, double c, int v);
        OHLCV(const std::vector<double>& init);
        
        OHLCV( const OHLCV& ) = default;
        OHLCV& operator=( OHLCV&& ) = default;
        OHLCV& operator=( const OHLCV& ) = default;
        
        int volume;
        double open, high, low, close;
    };

    
    struct BidAsk: public DataPoint {
        
        BidAsk(double b, double a);
        BidAsk(const std::vector<double>& init);
        
        BidAsk( const BidAsk& ) = default;
        BidAsk& operator=( BidAsk&& ) = default;
        BidAsk& operator=( const BidAsk& ) = default;
        
        double bid, ask;
    };
    

    // HELPERS
    
    // emulate reflection via templates
    template<typename T> std::vector<std::string> dp_names(){
        
        BOOST_STATIC_ASSERT((boost::is_base_of< DataPoint, T>::value));
        
        const char* res[] = { "" };
        return std::vector<std::string>(res,res+1);
    };

    template<> std::vector<std::string> inline dp_names<OHLC>(){
        const char* res[] = { "open", "high", "low", "close" };
        return std::vector<std::string>(res,res+4);
    };

    template<> std::vector<std::string> inline dp_names<OHLCV>(){
        const char* res[] = { "open", "high", "low", "close", "volume" };
        return std::vector<std::string>(res,res+5);
    };
    
    template<> std::vector<std::string> inline dp_names<BidAsk>(){
        const char* res[] = { "bid","ask" };
        return std::vector<std::string>(res,res+2);
    };
    
} //namespace datapoint




#endif





