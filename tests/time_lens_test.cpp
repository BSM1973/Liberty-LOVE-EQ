#include "TimeLensCore.h"
#include <cmath>
#include <iostream>
#include <limits>
static int fail(const char* reason) { std::cerr<<reason<<"\n";return 1; }
int main() {
    using loveeq::timelens::Detector;
    using loveeq::timelens::Phase;
    for(double sr:{44100.0,48000.0,96000.0}) {
        Detector detector;
        detector.prepare(sr);
        detector.configure(25.f,350.f,-40.f);
        auto x=detector.process(0.f);
        if(x.phase!=Phase::Silence)return fail("A silent input must remain unprocessed");
        // Sound with a realistic fast transient and steady body.
        int firstAttack=-1,firstBody=-1;
        for(int n=0;n<(int)(sr*0.22);++n) {
            const float amp=0.31f;
            const float sample=amp*(float)std::sin(2.*3.141592653589793*440.*n/sr);
            x=detector.process(sample);
            if(x.phase==Phase::Attack&&firstAttack<0)firstAttack=n;
            if(x.phase==Phase::Body&&firstBody<0)firstBody=n;
            const float sum=x.stage[0]+x.stage[1]+x.stage[2];
            if(!std::isfinite(sum)||sum>1.00001f||sum<0.f)
                return fail("Time Lens weights must stay normalized and finite");
        }
        if(firstAttack<0||firstBody<0 || firstBody<firstAttack+(int)(sr*0.018))
            return fail("Attack-to-body transition missing or too early");
        if(x.phase!=Phase::Body)return fail("Sustain unexpectedly left the body stage");
        // Sharp note release: tail must be detected and allowed to fade out.
        bool seenTail=false;
        float firstTailWeight=0.f,lastTailWeight=0.f;
        for(int n=0;n<(int)(sr*0.15);++n) {
            x=detector.process(0.f);
            if(x.phase==Phase::Tail) {
                if(!seenTail)firstTailWeight=x.stage[2];
                seenTail=true;
                lastTailWeight=x.stage[2];
            }
        }
        if(!seenTail||firstTailWeight<0.7f)return fail("Note release not detected");
        if(lastTailWeight>=firstTailWeight)return fail("Tail weight must decay over time");
        // Silence clears the stage and never leaves a stuck filter.
        for(int n=0;n<(int)(sr*0.8);++n)x=detector.process(0.f);
        if(x.phase!=Phase::Silence || x.stage[0]!=0.f||x.stage[1]!=0.f||x.stage[2]!=0.f)
            return fail("Time Lens tail did not return to transparent silence");
        // Another note must start a new attack, not inherit the previous note's body.
        bool reAttack=false;
        for(int n=0;n<(int)(sr*0.07);++n) {
            x=detector.process(0.30f*(float)std::sin(2.*3.141592653589793*660.*n/sr));
            reAttack |= x.phase==Phase::Attack;
        }
        if(!reAttack)return fail("Second note did not restart Attack");
        detector.reset();
        x=detector.process(std::numeric_limits<float>::quiet_NaN());
        if(x.phase!=Phase::Silence || !std::isfinite(x.envelope))
            return fail("Nonfinite audio sample must be ignored");
    }
    std::cout<<"Time Lens Attack/Body/Tail envelope tests passed\n";
    return 0;
}
