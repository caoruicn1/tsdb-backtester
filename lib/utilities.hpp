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
 * utilities.hpp
 *
 * Design Overview:
 *
 * Various utility functions, in particular for boost::posix_time conversion
 * and formatting.
 *
 */


#ifndef backtester_utilities_hpp
#define backtester_utilities_hpp

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace bpt = boost::posix_time;
namespace btg = boost::gregorian;
using namespace std;

namespace utilities {
    
    
    // NON-COPYABLE BASE CLASS 
    
    class Uncopyable{
        
    protected:
        Uncopyable(){};
        ~Uncopyable(){};
        
    private:
        Uncopyable(const Uncopyable&);
        Uncopyable& operator=(const Uncopyable&);
    };
    
    
    // DATETIME RELATED FUNCTIONALITY
    
    // pretty format for boost posix_time
   // string bpt_to_str( const bpt::ptime& );
    //string bpt_to_str( const bpt::ptime& );
    
    inline string bpt_to_str( const bpt::ptime &datetime )
    {
        string result = bpt::to_iso_extended_string(datetime);
        result.replace(10,1," "); //change the date-time separator
        return result;
    }

    // static global
    static bpt::ptime epoch(btg::date(1970, 1, 1));
    
    // converts a boost posix_time object to a unix timestamp
    inline time_t bpt_to_time_t(const bpt::ptime& pt)
    {
        bpt::time_duration diff(pt - epoch);
        return (diff.ticks() / diff.ticks_per_second());
    }
    
    
    // converts a string to a unix timestamp
    // throws if string cannot be converted to boost::ptime object

    inline time_t str_to_time_t(const string& str)
    {
        bpt::ptime pt( bpt::time_from_string(str) );
        return bpt_to_time_t(pt);
    }
    
    // for the reverse conversion use bpt::from_time_t() returning a bpt::ptime

}



#endif
