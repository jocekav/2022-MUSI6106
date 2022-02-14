#if !defined(__CombFilter_hdr__)
#define __CombFilter_hdr__

#include "ErrorDef.h"
#include "CombFilterIf.h"
#include "RingBuffer.h"


/*! \brief class for the comb filter (FIR & IIR)
*/
class CCombFilterBase: public CCombFilterIf
{
    public:
        CCombFilterBase();
        CCombFilterBase(int iMaxDelaySamples, int iNumChannels);
        virtual ~CCombFilterBase();
    
        Error_t init (int iMaxDelaySamples, int iNumChannels);

        Error_t setParam(FilterParam_t eParam, float fParamValue);
        float getParam(FilterParam_t eParam);
    

        virtual Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) = 0;
    
    protected:
        CRingBuffer<float>  **m_ppRingBuffer;
        int m_iDelayTimeSamples;
        int m_iNumChannels;
        int m_iMaxDelaySamples;
        float m_fGain;
    
    private:
        CCombFilterBase(const CCombFilterBase& that);
};

class CCombFilterFIR: public CCombFilterBase
{
    public:
        CCombFilterFIR (int iMaxDelaySamples, int iNumChannels):CCombFilterBase( iMaxDelaySamples, iNumChannels){};
//        CCombFilterFIR (int iMaxDelaySamples, int iNumChannels):CCombFilterBase(){};
        virtual ~CCombFilterFIR() {};

        Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) override;
        };

class CCombFilterIIR: public CCombFilterBase
{
    public:
        CCombFilterIIR (int iMaxDelaySamples, int iNumChannels):CCombFilterBase( iMaxDelaySamples, iNumChannels){};
//        CCombFilterIIR (int iMaxDelaySamples, int iNumChannels):CCombFilterBase(){};
        virtual ~CCombFilterIIR() {};

        Error_t process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames) override;
};

#endif // #if !defined(__CombFilter_hdr__)
