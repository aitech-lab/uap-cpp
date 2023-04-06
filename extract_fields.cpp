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

inline void 
parse_ua(
    stringstream& ss, 
    string data) {

    auto ua = uap.parse(data);
    
    ss 
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


inline void 
parse_header(
    stringstream& ss, 
    json header) {
    
    // parse and allocate url object.
    url u(header["Referer"][0]);
    ss << "\t" << u.host;
    ss << "\t" << u.path;
}


inline void 
parse_json(
    stringstream& ss, 
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
    
    parse_header(ss, j["_http_headers"]);
    
    ss << "\t" << j["dur"];
    
}


inline void 
parse_line(
    stringstream& ss, 
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

    parse_ua  (ss, tokens[4]);
    parse_json(ss, tokens[5]);

}


inline void 
process_stream(
    istream& st) {
    for(string line; getline(st, line);) {
        stringstream ss;

        ss << setprecision(17);
        
        try {
            parse_line(ss, line);
            cout << ss.str() << "\n";
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
