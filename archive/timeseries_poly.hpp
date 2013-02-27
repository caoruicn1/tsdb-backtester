//
//  timeseries.hpp
//  backtester
//
//  Created by Andreas Fragner on 1/25/13.
//  Copyright (c) 2013 Andreas Fragner. All rights reserved.
//



/************************************************************************
 *
 *                  TimeSeries Class Interface
 *
 *
 *  Design Overview:
 *
 *  This implementation uses a column-oriented representation of financial time series
 *  where each set of records . Columns in a relational database map one to one
 *  and losely modelled after python pandas dataframes, with some performance enhancements at the cost
 *  of reduced flexibility. with a tuple of data records at each point, e.g.
 *
 *
 *  This class makes heavy use of boost.Range, in particular Range adaptors
 *
 *
 ************************************************************************/


#ifndef backtester_timeseries_hpp
#define backtester_timeseries_hpp

//BOOST
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>

//STL
#include <list>
#include <iterator>
#include <vector>

//#include <map>

#include "datapoint.hpp"

namespace bpt = boost::posix_time;
namespace dp  = datapoint;


namespace timeseries {
    
    
    // EXCEPTIONS
    
    class TimeSeriesException: public std::exception {
        
    public:
        TimeSeriesException(const std::string& message):_msg(message){};
        ~TimeSeriesException() throw(){};
        
        virtual const char* what() const throw() {return (_spec + _msg).c_str(); }
        
    private:
        const std::string _msg;
        static const std::string _spec;
    };
    
    
    // *****************************************************************
    //
    // TIME SERIES BASE CLASS
    //
    // *****************************************************************
    
    class TimeSeries {
        
        
    public:
        
        // CONSTRUCTORS
        
        TimeSeries( const std::string& meta = "" );
        
        // COPY & ASSIGNMENT
        
        TimeSeries(const TimeSeries&);
        TimeSeries& operator=(TimeSeries& rhs);
        
        
        // META AND COLUMN INFORMATION
        
        
        // returns a list containing the column names
        std::list<std::string> columns() const;
        
        // set the column names
        void set_columns( const std::list<std::string>& col_names );
        
        // returns the meta information of the series
        std::string meta() const;
        
        // set the meta information of the series
        void set_meta( const std::string& meta);
        
        
        // MUTATORS
        
        
        
        // EXTRACTORS
        
        
        // ITERATORS
        std::vector< dp::DataPoint >::const_iterator begin();
        std::vector< dp::DataPoint >::const_iterator end();
        //std::vector< dp::DataPoint >::iterator begin();
        //std::vector< dp::DataPoint >::iterator end();
        
        
        
        
        // TIME RELATED
        
        // returns first time stamp
        bpt::ptime first() const;
        
        // returns last time stamp
        bpt::ptime last() const;
        
        // returns estimated frequency
        bpt::time_duration frequency();
        
        // returns a vector of all timestamps
        std::vector<std::time_t> timestamps() const;
        
        
        
        // COLUMN ACCESSORS & ITERATORS
        
        // returns a vector of given column
        virtual std::vector<double> get_column(const std::string& col_name);
        
        // loads all values in column into container beginning at start iterator

        
        
        // RESAMPLING & RELATED
        
        // resamples the series in place to unix_timestamp frequency
        void resample(time_t freq){
            // use boost filtered range for this
            //boost::copy( rng | boost::adaptors::filtered(pred), out );
        }
        
        // resamples the series to boost::ptime time_duration frequency
        void resample(bpt::time_duration freq){
            //if( freq > this->frequency() )
            //  throw TimeSeriesException("");
            
            
        }
        
        // STATE RELATED
        
        bool isLoaded() const;
        bool isEmpty() const;
        
            
        // FRIENDS
        // swap function
        
        
        
        // PRIVATE DATA MEMBERS
        
    protected:
        
        std::map<std::time_t,double> _data;     // internal data container
        std::time_t _frequency;                 // fundamental frequency of time series
        std::string _meta;                      // string with meta information
        std::list<std::string> _columns;        // list containing the column index names
        
        bool _isLoaded;
        
        
    }; // TimeSeries class
    
    
    // *****************************************************************
    //
    // TIME SERIES DERIVED CLASSES
    //
    // *****************************************************************
    
    
    class TimeSeriesOHLC: public TimeSeries {
        
    public:
         TimeSeriesOHLC(const std::string& meta = "");
        
    private:
        
        std::map<std::time_t,dp::OHLC> _data;
//        static const std::list<std::string> _columns;
        
    }; // TimeSeriesOHLC class

    
    class TimeSeriesOHLCV: public TimeSeries {

    private:
        
        std::map<std::time_t,dp::OHLCV> _data;
        static const std::vector<std::string> _columns;
        
    }; // TimeSeriesOHLCV class

    
    class TimeSeriesBidAsk: public TimeSeries {
    
    private:
        
        std::map<std::time_t,dp::OHLC> _data;
        static const std::vector<std::string> _columns;
        
    }; // TimeSeriesBidAsk class

    
    
} // namespace timeseries



#endif
