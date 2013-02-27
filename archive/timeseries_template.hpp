//
//  timeseries.hpp
//  simulator
//
//  Created by Andreas Fragner on 1/3/13.
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
 *  column-iteration is supported
 *  column access
 *
 *  time representation: posix time
 *
 *
 *  General Design:
 *
 *  The TimeSeries class is implemented as a template class (compile-time polymorphism)
 *  with concept checking. The specific type of datapoint is passed as a template
 *  parameter, e.g. TimeSeries< OHLC >, which must be a derived class type from
 *  the abstract base class dp::DataPoint. To ensure this, we use boost static assert
 *  which gives a compile time error when the template parameter is not of base type
 *  dp::DataPoint. The TimeSeries class has a template specialized constructor since
 *  the column names (_index) are special for each type of DataPoint.
 *
 *
 ************************************************************************/


#ifndef backtester_timeseries_hpp
#define backtester_timeseries_hpp


#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <vector>
#include <iterator>

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
    
    
    
    // CLASS INTERFACE
    
    template <typename DataPointType> class TimeSeries {
        
        //restricting template parameters
        //compile time error if template parameter is not inherited from dp::DataPoint
        //assertion will be evaluated at the time the template is instantiated
        BOOST_STATIC_ASSERT((boost::is_base_of< dp::DataPoint, DataPointT>::value));
        
    public:
        
        // CONSTRUCTORS

        TimeSeries( const std::string& meta = "" )
        :   _meta(meta),
            _isLoaded(false)
        {};
        
        // COPY CONSTRUCTOR
        
        TimeSeries(const TimeSeries&); //copy constructor
        
        
        // OVERLOADED OPERATORS
        
        TimeSeries& operator=(TimeSeries& rhs);
        
        // TSDB METHODS
        
        bool isLoaded(){
            return _isLoaded;
        }
        
        void load();
        
        
        // ACCESSORS
      
        
        // returns a vector containing the column names
        std::vector<std::string> index() const {
            return _index;
        }
        
        // returns the meta information of the series
        std::string meta() const {
            return _meta;
        }
        
        // MUTATORS
        

        
        // EXTRACTORS
        
        
        // ITERATORS
        std::vector< dp::DataPoint >::const_iterator begin();
        std::vector< dp::DataPoint >::const_iterator end();
        //std::vector< dp::DataPoint >::iterator begin();
        //std::vector< dp::DataPoint >::iterator end();
        
        
        // TIME RELATED
        bpt::ptime first() const {
            return bpt::from_time_t( *(_data.begin()->first) );
        }

        bpt::ptime last() const {
            return bpt::from_time_t( *(_data.rbegin()->first) );
        }
        
        bpt::time_duration frequency(){
            //estimate frequency of series from data
            
            //set the 
        }
        
        std::vector<std::time_t> timestamps() const{
            return _data.keys();
        }
        
        // RESAMPLING & RELATED
        
        // check if the time series has no entries
        bool isEmpty(){
            
            return _data.empty();
        };
        
        // resamples the series to unix_timestamp frequency
        void resample(time_t freq){
           // if( freq < _frequency )
            
        }
        
        // resamples the series to boost::ptime time_duration frequency
        void resample(bpt::time_duration freq){
            //if( freq > this->frequency() )
              //  throw TimeSeriesException("");
            
            
        }
        
        
        // COLUMN ACCESSORS & ITERATORS
      

        
        // FRIENDS
        // swap function
        
        
        
        // PRIVATE DATA MEMBERS
        
    private:
        
        std::map< std::time_t, DataPointT > _data;
        
        std::time_t _frequency;         // fundamental frequency of time series
        bpt::ptime _first, _last;       // first and last timestamp
        std::string _meta;              // string with meta information
        
        static const std::vector<std::string> _index;
        //vector containing the column index names, inferred at compile time according to template parameter of TimeSeries

        bool _isLoaded;
        
        
    };
    
    
    // STATICS
    
    // static member _index initialized based on template parameter
    template<typename T> const std::vector<std::string> TimeSeries<T>::_index=dp::datapoint_index<T>();
    

} // namespace timeseries



#endif



