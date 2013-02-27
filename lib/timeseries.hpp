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
 * timeseries.hpp
 *
 * Design Overview:
 *  
 * Template class implementation of a time series container type
 * optimized for row-oriented, non-vectorizable operations. Data is
 * internally represented as a std::map keyed off unix timestamps to
 * guarantee data alignment and time causality. Value types are restricted
 * to structures derived from DataPoint (see datapoint.hpp). This makes
 * writing algorithms more concise and intuitive and minimizes
 * implementation risk.
 *
 * Iterable memberspaces are used to selectively expose internal collection
 * data. Multivariate price data and timestamps can be accessed as if they
 * were separate containers. Allows user to iterate over filtered,
 * modified or resampled elements of a series in expressive client
 * code.
 *
 * As always, nice client code comes at a performance penality, in this
 * case O(log N) for all main operations and potentially some cache misses.
 * The sister class DataFrame uses linear flat arrays for internal data
 * representation and should be used for more performance-critical tasks.
 *
 */


#ifndef backtester_timeseries_hpp
#define backtester_timeseries_hpp

//BOOST
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

//STL
#include <iterator>
#include <vector>
#include <map>
#include <stdexcept> 
#include <type_traits>

#include "datapoint.hpp"

namespace bpt = boost::posix_time;
namespace dp  = datapoint;

namespace timeseries {
    
    // EXCEPTIONS 
    
    class TimeSeriesException: public std::exception {
        
    public:
        TimeSeriesException(const std::string& message);
        ~TimeSeriesException() throw();
        
        virtual const char* what() const throw();
        
    private:
        const std::string _msg;
        static const std::string _spec;
    };
    
        
    // -----------------------------------------------------------------
    // TIME SERIES TEMPLATE CLASS
    // -----------------------------------------------------------------
    
    template <typename T> class TimeSeries {
        
        BOOST_STATIC_ASSERT((boost::is_base_of< dp::DataPoint, T>::value));
            
    public:
    
        typedef std::map<time_t,T> TimeMap;
        typedef typename TimeMap::iterator iterator;
        typedef typename TimeMap::const_iterator const_iterator;

        
        // DEFAULT, MOVE & COPY CONSTRUCTION
        
        TimeSeries( const std::string& meta = "" ) //default ctor
        :   _meta(meta),
            _data(),
            _isLoaded(false),
            values(*this),
            timestamps(*this)
        {};
       
        TimeSeries( const TimeSeries& ts ) // copy ctor
        :   _meta( ts._meta ),
            _data( ts._data ),
            _isLoaded( ts._isLoaded),
            values(*this),
            timestamps(*this)
        {};
        
        
        TimeSeries( TimeSeries&& ts ) // move ctor
        :   _meta( std::move(ts._meta) ),
            _data( std::move(ts._data) ),
            _isLoaded( ts._isLoaded ),
            values( *this ),
            timestamps( *this )
        {
            ts._isLoaded = false;
            // no need to move the memberspace refs
        };
        
        ~TimeSeries() = default;
        
        
        // ASSIGNMENT

        TimeSeries& assign( TimeSeries rhs ){ // strong exception safety
            swap(*this,rhs);
            return *this;
        };


        TimeSeries& operator=(const TimeSeries& rhs){ // basic exception safety
            if( this != &rhs ) {
                this->_data = rhs._data;
                _meta = rhs._meta;
                _isLoaded = rhs._isLoaded;
            }
            return *this;
        };

        TimeSeries& operator=(TimeSeries&& rhs) { // move assignment

            if( this != &rhs ) //probably not necessary
            {
                _meta = std::move(rhs._meta);
                _data = std::move(rhs._data);
                _isLoaded = rhs._isLoaded;
                rhs._isLoaded = false;
            }
            return *this;
        };
        
        
        friend void swap(TimeSeries<T>& ts1, TimeSeries<T>& ts2)
        {
            using std::swap; // enable ADL
            
            swap( ts1._isLoaded, ts2._isLoaded );
            ts1._meta.swap( ts2._meta );
            ts1._data.swap( ts2._data );
        };
        
        
        // MUTATORS
        
        bool insert( const typename TimeMap::value_type& val ) {
            return _data.insert(val).second;
        }

        bool insert( typename TimeMap::value_type&& val ) { // move insertion
            return _data.insert(std::move(val)).second;
        }
        
        bool insert( time_t&& t, typename TimeMap::mapped_type&& mval ) { //inplace pair construction & move
            return _data.emplace(std::move(t),std::move(mval)).second;
        }
        
        
        // ITERATORS
        
        iterator begin() {
            return _data.begin();
        };
        
        iterator end() {
            return _data.end();
        };
        
        iterator rbegin() {
            return _data.rbegin();
        };
        
        const_iterator cbegin() const {
            return _data.cbegin();
        };
        
        const_iterator cend() const {
            return _data.cend();
        };

        const_iterator on(time_t tm) const { //get iterator by timestamp; returns end() if timestamp not found
             try {
                return _data.at(tm);
            }
            catch( const std::out_of_range &e ){
                return cend();
            }
        };

        
        // ACCESSORS
        
        typename TimeMap::mapped_type& operator[] (const typename TimeMap::key_type& k ){ //throws
            return _data.at( k );
        }
        
        bpt::ptime first() const {
            return bpt::from_time_t( _data.begin()->first );
        }
        
        bpt::ptime last() const {
            if( !size() )
                return bpt::from_time_t( _data.begin()->first );
            return bpt::from_time_t( _data.rbegin()->first );
        }

        std::vector<time_t> get_timestamps() { // returns a vector of all timestamps
            
            std::vector<time_t> ts;
            ts.reserve( _data.size() );
        
            boost::copy(_data | boost::adaptors::map_keys, std::back_inserter(ts));
            return ts;
        }


        // VALUES MEMBERSPACE
        
        struct Values {

        private:
            
            struct getDataPoint {
                T& operator() (const std::pair<time_t, T> &p) const
                {   //temporary hack to make this compile under clang/libc++ with C++11 support
                    return const_cast<T&>(p.second); }
            };
            
        public:
            
            friend TimeSeries<T>;
            
            typedef boost::transform_iterator<getDataPoint, typename TimeMap::iterator> iterator;
            typedef iterator const_iterator;
            
            iterator begin() {
                return boost::make_transform_iterator(owner.begin(),getDataPoint());
            }
            iterator end() {
                return boost::make_transform_iterator(owner.end(), getDataPoint());
            }
            
            const_iterator cbegin() {
                return boost::make_transform_iterator(owner.cbegin(),getDataPoint());
            }
            const_iterator cend() {
                return boost::make_transform_iterator(owner.cend(), getDataPoint());
            }
            
        private:
            
            Values( TimeSeries<T>& s ): owner(s){};
            TimeSeries<T>& owner; // keep an internal reference to the containing object
            
        } values;
        
        typedef typename Values::iterator value_iterator;
        
        
        // TIMESTAMPS MEMBERSPACE
        // to do: abstract out logic of both memberspaces
        
        struct TimeStamps {
        
        private:
            
            struct getTimeStamp {
                time_t& operator()(const std::pair<time_t, T> &p) const
                {   //temporary hack to make this compile under clang/libc++ with C++11 support
                    return const_cast<time_t&>(p.first); }
            };
            
        public:
            
            friend TimeSeries<T>;
            
            typedef boost::transform_iterator<getTimeStamp, typename TimeMap::iterator> iterator;
            typedef iterator const_iterator;
            
            iterator begin() {
                return boost::make_transform_iterator(owner._data.begin(), getTimeStamp() );
            }
            iterator end() {
                return boost::make_transform_iterator(owner._data.end(), getTimeStamp() );
            }
            
            const_iterator cbegin() {
             return boost::make_transform_iterator(owner._data.cbegin(),getTimeStamp() );
            }
            const_iterator cend() {
                return boost::make_transform_iterator(owner._data.cend(), getTimeStamp() );
            }
            
        private:
            
            TimeStamps( TimeSeries<T>& s ): owner(s){};
            TimeSeries<T>& owner; // keep an internal reference to the containing object for access
                    
        } timestamps;
        
        typedef typename TimeStamps::iterator time_iterator;
        
        
        // STATE RELATED
        
        bool isLoaded() const {
            return _isLoaded;
        }
        
        bool isEmpty() const {
            return _data.empty();
        }
        
        size_t size() const {
            return _data.size();
        }
        
        void clear() {
            _data.clear();
        }

        
        // META AND COLUMN INFORMATION
        
        std::vector<std::string> column_names() const {
            return dp::dp_names<T>();
        }
        
        std::string meta() const {
            return _meta;
        }
        
        void set_meta( const std::string& meta) {
            _meta.assign(meta);
        }
        
        void print_meta() {
            
            std::vector<std::string> cols = column_names();
            std::cout << std::endl;
            std::cout << "Meta/Name: "<<_meta<<std::endl;
            std::cout << "Dimensions: "<< size() <<" rows, "<< cols.size()+1 <<" columns"<< std::endl;
            std::cout << "Columns: ";
            std::copy( cols.begin(),cols.end(),std::ostream_iterator<std::string>(std::cout," "));
            std::cout << std::endl;
            std::cout << "First timestamp: " << first() << std::endl;
            std::cout << "Last timestamp: " << last() << std::endl;
        }
                   
    // DATA MEMBERS
        
    private:
        
        TimeMap _data;                  // internal data container
        std::string _meta;              // string with meta information
        bool _isLoaded;                 // load flag
    
        
    
    // TO DOs
    
    //get next closest iterator before given date; returns
    //const_iterator before(time_t) const {};
    
    //get an iterator to t timesteps after given datetime; returns .end() if out of range
    //const_iterator after(time_t,unsigned) const {};
    
    //get iterator on a given datetime, or the closest datetime after if not found
    //const_iterator on_or_after(time_t) const;
    
    //get iterator on a given date, or the closest date before if not found
    //const_iterator on_or_before(time_t) const;
        
    // returns estimated frequency - tbc
    //bpt::time_duration frequency(){
    //    return bpt::time_duration();
    //}
    
    // COLUMN ACCESSORS & ITERATORS - tbc
    
    // returns a vector of given column
    //std::vector<double> get_column(const std::string& col_name)
    //{
    // NOT DONE YET
    //   std::vector<double> column;
    //   column.reserve( _data.size() );
    
    //   boost::copy(_data | boost::adaptors::map_values, std::back_inserter(column));
    //   return column;
    //}
    
    
    // RESAMPLING & RELATED - tbc
    
    // resamples the series in place to unix_timestamp frequency
    //void resample(time_t freq){
    // use boost filtered range for this
    //boost::copy( rng | boost::adaptors::filtered(pred), out );
    //}
    
    // resamples the series to boost::ptime time_duration frequency
    //void resample(bpt::time_duration freq){
    //if( freq > this->frequency() )
    //  throw TimeSeriesException("");
    //    }

    
    }; // TimeSeries class
    
    
    
} // namespace timeseries




#endif
