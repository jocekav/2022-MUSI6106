
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
void    test1();
void    test2();

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
    if (argc == 1)
    {
        cout << "Test 1" << endl;
        test1();
        
        cout << "Test 2" << endl;
        test2();
        
        return 0;
        
    } else if (argc < 5 && argc != 1)
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
        
        // run filtering
        return filterProcess(sInputFilePath, sOutputFilePath, combFilterType, fDelayInSec, fGain, kBlockSize);
    }
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
        
    }

    cout << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

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
    std::string sInputFilePath = "/Users/jocekav/Documents/GitHub/2022-MUSI6106/sine440.wav";
                
    std::string sOutputFilePath = "/Users/jocekav/Documents/GitHub/2022-MUSI6106/sine440_test1.wav";

    int blockSize = 1024;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::CombFilterType_t::kCombFIR;
    
    // params for filter
    float                   fDelayInSec = 1;
    float                   fGain = -1;
    
    filterProcess(sInputFilePath, sOutputFilePath, combFilterType, fDelayInSec, fGain, blockSize);
    
    // Check Output - should be 0
    CAudioFileIf *phAudioOutputFile = 0;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    float **ppfAudioOutputData = 0;
    
    phAudioOutputFile -> create(phAudioOutputFile);
    phAudioOutputFile -> openFile(sOutputFilePath, CAudioFileIf::kFileRead);

    phAudioOutputFile -> getFileSpec(stFileSpec);

    ppfAudioOutputData = new float* [stFileSpec.iNumChannels];
    
    // allocate array
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        ppfAudioOutputData[i] = new float[blockSize];
    }

    while (!phAudioOutputFile -> isEof())
    {
        long long iNumFrames = blockSize; //number of frames to read at a time
        //read in 1024 frames
        //store in ppfOutput
        phAudioOutputFile -> readData(ppfAudioOutputData, iNumFrames);
        
        // check if output is 0
        for (int i = 0; i < stFileSpec.iNumChannels; i++) //compare the two arrays
        {
            for (int j = 0; j < iNumFrames; j++)
            {
                if (ppfAudioOutputData[i][j] != 0)
                {
                    cout << "Test 1 Failed" << endl;
                    return;
                }
            }
        }
    }
        cout << "Test 1 Passed" << endl;

    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioOutputFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfAudioOutputData[i];
    }
    
    delete[] ppfAudioOutputData;
    
    ppfAudioOutputData = 0;

}

void test2()
{
    std::string sInputFilePath = "/Users/jocekav/Documents/GitHub/2022-MUSI6106/sine440.wav";
                
    std::string sOutputFilePath = "/Users/jocekav/Documents/GitHub/2022-MUSI6106/sine440_test2.wav";

    int blockSize = 1024;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::CombFilterType_t::kCombIIR;
    
    // params for filter
    float                   fDelayInSec = 0.5;
    float                   fGain = 1;
    
    filterProcess(sInputFilePath, sOutputFilePath, combFilterType, fDelayInSec, fGain, blockSize);
}

void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

