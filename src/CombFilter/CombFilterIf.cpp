
// standard headers

// project headers
#include "MUSI6106Config.h"

#include "ErrorDef.h"
#include "Util.h"

#include "CombFilterIf.h"
#include "CombFilter.h"

static const char*  kCMyProjectBuildDate = __DATE__;


CCombFilterIf::CCombFilterIf () :
    m_bIsInitialized(false),
    m_pCCombFilter(0),
    m_fSampleRate(0)
{
    // this should never hurt
    this->reset ();
}


CCombFilterIf::~CCombFilterIf ()
{
    this->reset ();
}

const int  CCombFilterIf::getVersion (const Version_t eVersionIdx)
{
    int iVersion = 0;

    switch (eVersionIdx)
    {
    case kMajor:
        iVersion    = MUSI6106_VERSION_MAJOR; 
        break;
    case kMinor:
        iVersion    = MUSI6106_VERSION_MINOR; 
        break;
    case kPatch:
        iVersion    = MUSI6106_VERSION_PATCH; 
        break;
    case kNumVersionInts:
        iVersion    = -1;
        break;
    }

    return iVersion;
}
const char*  CCombFilterIf::getBuildDate ()
{
    return kCMyProjectBuildDate;
}

Error_t CCombFilterIf::create (CCombFilterIf*& pCCombFilter)
{
    pCCombFilter = new CCombFilterBase();

    return Error_t::kNoError;
}

Error_t CCombFilterIf::destroy (CCombFilterIf*& pCCombFilter)
{
    pCCombFilter -> reset();
    
    delete pCCombFilter;
    pCCombFilter = 0;

    return Error_t::kNoError;
}

Error_t CCombFilterIf::init (CombFilterType_t eFilterType, float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels)
{
    reset();

    m_eFilterType = eFilterType;
    m_fMaxDelayLengthInS = fMaxDelayLengthInS;
    m_fSampleRate = fSampleRateInHz;
    m_iNumChannels = iNumChannels;
    m_fGain = 1;

    int iDelaySamples = fMaxDelayLengthInS * fSampleRateInHz;
    
    m_pCCombFilter -> CCombFilterBase::init(iDelaySamples, iNumChannels);
    
    switch(eFilterType)
    {
        case kCombFIR:
            m_pCCombFilter = static_cast<CCombFilterBase*> (new CCombFilterFIR (iDelaySamples, iNumChannels));
            break;
        case kCombIIR:
            m_pCCombFilter = static_cast<CCombFilterBase*> (new CCombFilterIIR (iDelaySamples, iNumChannels));
            break;
    }

    m_bIsInitialized = true;

    return Error_t::kNoError;
}

Error_t CCombFilterIf::reset ()
{

    m_fMaxDelayLengthInS = 0;
    m_fSampleRate = 0;
    m_iNumChannels = 0;
    m_fGain = 0;
    
    m_bIsInitialized = false;

    return Error_t::kNoError;
}

Error_t CCombFilterIf::process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{

    return m_pCCombFilter -> process(ppfInputBuffer, ppfOutputBuffer, iNumberOfFrames);
}

Error_t CCombFilterIf::setParam (FilterParam_t eParam, float fParamValue)
{
    switch (eParam)
    {
        case kParamDelay:
            return m_pCCombFilter -> setParam(eParam, fParamValue * m_fSampleRate);
        case kParamGain:
            return m_pCCombFilter -> setParam(eParam, fParamValue);
    }

    return Error_t::kNoError;
}

float CCombFilterIf::getParam (FilterParam_t eParam) const
{
    switch (eParam)
    {
        case kParamDelay:
            return m_pCCombFilter -> getParam(eParam) / m_fSampleRate;
        case kParamGain:
            return m_pCCombFilter -> getParam(eParam);
    }

    return 0;
}
