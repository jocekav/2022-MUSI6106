#if !defined(__CombFilter_hdr__)
#define __CombFilter_hdr__

#include "ErrorDef.h"

/*! \brief class for the comb filter (FIR & IIR)
*/
class CCombFilterBase: public CombFilterIf
{
    public: 
        CCombFilterBase(int iMaxDelaySamples, int iNumChannels);
        virtual ~CCombFilterBase();

        Error_t setParam(eParam, float fParamValue) override;
        float getParam(eParam) override;

        virtual Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames);
    
    private:
        CCombFilter(const CCombFilter& that);
}

class CCombFilterFIR: public CCombFilterBase
{
    CCombFilterFIR (int iMaxDelaySamples, int iNumChannels):CCombFilterBase(iMaxDelayInFrames, iNumChannels);
    virtual ~CCombFilterFIR ();

    Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) override;
}

class CCombFilterIIR: public CCombFilterBase
{
    CCombFilterIIR (int iMaxDelaySamples, int iNumChannels):CCombFilterBase(iMaxDelayInFrames, iNumChannels);
    virtual ~CCombFilterIIR ();

    Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) override;
}

#endif // #if !defined(__CombFilter_hdr__)