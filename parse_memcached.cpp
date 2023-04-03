#include <iostream>
#include <iomanip>
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

#include "metrics.h"

// {
// "_http_headers":{
//    "Accept":["*/*"],
//    "Accept-Encoding":["gzip, deflate, br"],
//    "Accept-Language":["ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7"],
//    "Connection":["close"],
//    "Content-Length":["2607"],
//    "Content-Type":["application/json;charset=UTF-8"],
//    "Origin":["https://domain.org"],
//    "Referer":["https://domain.org/"],
//    "Sec-Ch-Ua":["\\"Google Chrome\\";v=\\"111\\", \\"Not(A:Brand\\";v=\\"8\\", \\"Chromium\\";v=\\"111\\"],
//    "Sec-Ch-Ua-Mobile":["?1"],
//    "Sec-Ch-Ua-Platform":["\\"Android\\"],
//    "Sec-Fetch-Dest":["empty"],
//    "Sec-Fetch-Mode":["cors"],
//    "Sec-Fetch-Site":["cross-site"],
//    "User-Agent":["Mozilla/5.0 (Linux; Android 12; SM-A315F) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/111.0.0.0 Mobile Safari/537.36"],
//    "X-Add-Http-Headers":["1"],
//    "X-Forwarded-For":["77.45.187.11"],
//    "X-Pid":["2890426"],
//    "X-User-Id":["oYB32eJDRg8hrzCyvCovEA"]
//    }


struct url {
  public:
    url(const string& url_s) {
      this->parse(url_s);
    }
    string protocol, host, path, query;
  private:
    void parse(const string& url_s);
};


void url::parse(const string& url_s) {
    
    const string prot_end("://");
    string::const_iterator prot_i = search(
        url_s.begin(), 
        url_s.end(), 
        prot_end.begin(), 
        prot_end.end());

    protocol.reserve(distance(url_s.begin(), prot_i));
    transform(
        url_s.begin(), 
        prot_i,
        back_inserter(protocol),
        [](unsigned char c) { return tolower(c); }
    ); // protocol is icase
    if( prot_i == url_s.end()) return;
    
    advance(prot_i, prot_end.length());
    string::const_iterator path_i = find(prot_i, url_s.end(), '/');
    host.reserve(distance(prot_i, path_i));
    transform(
        prot_i, 
        path_i,
        back_inserter(host),
        [](unsigned char c) { return tolower(c); }
    ); // host is icase
    
    string::const_iterator query_i = find(path_i, url_s.end(), '?');
    path.assign(path_i, query_i);
    if(query_i != url_s.end()) ++query_i;

    query.assign(query_i, url_s.end());
}


inline void parse_header(
    stringstream& key_ss, 
    stringstream& value_ss, 
    json header) {
	// parse and allocate url object.
	url u(header["Referer"][0]);
	value_ss << "\t" << u.host;
	value_ss << "\t" << u.path;
}


inline void parse_fp(
    stringstream& key_ss,
    stringstream& value_ss,
    json fp) {

    string s;
    
    // c0 = unpack("d", comps[0]) // double
    // c1 = unpack("i", comps[1]) // signed int
    // c2 = unpack("I", comps[2]) // unsigned int 
    // c3 = unpack("B", comps[3]) // unsigned byte
    json c = fp["components"];

    uint32_t 
        fp_hash=0, 
        fp_stable_hash=0, 
        fp_unstable_hash=0;

    string data = c.dump();
    fp_hash = MurmurHash2(data.data(), data.size(), SEED);

    s = base64_decode(c[0]);
    // cerr << s.size() << "\t";
    double 
        c0[s.size()/sizeof(double)],
        c0_stable[s.size()/sizeof(double)];
    memcpy(c0, s.data(), s.size());
    memcpy(c0_stable, s.data(), s.size());
    // reset unstable metrics
    for (int&i : metrics_v01::fp_unstable_block_0) c0_stable[i] = 0.0;
    fp_stable_hash = fp_stable_hash ^ MurmurHash2(&c0_stable, sizeof(c0_stable), SEED);

    s = base64_decode(c[1]);
    // cerr << s.size() << "\t";
    int32_t c1[s.size()/sizeof(int32_t)];
    memcpy(c1, s.data(), s.size());

    s = base64_decode(c[2]);
    // cerr << s.size() << "\t";
    uint32_t 
        c2[s.size()/sizeof(uint32_t)], 
        c2_stable[s.size()/sizeof(uint32_t)];
    memcpy(c2, s.data(), s.size());
    memcpy(c2_stable, s.data(), s.size());
    // reset unstable metrics
    for (int&i : metrics_v01::fp_unstable_block_2) c2_stable[i]=0;
    fp_stable_hash = fp_stable_hash ^ MurmurHash2(&c2_stable, sizeof(c2_stable), SEED);

    s = base64_decode(c[3]);
    // cerr << s.size() << "\n";u
    uint8_t c3[s.size()/sizeof(uint8_t)];
    memcpy(c3, s.data(), s.size());
        
    stringstream unstable_hash_ss;
    for (int&i : metrics_v01::fp_unstable_block_0) unstable_hash_ss << c0[i] << ",";
    for (int&i : metrics_v01::fp_unstable_block_2) unstable_hash_ss << c2[i] << ",";
    string unstable_hash = unstable_hash_ss.str();
    fp_unstable_hash = MurmurHash2(unstable_hash.data(), unstable_hash.size(), SEED);

    key_ss << "\t" << fp_hash;
    key_ss << "\t" << fp_stable_hash;
    key_ss << "\t" << fp_unstable_hash;
    
    for(double   v : c0) value_ss << "\t" << v;
    for(int32_t  v : c1) value_ss << "\t" << v;
    for(uint32_t v : c2) value_ss << "\t" << v;
    for(uint8_t  v : c3) value_ss << "\t" << unsigned(v);

}


inline void parse_json(
    stringstream& key_ss, 
    stringstream& value_ss, 
    string data) {
    
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
        parse_header(key_ss, value_ss, j["_http_headers"]);
        parse_fp    (key_ss, value_ss, j["fp"]);
    } else {
        parse_header(key_ss, value_ss, j["_http_headers"]);
        key_ss << "\t" << 0; // fp_hash;
        key_ss << "\t" << 0; // fp_stable_hash;
        key_ss << "\t" << 0; // fp_unstable_hash;
    } 

}


inline void parse_ua(
    stringstream& key_ss, 
    stringstream& value_ss, 
    string data) {

    uint32_t ua_hash = MurmurHash2(data.data(), data.size(), SEED);
    string ua_chars  = regex_replace(data, regex("[^A-za-z]*"), "");
    uint32_t ua_chars_hash = MurmurHash2(ua_chars.data(), ua_chars.size(), SEED);
    string ua_digits = regex_replace(data, regex("[^0-9]*"), "");
    uint32_t ua_digits_hash = MurmurHash2(ua_digits.data(), ua_digits.size(), SEED);
    
    key_ss
        // << "\t" << ua_hash
        << "\t" << ua_hash
        << "\t" << ua_chars_hash
        << "\t" << ua_digits_hash
    ;

    auto ua = uap.parse(data);
    
    value_ss 
        << "\t" << ua.browser.family
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
        << "\t" << ua.device.model;
}

inline void parse_ip(
    stringstream& key_ss, 
    stringstream& value_ss, 
    string ip) {
    
    string net24 = regex_replace(ip, regex("\\.\\d*$"), ".0");
    string net16 = regex_replace(ip, regex("\\.\\d+\\.\\d+$"), ".0.0");
    key_ss << "\t" << ip;
    key_ss << "\t" << net24;
    key_ss << "\t" << net16;

}

inline void parse_line(
    stringstream& key_ss, 
    stringstream& value_ss, 
    string line) {

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

    key_ss         << tokens[1]  // ts
           << "\t" << tokens[0]; // uuid
    value_ss   
       << "\t" << tokens[2]; // PIXEL
    
    parse_ua  (key_ss, value_ss, tokens[4]);
    parse_json(key_ss, value_ss, tokens[5]);
    value_ss << "\t" << tokens[6];

    // key_ss << "\t" << tokens[3]; // ip
    parse_ip(key_ss, value_ss, tokens[3]);

}


inline void process_stream(istream& st) {
    for(string line; getline(st, line);) {
        stringstream 
            key_ss, 
            value_ss;

        key_ss   << setprecision(17);
        value_ss << setprecision(17);
        
        try {
            parse_line(key_ss, value_ss, line);
            string value = value_ss.str();
            size_t value_size = value.size() * sizeof(std::string::value_type);
            // set key flags exptime bytes [noreply] 
            // value 
            // set key_1 0 86400 5\r\nabcde\r\n'
            cout << key_ss.str() << "\t" << value_size << "\t" << value << "\n";
        }
        catch (const exception &exc) {
            cerr << exc.what() << endl;
            // cerr << line << endl;
        };
    };
}


int main(int argc, char* argv[]) {
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
