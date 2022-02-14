
#include <iostream>
#include <ctime>
#include <cmath>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();
int     filterProcess(std::string sInputFilePath, std::string sOutputFilePath, CCombFilterIf::CombFilterType_t combFilterType, float fDelayInSec, float fGain, int blockSize);

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string sInputFilePath,                 //!< file paths
                sOutputFilePath;

    static const int kBlockSize = 1024;

    clock_t time = 0;

    float **ppfAudioData = 0;
    float **ppfAudioOutputData = 0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioOutputFile = 0;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    CCombFilterIf           *phCombFilter = 0;
    
    CCombFilterIf::CombFilterType_t combFilterType;
    
    // params for filter
    float                   fDelayInSec = 0;
    float                   fGain = 1;
    float                   fMaxDelayInS = 1;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    // arguments [input path, filter type, delay time, gain]
        
    if (argc < 5 && argc != 1)
    {
        cout << "Missing arguments!";
        return -1;
    }
    else
    {
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + "_combFilter.wav";
        
        if (strcmp(argv[2], "FIR"))
        {
            combFilterType = CCombFilterIf::CombFilterType_t::kCombFIR;
        } else if (strcmp(argv[2], "IIR"))
        {
            combFilterType = CCombFilterIf::CombFilterType_t::kCombIIR;
        } else
        {
            cout << "Incorrect filters - FIR or IIR";
            return -1;
        }
        
        fDelayInSec = atof(argv[3]);
        fGain = atof(argv[4]);
    }
    // run filtering
    return filterProcess(sInputFilePath, sOutputFilePath, combFilterType, fDelayInSec, fGain, kBlockSize);
}

int filterProcess(std::string sInputFilePath, std::string sOutputFilePath, CCombFilterIf::CombFilterType_t combFilterType, float fDelayInSec, float fGain, int blockSize)
{

    static const int kBlockSize = blockSize;

    clock_t time = 0;

    float **ppfAudioData = 0;
    float **ppfAudioOutputData = 0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioOutputFile = 0;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    CCombFilterIf           *phCombFilter = 0;
    
    // params for filter
    float                   fMaxDelayInS = 1;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    CAudioFileIf::create(phAudioOutputFile);
    
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stFileSpec);
    
    phAudioOutputFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    
    if (!phAudioFile->isOpen())
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    if (!phAudioOutputFile->isOpen())
        {
            cout << "Wave file initialization error!";
            CAudioFileIf::destroy(phAudioOutputFile);
            return -1;
        }

    //////////////////////////////////////////////////////////////////////////////
    // initialize the comb filter
    CCombFilterIf::create(phCombFilter);
    phCombFilter -> init(combFilterType, fMaxDelayInS, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    
    // set parameters of the comb filter
    CCombFilterIf::FilterParam_t param = CCombFilterIf::FilterParam_t::kParamDelay;
    phCombFilter -> setParam(param, fDelayInSec);
    param = CCombFilterIf::FilterParam_t::kParamGain;
    phCombFilter -> setParam(param, fGain);
    
    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];
    
    ppfAudioOutputData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioOutputData[i] = new float[kBlockSize];

    if (ppfAudioData == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    if (ppfAudioData[0] == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    
    cout << "memory allocated";

    time = clock();

    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output file
    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames = kBlockSize;

        // read data (iNumOfFrames might be updated!)
        phAudioFile -> readData(ppfAudioData, iNumFrames);
        
        // apply filtering
        phCombFilter -> process(ppfAudioData, ppfAudioOutputData, iNumFrames);
        
        // write to file
        phAudioOutputFile->writeData(ppfAudioOutputData, iNumFrames);
        
        cout << "\r" << "reading and writing";
    }

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioOutputFile);
    CCombFilterIf::destroy(phCombFilter);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfAudioData[i];
        delete[] ppfAudioOutputData[i];
    }
    
    delete[] ppfAudioData;
    delete[] ppfAudioOutputData;
    
    ppfAudioData = 0;
    ppfAudioOutputData = 0;

    // all done
    return 0;

}

void test1()
{
    
}

void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

