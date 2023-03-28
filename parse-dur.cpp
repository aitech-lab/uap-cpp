#include <iostream>
#include <istream>
#include <vector>
#include <queue>
#include <memory>
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

// gzip
#include "zstr.hpp"


// gcc hash seed for murmur
// https://github.com/gcc-mirror/gcc/blob/41d6b10e96a1de98e90a7c0378437c3255814b16/libstdc%2B%2B-v3/include/bits/functional_hash.h#L191
#define SEED 0xc70f6907
#include "MurmurHash2.h"

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
    // cerr << s.size() << "\t";
    double c0[s.size()/sizeof(double)];
    memcpy(c0, s.data(), s.size());
    
    s = base64_decode(c[1]);
    // cerr << s.size() << "\t";
    int32_t c1[s.size()/sizeof(int32_t)];
    memcpy(c1, s.data(), s.size());

    s = base64_decode(c[2]);
    // cerr << s.size() << "\t";
    uint32_t c2[s.size()/sizeof(uint32_t)];
    memcpy(c2, s.data(), s.size());

    s = base64_decode(c[3]);
    // cerr << s.size() << "\n";u
    uint8_t c3[s.size()/sizeof(uint8_t)];
    memcpy(c3, s.data(), s.size());

    for(double   v : c0) ss << "\t" << v;
    for(int32_t  v : c1) ss << "\t" << v;
    for(uint32_t v : c2) ss << "\t" << v;
    for(uint8_t  v : c3) ss << "\t" << unsigned(v);
    
}

inline void parse_json(stringstream& ss, string data) {
    
    // unescape double escaped jsons
    if(data[0] == '"') {
        data = regex_replace(data, regex("^\"|\"$"), "");
        data = regex_replace(data, regex("\\\\\\\""), "\"");
        data = regex_replace(data, regex("\\\\\\\""), "\"");
        if (data[1]=='\\')
            data = regex_replace(data, regex("\\\\\\\""), "\"");
    }

    json j;

    j = json::parse(data);
    if (!j["err"].is_string()) {
        // parse_fp(ss, j["fp"]);
        int d = j["dur"];
        ss << "\t" << d;
    }

}

inline void parse_ua(stringstream& ss, string data) {

    uint32_t ua_hash = MurmurHash2(data.data(), data.size(), SEED);
    string ua_chars  = regex_replace(data, regex("[^A-za-z]*"), "");
    uint32_t ua_chars_hash = MurmurHash2(ua_chars.data(), ua_chars.size(), SEED);
    string ua_digits = regex_replace(data, regex("[^0-9]*"), "");
    uint32_t ua_digits_hash = MurmurHash2(ua_digits.data(), ua_digits.size(), SEED);
    
    ss << "\t" << ua_hash
       << "\t" << ua_chars_hash
       << "\t" << ua_digits_hash
    ;

    auto ua = uap.parse(data);
    
    ss  << "\t" << ua.browser.family
        << "\t" << ua.browser.major
        << "\t" << ua.browser.minor
        << "\t" << ua.browser.patch
        << "\t" << ua.browser.patch_minor
        << "\t" << ua.os.family
        << "\t" << ua.os.major
        << "\t" << ua.os.minor
        << "\t" << ua.os.patch
        << "\t" << ua.os.patch_minor
        << "\t" << ua.device.family
        << "\t" << ua.device.brand
        << "\t" << ua.device.model
    ;
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
    
    ss         << tokens[0] // first
       << "\t" << tokens[1]
       << "\t" << tokens[2]
       << "\t" << tokens[3]
    ;
    parse_ua(ss, tokens[4]);
    parse_json(ss, tokens[5]);
    ss << "\t" << tokens[6];
}

inline void process_stream(istream& st) {
    for(string line; getline(st, line);) {
        stringstream ss;
        try {
            parse_line(ss, line);
            cout << ss.str() << endl;
        }
        catch (const exception &exc) {
            cerr << exc.what() << endl;
            // cerr << line << endl;
        };
    };

}

int main(int argc, char* argv[])
{
    if(argc>1) {
        string gzip_file = argv[1];
        ifstream zst (gzip_file, ios::binary);
        zstr::istream st(zst);
        process_stream(st);
    } else {
        process_stream(cin);
    }
    
    return 0;
}
