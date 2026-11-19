#include "../Source/FactoryPresets.h"
#include <cassert>
#include <set>
#include <string>
int main() {
    auto& a=loveeq::factoryPresets();assert(a.size()==100);
    std::set<std::string> names;
    for (std::size_t i=0;i<a.size();++i) {
        assert(names.insert(a[i].name).second);
        auto p=loveeq::makePreset(i);
        int active=0;
        for(const auto& band:p) if(band.enabled) {++active;assert(band.hz>=20 && band.hz<=20000);}
        assert(active>=3);
    }
}
