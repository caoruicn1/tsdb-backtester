//
//  timeseries.cpp
//  backtester
//
//  Created by Andreas Fragner on 1/25/13.
//  Copyright (c) 2013 Andreas Fragner. All rights reserved.
//

#include "timeseries_poly.hpp"

using namespace timeseries;


// STATICS

// static member _columns initialized based on template parameter

//const std::list<std::string> TimeSeriesOHLC::_columns   = dp::datapoint_columns<dp::OHLC>();
const vector<string> TimeSeriesOHLCV::_columns  = dp::dp_columns<dp::OHLCV>();
const vector<string> TimeSeriesBidAsk::_columns = dp::dp_columns<dp::BidAsk>();


// *****************************************************************
//
// TIME SERIES BASE CLASS
//
// *****************************************************************


TimeSeries::TimeSeries( const std::string& meta )
:   _meta(meta),
    _isLoaded(false),
    _columns(),
    _data(),
    _frequency()
{};


// META AND COLUMN INFORMATION

std::list<std::string> TimeSeries::columns() const {
    return _columns;
}

void TimeSeries::set_columns( const std::list<std::string>& col_names ){
    _columns.assign(col_names.begin(),col_names.end());
}

std::string TimeSeries::meta() const {
    return _meta;
}

void TimeSeries::set_meta( const std::string& meta) {
    _meta.assign(meta);
}


// TIME RELATED

bpt::ptime TimeSeries::first() const {
    return bpt::from_time_t( _data.begin()->first );
}

bpt::ptime TimeSeries::last() const {
    return bpt::from_time_t( _data.rbegin()->first );
}

bpt::time_duration TimeSeries::frequency(){
    return bpt::time_duration();
}

std::vector<std::time_t> TimeSeries::timestamps() const
{
    std::vector<std::time_t> keys;
    keys.reserve( _data.size() );
    
    boost::copy(_data | boost::adaptors::map_keys, std::back_inserter(keys));
    return keys;
}


// STATE RELATED

bool TimeSeries::isLoaded() const{
    return _isLoaded;
}

bool TimeSeries::isEmpty() const{
    return _data.empty();
};



// COLUMN ACCESSORS & ITERATORS

std::vector<double> TimeSeries::get_column(const std::string& col_name)
{
    // NOT DONE YET
    std::vector<double> column;
    column.reserve( _data.size() );
    
    boost::copy(_data | boost::adaptors::map_values, std::back_inserter(column));
    return column;
}



// *****************************************************************
//
// TIME SERIES DERIVED CLASS
//
// *****************************************************************


TimeSeriesOHLC::TimeSeriesOHLC(const std::string& meta)
: TimeSeries(meta)
{
    _columns = dp::dp_columns<dp::OHLC>();
};








