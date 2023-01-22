#include "UaParser.h"
using namespace uap_cpp;

#include <iostream>
#include <string>
using namespace std;

int main(void) {

    UserAgentParser p("uap-core/regexes.yaml");

    for (string line; getline(cin, line);) {
            UserAgent ua = p.parse(line);
            cout
            << ua.browser.family      << "\t"
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
            << ua.device.model        << "\t"
            << "\n";
    }
    return 0;
}
