// standard headers
#include <cassert>
#include <iostream>

// project headers
#include "MUSI6106Config.h"

#include "ErrorDef.h"
#include "Util.h"

#include "CombFilterIf.h"
#include "CombFilter.h"

CCombFilterBase::CCombFilterBase(int iMaxDelaySamples, int iNumChannels):
    m_ppRingBuffer(0),
    m_iNumChannels(iNumChannels),
    m_iMaxDelaySamples(iDelaySamples),
    m_iDelayTimeSamples(1),
    m_fGain(0.5f)
{
    m_ppRingBuffer = new CRingBuffer<float>* [iNumChannels];
    for (int i = 0; i < iNumChannels; i++)
    {
        m_ppRingBuffer[i] = new CRingBuffer<float>(iMaxDelaySamples);
    }
}

CCombFilterBase::~CCombFilterBase()
{
    for (int i = 0; i < m_iNumChannels; i++)
    {
        delete m_ppRingBuffer[i];
    }
    delete [] m_ppRingBuffer;
    m_ppRingBuffer = 0;
}

Error_t CCombFilterBase::setParam(eParam, float fParamValue)
{
    switch (eParam)
    {
        case kParamDelay:
            CCombFilterBase::m_iDelayTimeSamples = fParamValue;
            for (int i = 0; i < m_iNumChannels; i++)
            {
                m_ppRingBuffer[i] -> setWriteIdx((int)fParamValue);
                m_ppRingBuffer[i] -> setReadIdx(0);
            }
        case kParamGain:
            CCombFilterBase::m_fGain = fParamValue;
    }
    return kNoError;
}

Error_t CCombFilterBase::getParam(eParam)
{
    switch (eParam)
    {
        case kParamDelay:
            return m_iDelayTimeSamples;
        case kParamGain:
            return m_fGain;
    }
}

CCombFilterFIR::CCombFilterFIR(int iMaxDelaySamples, int iNumChannels) : CCombFilterBase(iMaxDelaySamples, iNumChannels)
{

}

Error_t CCombFilterFIR::process(float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) 
{
    for (int i = 0; i < CCombFilterBase::m_iNumChannels; i++)
    {
        for (int j = 0; j < iNumberOfFrames; j++)
        {
            ppfOutputBuffer[i][j] = ppfInputBuffer[i][j] + CCombFilterBase::m_fGain * CCombFilterBase::m_ppRingBuffer[i]->getPostInc();
            CCombFilterBase::m_ppRingBuffer[i]->putPostInc(ppfInputBuffer[i][j])
        }
    }
    return kNoError;
}

CCombFilterFIR::CCombFilterIIR(int iMaxDelaySamples, int iNumChannels) : CCombFilterBase(iMaxDelaySamples, iNumChannels)
{

}

Error_t CCombFilterIIR::process(float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) 
{
    for (int i = 0; i < CCombFilterBase::m_iNumChannels; i++)
    {
        for (int j = 0; j < iNumberOfFrames; j++)
        {
            ppfOutputBuffer[i][j] = ppfInputBuffer[i][j] + CCombFilterBase::m_fGain * CCombFilterBase::m_ppRingBuffer[i]->getPostInc();
            CCombFilterBase::m_ppRingBuffer[i]->putPostInc(ppfOutputBuffer[i][j])
        }
    }
    return kNoError;
}

