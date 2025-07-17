/*
  ==============================================================================

    FMAlgorithmRouter.h
    Created: 7 Apr 2025 8:20:46pm
    Author:  majab

  ==============================================================================
*/

#pragma once
#include "OscData.h"

class FMAlgorithmRouter
{
public:

    static float processAlgorithm(int algorithmIndex,
        OscData& osc1, OscData& osc2, OscData& osc3, OscData& osc4,
        float env1, float env2, float env3, float env4);
};
