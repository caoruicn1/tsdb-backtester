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

#include "datapoint.hpp"

using namespace datapoint;

const std::string DataPointException::base_msg = "DataPoint Exception: ";

const std::map<int,std::string> DataPointException::messages {
    {1000, "Vector initialization failed. Index out of range."},
    {0, "Unknown DataPoint Exception."}};

DataPoint::~DataPoint(){};


OHLC::OHLC(double o, double h, double l, double c)
:   open(o), high(h), low(l), close(c)
{ };

OHLC::OHLC( const std::vector<double>& init ) //fixed arg ordering assumed
try:
    open(init.at(0)), high(init.at(1)), low(init.at(2)), close(init.at(3))
{ }
catch( std::out_of_range& e){
    throw DataPointException(1000);
};



OHLCV::OHLCV(double o, double h, double l, double c, int v)
:   open(o), high(h), low(l), close(c), volume(v)
{ };

OHLCV::OHLCV( const std::vector<double>& init ) //fixed arg ordering assumed
try:
open(init.at(0)), high(init.at(1)), low(init.at(2)), close(init.at(3)), volume(init.at(4))
{ }
catch( std::out_of_range& e){
    throw DataPointException(1000);
};



BidAsk::BidAsk(double b, double a)
:   bid(b), ask(a)
{ };

BidAsk::BidAsk( const std::vector<double>& init ) //fixed arg ordering assumed
try:
bid(init.at(0)), ask(init.at(1))
{ }
catch( std::out_of_range& e){
    throw DataPointException(1000);
};



