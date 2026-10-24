#include "l1parser.hpp"
#include <cassert>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

int main()
{
    char temp[] = "/tmp/l1parser-hwifi-XXXXXX";
    const char *dir = mkdtemp(temp);
    assert(dir);
    const std::string base(dir);
    const std::string init = base + "/init.dat";
    const std::string l1 = base + "/l1profile.dat";

    auto check = [&](const std::string& profile, const std::string& band0,
                     const std::string& band1) {
        std::ofstream(l1) << "Default\nINDEX0=MT7992\nINDEX0_main_ifname=ra0;rai0\n"
                            "INDEX0_profile_path=" << profile << "\n";
        L1Parser parser;
        assert(parser.load(l1));
        assert(parser.if2dat("ra0").value_or("") == band0);
        assert(parser.if2dat("rai0").value_or("") == band1);
    };

    std::ofstream(init) << "Default\nBN0_profile_path=" << base << "/b0.dat\n"
                          "BN1_profile_path=" << base << "/b1.dat # comment\n";
    check(init, base + "/b0.dat", base + "/b1.dat");
    check(base + "/b0.dat;" + base + "/b1.dat", base + "/b0.dat", base + "/b1.dat");
    std::ofstream(init) << "Default\nChannel=1\n";
    check(init, init, "");
    std::ofstream(init) << "Default\nBN0_profile_path=" << base << "/b0.dat\n";
    check(init, base + "/b0.dat", "");
    std::filesystem::remove_all(base);
    std::cout << "PASS: HWIFI BN profiles, legacy lists, single DAT and missing band\n";
}
