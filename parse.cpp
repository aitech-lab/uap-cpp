#include <iostream>
#include <vector>
#include <chrono>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <regex>

using namespace std;

#include "UaParser.h"
using namespace uap_cpp;

#include "json.hpp"
using json=nlohmann::json;

#include "base64.h"

// UserAgentParser p("../uap-core/regexes.yaml");
UserAgentParser uap("regexes.yaml");


inline void parse_fp(stringstream& ss, json fp) {
    string s;
    // c0 = unpack("d", comps[0]) // double
    // c1 = unpack("i", comps[1]) // signed int
    // c2 = unpack("I", comps[2]) // unsigned int 
    // c3 = unpack("B", comps[3]) // unsigned byte
    json c = fp["components"];

    s = base64_decode(c[0]);
    cerr << s.size() << "\t";
    double c0[s.size()/sizeof(double)];
    memcpy(c0, s.data(), s.size());
    
    s = base64_decode(c[1]);
    cerr << s.size() << "\t";
    int16_t c1[s.size()/sizeof(int16_t)];
    memcpy(c1, s.data(), s.size());

    s = base64_decode(c[2]);
    cerr << s.size() << "\t";
    uint16_t c2[s.size()/sizeof(uint16_t)];
    memcpy(c2, s.data(), s.size());

    s = base64_decode(c[3]);
    cerr << s.size() << "\n";
    uint8_t c3[s.size()/sizeof(uint8_t)];
    memcpy(c3, s.data(), s.size());

    for(double   v : c0) ss << v << "\t";
    for(int16_t  v : c1) ss << v << "\t";
    for(uint16_t v : c2) ss << v << "\t";
    for(uint8_t  v : c3) ss << unsigned(v) << "\t";
}

inline void parse_json(stringstream& ss, string data) {
    
    if(data[0] == '"') {
        data = regex_replace(data, regex("^\"|\"$"), "");
        data = regex_replace(data, regex("\\\\\\\""), "\"");
        data = regex_replace(data, regex("\\\\\\\""), "\"");
    }

    json j;

    try {
        j = json::parse(data);
        if (!j["err"].is_string()) {
            parse_fp(ss, j["fp"]);
        }
    } 
    catch (...) {
        cerr << "ERROR" << endl;
        cerr << data << endl;
        exit(1);
    };

}

inline void parse_ua(stringstream& ss, string data) {

    auto ua = uap.parse(data);
    ss  << ua.browser.family      << "\t"
        << ua.browser.major       << "\t"
        << ua.browser.minor       << "\t"
        << ua.browser.patch       << "\t"
        << ua.browser.patch_minor << "\t"
        << ua.os.family           << "\t"
        << ua.os.major            << "\t"
        << ua.os.minor            << "\t"
        << ua.os.patch            << "\t"
        << ua.os.patch_minor      << "\t"
        << ua.device.family       << "\t"
        << ua.device.brand        << "\t"
        << ua.device.model        << "\t";
}

inline void parse_line(stringstream& ss, string line) {
    istringstream is(line);
    vector<string> tokens;
    string token;
    while(getline(is, token, '\t')) {
        if(!token.empty()) {
            tokens.push_back(token);
        }
    }
    if(tokens.size()==6) tokens.push_back("");
    if(tokens.size()!=7) return;
    ss << tokens[0] << "\t"
       << tokens[1] << "\t"
       << tokens[2] << "\t"
       << tokens[3] << "\t";
    parse_ua(ss, tokens[4]);
    parse_json(ss, tokens[5]);
    ss << tokens[6] << "\t";
}

int main(int argc, char** argv)
{
    
    ifstream fin("../test.tsv");
    for(string line; getline(fin, line);) {
        stringstream ss;
        parse_line(ss, line);
        // cout << ss.str() << endl;
    };

    // while(true) {
    //     vector< future<string> > results;
    //     size_t i=0;
    //     for(string line; getline(fin, line);) {
    //     // for(string line; getline(cin, line);) {
    //         results.push_back(async(&parse_line, line));
    //         if(i==10000) break;
    //         i++;   
    //     }
    //     if (results.size()==0) break;

    //     // out results
    //     for(auto&& result: results) {
    //         string r = result.get();
    //         cout << r << endl;
    //     }
    // }
        
    return 0;
}
