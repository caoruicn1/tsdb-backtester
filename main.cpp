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

#include <iostream>
#include <ctime>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "lib/tsdb.hpp"
#include "lib/timeseries.hpp"
#include "lib/utilities.hpp"

using namespace timeseries;
using namespace datapoint;

int main(int argc, const char * argv[])
{

    //------------------------------------
    // TSDB interface connection
    //------------------------------------
    
    string user = "root";
    string pw = "";
    
    tsdb::Interface ifc(user,pw);
    
    try {
        
        ifc.connect();
        ifc.print_connection_info();
        ifc.print_metadata();
        
        ifc.disconnect();
        ifc.print_connection_info();
        
        ifc.connect();
        
    }
    catch(tsdb::TSDBInterfaceException& ex) {
        std::cout << ex.what() << endl;
    }
    
    
    //------------------------------------
    // TimeSeries Instantiation
    //------------------------------------
    
    // load 2 years worth of min interval data into an open/high/low/close series
    TimeSeries<OHLC> ts1("ts1");
    
    bpt::ptime start = bpt::time_from_string("2010-10-18 9:30:00");
    bpt::ptime end = bpt::time_from_string("2012-10-18 16:30:00");
    
    ifc.load(ts1, "ts_1_817289", start, end);
    ts1.print_meta();
    
    // move construct
    TimeSeries<OHLC> ts2( TimeSeries<OHLC>("ts2") );
    ifc.load(ts2, "ts_1_817289", start);
    ts2.print_meta();
    
    
    //------------------------------------
    // Some Examples
    //------------------------------------

    double results[ts1.size()];
    
    // output all timestamps
    std::copy( ts1.timestamps.begin(), ts1.timestamps.end(), std::ostream_iterator<long>(cout, ", "));
    
    // get the trading range for all one minute intervals
    
    auto range = [](OHLC a){ return (a.high - a.low); };
    std::transform( ts1.values.begin(), ts1.values.end(), results, range );

    // get a binary returns discretization of the series
    
    auto discretize = [](OHLC a)->short { return (a.close - a.open) > 0 ? 1 : 0; };
    std::transform( ts1.values.begin(), ts1.values.end(), results, discretize );
    
    // accumulate returns over series
    
    std::vector< std::pair<time_t,double> > rets;
    rets.reserve(ts1.size());
    
    for( TimeSeries<OHLC>::iterator it = ts1.begin(); it != ts1.end(); ++it)
        rets.push_back( std::make_pair(it->first, it->second.close-(--it)->second.close ));
    
}





