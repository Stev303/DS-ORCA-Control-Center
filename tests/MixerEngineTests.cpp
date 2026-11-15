#include "MixerEngine.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
namespace {
bool Near(float a,float b,float e=0.0005f){return std::fabs(a-b)<=e;}
void Check(bool ok,const char* msg){if(!ok){std::cerr<<"FAIL: "<<msg<<'\n';std::exit(1);}}
}
int main(){
 using namespace dsorca; MixerEngine mixer;
 const float a[]={0.25f,-0.5f,0.5f,-0.25f}; const float* in[]={a}; float out[4]{};
 mixer.Process(in,1,out,2);
 Check(Near(out[0],0.25f)&&Near(out[1],-0.5f),"unity pass-through");
 Check(Near(mixer.GetMasterMeter().leftPeak,0.5f),"master peak meter");
 ChannelState s{}; s.gainDb=-6.0206f; Check(mixer.SetChannelState(0,s),"set channel");
 mixer.Process(in,1,out,2); Check(Near(out[0],0.125f),"-6 dB gain");
 s.muted=true; mixer.SetChannelState(0,s); mixer.Process(in,1,out,2);
 Check(Near(out[0],0.0f)&&Near(out[1],0.0f),"mute");
 s.muted=false; s.pan=1.0f; mixer.SetChannelState(0,s); mixer.Process(in,1,out,2);
 Check(Near(out[0],0.0f)&&Near(out[1],-0.25f), "hard-right balance");
 const float b[]={0.2f,0.2f,0.2f,0.2f}; const float* two[]={a,b};
 s.pan=0.0f; s.solo=true; mixer.SetChannelState(0,s); mixer.Process(two,2,out,2);
 Check(Near(out[0],0.125f),"solo excludes other strips");
 Check(!mixer.SetChannelState(kMaxMixerChannels,{}),"bounds check");
 mixer.SetMasterGainDb(6.0f); Check(Near(mixer.GetMasterGainDb(),6.0f),"master gain");
 mixer.ResetMeters(); Check(Near(mixer.GetMasterMeter().leftPeak,0.0f),"meter reset");
 std::cout<<"MixerEngine tests passed\n"; return 0;
}
