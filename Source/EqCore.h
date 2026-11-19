#pragma once
#include <algorithm>
#include <cmath>
#include <complex>

namespace loveeq {
constexpr double pi = 3.1415926535897932384626433832795;
enum class Type { Bell, LowShelf, HighShelf, HighPass, LowPass, Notch, BandPass };
struct Coefficients {
    double b0 = 1.0, b1 = 0.0, b2 = 0.0, a1 = 0.0, a2 = 0.0;
};
struct Biquad {
    Coefficients c;
    double z1 = 0.0, z2 = 0.0;
    void set(Coefficients next) noexcept { c = next; }
    void reset() noexcept { z1 = z2 = 0.0; }
    float process(float sample) noexcept {
        const double out = c.b0 * sample + z1;
        z1 = c.b1 * sample - c.a1 * out + z2;
        z2 = c.b2 * sample - c.a2 * out;
        if (!std::isfinite(out) || !std::isfinite(z1) || !std::isfinite(z2)) { reset(); return 0.0f; }
        return static_cast<float>(out);
    }
};
inline Coefficients normalize(double b0,double b1,double b2,double a0,double a1,double a2) noexcept {
    if (!std::isfinite(a0) || a0 <= 1e-12) return {};
    return {b0/a0,b1/a0,b2/a0,a1/a0,a2/a0};
}
inline Coefficients design(Type type, double sampleRate, double hz, double gainDb, double q, bool firstOrder=false) noexcept {
    const double sr = std::max(sampleRate, 8000.0);
    const double f = std::clamp(hz, 15.0, 0.475*sr);
    const double Q = std::clamp(q, 0.15, 18.0);
    if (firstOrder && (type == Type::HighPass || type == Type::LowPass)) {
        const double k = std::tan(pi*f/sr);
        const double norm = 1.0 / (1.0+k);
        return type == Type::LowPass ? Coefficients{k*norm,k*norm,0,(k-1.0)*norm,0}
                                      : Coefficients{norm,-norm,0,(k-1.0)*norm,0};
    }
    const double w = 2.0*pi*f/sr;
    const double cs=std::cos(w), sn=std::sin(w), alpha=sn/(2.0*Q);
    const double A=std::pow(10.0,std::clamp(gainDb,-36.0,36.0)/40.0);
    switch (type) {
        case Type::Bell:
            return normalize(1.0+alpha*A,-2.0*cs,1.0-alpha*A,
                             1.0+alpha/A,-2.0*cs,1.0-alpha/A);
        case Type::LowShelf: case Type::HighShelf: {
            const double beta=2.0*std::sqrt(A)*std::sqrt(2.0)*sn/2.0;
            if (type==Type::LowShelf)
                return normalize(A*((A+1)-(A-1)*cs+beta),2*A*((A-1)-(A+1)*cs),
                                 A*((A+1)-(A-1)*cs-beta),
                                 (A+1)+(A-1)*cs+beta,-2*((A-1)+(A+1)*cs),
                                 (A+1)+(A-1)*cs-beta);
            return normalize(A*((A+1)+(A-1)*cs+beta),-2*A*((A-1)+(A+1)*cs),
                             A*((A+1)+(A-1)*cs-beta),
                             (A+1)-(A-1)*cs+beta,2*((A-1)-(A+1)*cs),
                             (A+1)-(A-1)*cs-beta);
        }
        case Type::HighPass:
            return normalize((1+cs)/2,-(1+cs),(1+cs)/2,1+alpha,-2*cs,1-alpha);
        case Type::LowPass:
            return normalize((1-cs)/2,1-cs,(1-cs)/2,1+alpha,-2*cs,1-alpha);
        case Type::Notch:
            return normalize(1,-2*cs,1,1+alpha,-2*cs,1-alpha);
        case Type::BandPass:
            return normalize(alpha,0,-alpha,1+alpha,-2*cs,1-alpha);
    }
    return {};
}
inline double magnitudeDb(Coefficients c, double sampleRate, double hz) noexcept {
    const double w=2.0*pi*std::clamp(hz,0.0,0.5*sampleRate)/sampleRate;
    const std::complex<double> z=std::polar(1.0,-w), z2=z*z;
    const auto n=c.b0+c.b1*z+c.b2*z2;
    const auto d=1.0+c.a1*z+c.a2*z2;
    return 20.0*std::log10(std::max(1e-12,std::abs(n)/std::max(1e-12,std::abs(d))));
}
inline int stagesForSlope(int choice) noexcept { return std::clamp((choice+2)/2,1,4); }
inline bool needsFirstOrder(int choice) noexcept { return choice==0 || choice==2; }
}
