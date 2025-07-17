/*
  ==============================================================================

    FMAlgorithmRouter.cpp
    Created: 7 Apr 2025 8:20:46pm
    Author:  majab

  ==============================================================================
*/

#include "FMAlgorithmRouter.h"
#include <JuceHeader.h>

float FMAlgorithmRouter::processAlgorithm(int algorithmIndex,
    OscData& osc1, OscData& osc2, OscData& osc3, OscData& osc4,
    float env1, float env2, float env3, float env4)
{
    float out1 = 0.0f, out2 = 0.0f, out3 = 0.0f, out4 = 0.0f;
    switch (algorithmIndex)
    {
        // algorytm 1: osc4 -> osc3 -> osc2 -> osc1
    case 0:
        out4 = osc4.getModulatedSample(0.0f, env4);
        out3 = osc3.getModulatedSample(out4, env3);
        out2 = osc2.getModulatedSample(out3, env2);
        out1 = osc1.getModulatedSample(out2, env1);
        return out1;

        // algorytm 2: (osc4 + osc3) -> osc2 -> osc1
    case 1:
        out4 = osc4.getModulatedSample(0.0f, env4);
        out3 = osc3.getModulatedSample(0.0f, env3);
        out2 = osc2.getModulatedSample(out4 + out3, env2);
        out1 = osc1.getModulatedSample(out2, env1);
        return out1;

        // algorytm 3: osc4 -> (osc3 + osc2) -> osc1
    case 2:
        out4 = osc4.getModulatedSample(0.0f, env4);
        out3 = osc3.getModulatedSample(out4, env3);
        out2 = osc2.getModulatedSample(out4, env2);
        out1 = osc1.getModulatedSample(out3 + out2, env1);
        return out1;

        // algorytm 4: osc4 -> osc3 -> (osc2 + osc1)
    case 3:
        out4 = osc4.getModulatedSample(0.0f, env4);
        out3 = osc3.getModulatedSample(out4, env3);
        out2 = osc2.getModulatedSample(out3, env2);
        out1 = osc1.getModulatedSample(out3, env1);
        return (out2 + out1) * 0.5f;

        // algorytm 5: (osc4 + osc3 + osc2) -> osc1
    case 4:
        out4 = osc4.getModulatedSample(0.0f, env4);
        out3 = osc3.getModulatedSample(0.0f, env3);
        out2 = osc2.getModulatedSample(0.0f, env2);
        out1 = osc1.getModulatedSample(out4 + out3 + out2, env1);
        return out1;

        // algorytm 6: osc4 -> (osc3 + osc2 + osc1)
    case 5:
        out4 = osc4.getModulatedSample(0.0f, env4);
        out3 = osc3.getModulatedSample(out4, env3);
        out2 = osc2.getModulatedSample(out4, env2);
        out1 = osc1.getModulatedSample(out4, env1);
        return (out3 + out2 + out1) / 3.0f;

        // algorytm 7: (osc4 + osc3) -> (osc2 + osc1)
    case 6:
    {
        out4 = osc4.getModulatedSample(0.0f, env4);
        out3 = osc3.getModulatedSample(0.0f, env3);
        float combinedMod = out4 + out3;
        out2 = osc2.getModulatedSample(combinedMod, env2);
        out1 = osc1.getModulatedSample(combinedMod, env1);
        return (out2 + out1) * 0.5f;
    }

        // algorytm 8: osc4 + osc3 + osc2 + osc1
    case 7:
        out4 = osc4.getModulatedSample(0.0f, env4);
        out3 = osc3.getModulatedSample(0.0f, env3);
        out2 = osc2.getModulatedSample(0.0f, env2);
        out1 = osc1.getModulatedSample(0.0f, env1);
        return (out4 + out3 + out2 + out1) * 0.25f;

    default:
        jassertfalse; // zly indeks algorytmu na wszelki
        return 0.0f;
    }
}