
#include <iostream>
#include <ctime>
#include <cmath>
# include <assert.h>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();
int     filterProcess(std::string sInputFilePath, std::string sOutputFilePath, CCombFilterIf::CombFilterType_t combFilterType, float fDelayInSec, float fGain, int blockSize);
void    realAudio1();
void    realAudio2();
void    test1();
void    test2();
void    test3(CCombFilterIf::CombFilterType_t combFilterType);
void    test4(CCombFilterIf::CombFilterType_t combFilterType);
void    test5(CCombFilterIf::CombFilterType_t combFilterType);

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
        realAudio1();
        realAudio2();
        
        cout << "Test 1" << endl;
        test1();
        
        cout << "Test 2" << endl;
        test2();
        
        cout << "Test 3 - FIR" << endl;
        test3(CCombFilterIf::CombFilterType_t::kCombFIR);
        
        cout << "Test 3 - IIR" << endl;
        test3(CCombFilterIf::CombFilterType_t::kCombIIR);
        
        cout << "Test 4 - FIR" << endl;
        test4(CCombFilterIf::CombFilterType_t::kCombFIR);
        
        cout << "Test 4 - IIR" << endl;
        test4(CCombFilterIf::CombFilterType_t::kCombIIR);
        
        cout << "Test 5 - FIR" << endl;
        test5(CCombFilterIf::CombFilterType_t::kCombFIR);
        
        cout << "Test 5 - IIR" << endl;
        test5(CCombFilterIf::CombFilterType_t::kCombIIR);
        
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
        CAudioFileIf::destroy(phAudioOutputFile);
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    if (!phAudioOutputFile->isOpen())
        {
            cout << "Wave file initialization error!";
            CAudioFileIf::destroy(phAudioOutputFile);
            CAudioFileIf::destroy(phAudioFile);
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
        
        return -1;
    }
    if (ppfAudioData[0] == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        
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

void realAudio1()
{
    std::string sInputFilePath = "../../audio/bagpipe.wav";
                
    std::string sOutputFilePath = "../../audio/bagpipe_FIR.wav";

    int blockSize = 1024;
    
    int samplingRate = 22000;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::CombFilterType_t::kCombFIR;
    
    // params for filter
    float                   fDelayInSec = 100 / samplingRate;
    float                   fGain = 0.5;
    
    filterProcess(sInputFilePath, sOutputFilePath, combFilterType, fDelayInSec, fGain, blockSize);
    
}

void realAudio2()
{
    std::string sInputFilePath = "../../audio/acomic.wav";
                
    std::string sOutputFilePath = "../../audio/acomic_IIR.wav";

    int blockSize = 1024;
    
    int samplingRate = 22000;
    
    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::CombFilterType_t::kCombIIR;
    
    // params for filter
    float                   fDelayInSec = 100 / samplingRate;
    float                   fGain = 0.5;
    
    filterProcess(sInputFilePath, sOutputFilePath, combFilterType, fDelayInSec, fGain, blockSize);
}

void test1()
{
    std::string sInputFilePath = "../../audio/sine440.wav";
                
    std::string sOutputFilePath = "../../audio/sine440_test1.wav";

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
        long long iNumFrames = blockSize;

        phAudioOutputFile -> readData(ppfAudioOutputData, iNumFrames);
        
        // check if output is 0
        for (int i = 0; i < stFileSpec.iNumChannels; i++)
        {
            for (int j = 0; j < iNumFrames; j++)
            {
                if (ppfAudioOutputData[i][j] != 0)
                {
                    cout << "Test 1 Failed" << endl;
                    
                    // clean-up (close files and free memory)
                    CAudioFileIf::destroy(phAudioOutputFile);

                    for (int i = 0; i < stFileSpec.iNumChannels; i++)
                    {
                        delete[] ppfAudioOutputData[i];
                    }
                    
                    delete[] ppfAudioOutputData;
                    
                    ppfAudioOutputData = 0;
                    
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
    std::string sInputFilePath = "../../audio/sine440.wav";

    std::string sOutputFilePath = "../../audio/sine440_test2.wav";

    int blockSize = 1024;

    CCombFilterIf::CombFilterType_t combFilterType = CCombFilterIf::CombFilterType_t::kCombIIR;

    // params for filter
    float                   fFreq = 440;
    float                   fDelayInSec = 1 / fFreq;
    float                   fGain = 1;

    filterProcess(sInputFilePath, sOutputFilePath, combFilterType, fDelayInSec, fGain, blockSize);


    // Check Output - should be scaled
    float **ppfAudioData = 0;
    float **ppfAudioOutputData = 0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioOutputFile = 0;
    CAudioFileIf::FileSpec_t stFileSpec;


    phAudioOutputFile -> create(phAudioOutputFile);
    phAudioOutputFile -> openFile(sOutputFilePath, CAudioFileIf::kFileRead);

    phAudioOutputFile -> getFileSpec(stFileSpec);
    ppfAudioOutputData = new float* [stFileSpec.iNumChannels];

    phAudioFile -> create(phAudioFile);
    phAudioFile -> openFile(sInputFilePath, CAudioFileIf::kFileRead);
    
    long long iLengthInFrames = 0;
    phAudioFile -> getLength(iLengthInFrames);

    phAudioFile -> getFileSpec(stFileSpec);
    ppfAudioData = new float* [stFileSpec.iNumChannels];

    // allocate array
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        ppfAudioOutputData[i] = new float[blockSize];
        ppfAudioData[i] = new float[blockSize];
    }
    
    int         iSamplingRate = stFileSpec.fSampleRateInHz;
    int         iCycleInSamples = int(iSamplingRate / fFreq);

    while (!phAudioOutputFile -> isEof() || !phAudioFile -> isEof())
    {
        long long iNumFrames = blockSize;

        phAudioOutputFile -> readData(ppfAudioOutputData, iNumFrames);
        phAudioFile -> readData(ppfAudioData, iNumFrames);

        for (int i = 0; i < stFileSpec.iNumChannels; i++)
        {
            float fCurrGain = 1;
            for (int j = iCycleInSamples + 1; j < iLengthInFrames; j++)
            {
                if (ppfAudioData[i][j] != 0)
                {
                    if (abs(ppfAudioOutputData[i][j] / ppfAudioData[i][j]) - fCurrGain < 1)
                    {
                        fCurrGain = abs(ppfAudioOutputData[i][j] / ppfAudioData[i][j]);
                        
                        cout << "Test 2 Failed" << endl;
                        
                        // clean-up (close files and free memory)
                        CAudioFileIf::destroy(phAudioFile);
                        CAudioFileIf::destroy(phAudioOutputFile);

                        for (int i = 0; i < stFileSpec.iNumChannels; i++)
                        {
                            delete[] ppfAudioData[i];
                            delete[] ppfAudioOutputData[i];
                        }

                        delete[] ppfAudioData;
                        delete[] ppfAudioOutputData;

                        ppfAudioData = 0;
                        ppfAudioOutputData = 0;
                        
                        return;
                    }
                    fCurrGain = abs(ppfAudioOutputData[i][j] / ppfAudioData[i][j]);
                }
            }
        }
    }
    
        cout << "Test 2 Passed" << endl;

    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioOutputFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfAudioData[i];
        delete[] ppfAudioOutputData[i];
    }

    delete[] ppfAudioData;
    delete[] ppfAudioOutputData;

    ppfAudioData = 0;
    ppfAudioOutputData = 0;
}


void test3(CCombFilterIf::CombFilterType_t combFilterType)
{
    std::string sInputFilePath = "../../audio/sine440.wav";
                
    std::string sOutputFilePath1 = "../../audio/sine440_test3_1.wav";
    std::string sOutputFilePath2 = "../../audio/sine440_test3_2.wav";
    
    // params for filter
    float                   fDelayInSec = 0.1;
    float                   fGain = 0.5;
    
    int blockSize = 512;
    
    filterProcess(sInputFilePath, sOutputFilePath1, combFilterType, fDelayInSec, fGain, blockSize);
    
    blockSize = 1024;
    
    filterProcess(sInputFilePath, sOutputFilePath2, combFilterType, fDelayInSec, fGain, blockSize);


    // Check Output - should be scaled
    float **ppfAudioData1 = 0;
    float **ppfAudioData2 = 0;

    CAudioFileIf *phAudioFile1 = 0;
    CAudioFileIf *phAudioFile2 = 0;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    
    phAudioFile1 -> create(phAudioFile1);
    phAudioFile1 -> openFile(sOutputFilePath1, CAudioFileIf::kFileRead);

    phAudioFile1 -> getFileSpec(stFileSpec);
    ppfAudioData1 = new float* [stFileSpec.iNumChannels];
    
    phAudioFile2 -> create(phAudioFile2);
    phAudioFile2 -> openFile(sOutputFilePath2, CAudioFileIf::kFileRead);

    phAudioFile2 -> getFileSpec(stFileSpec);
    ppfAudioData2 = new float* [stFileSpec.iNumChannels];
    
    // allocate array
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        ppfAudioData1[i] = new float[blockSize];
        ppfAudioData2[i] = new float[blockSize];
    }

    while (!phAudioFile1 -> isEof() || !phAudioFile2 -> isEof())
    {
        long long iNumFrames = blockSize;

        phAudioFile1 -> readData(ppfAudioData1, iNumFrames);
        phAudioFile2 -> readData(ppfAudioData2, iNumFrames);
        
        // check if outputs are the same
        for (int i = 0; i < stFileSpec.iNumChannels; i++)
        {
            for (int j = 0; j < iNumFrames; j++)
            {
                if (ppfAudioData1[i][j] != ppfAudioData2[i][j])
                {
                    cout << "Test 3 Failed" << endl;
                    
                    // clean-up (close files and free memory)
                    CAudioFileIf::destroy(phAudioFile1);
                    CAudioFileIf::destroy(phAudioFile2);

                    for (int i = 0; i < stFileSpec.iNumChannels; i++)
                    {
                        delete[] ppfAudioData1[i];
                        delete[] ppfAudioData2[i];
                    }
                    
                    delete[] ppfAudioData1;
                    delete[] ppfAudioData2;
                    
                    ppfAudioData1 = 0;
                    ppfAudioData2 = 0;
                    
                    return;
                }
            }
        }
    }
        cout << "Test 3 Passed" << endl;

    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile1);
    CAudioFileIf::destroy(phAudioFile2);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfAudioData1[i];
        delete[] ppfAudioData2[i];
    }
    
    delete[] ppfAudioData1;
    delete[] ppfAudioData2;
    
    ppfAudioData1 = 0;
    ppfAudioData2 = 0;
}

void test4(CCombFilterIf::CombFilterType_t combFilterType)
{
    std::string sInputFilePath = "../../audio/silence.wav";
                
    std::string sOutputFilePath = "../../audio/silence_test4.wav";
    
    // params for filter
    float                   fDelayInSec = 0.1;
    float                   fGain = 0.5;
    
    int blockSize = 1024;
    
    filterProcess(sInputFilePath, sOutputFilePath, combFilterType, fDelayInSec, fGain, blockSize);
    


    // Check Output - should be scaled
    float **ppfAudioData = 0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    
    phAudioFile -> create(phAudioFile);
    phAudioFile -> openFile(sOutputFilePath, CAudioFileIf::kFileRead);

    phAudioFile -> getFileSpec(stFileSpec);
    ppfAudioData = new float* [stFileSpec.iNumChannels];

    // allocate array
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        ppfAudioData[i] = new float[blockSize];
    }

    while (!phAudioFile -> isEof())
    {
        long long iNumFrames = blockSize;

        phAudioFile -> readData(ppfAudioData, iNumFrames);
        
        // check if outputs are the same
        for (int i = 0; i < stFileSpec.iNumChannels; i++)
        {
            for (int j = 0; j < iNumFrames; j++)
            {
                if (ppfAudioData[i][j] != 0)
                {
                    cout << "Test 4 Failed" << endl;
                    
                    // clean-up (close files and free memory)
                    CAudioFileIf::destroy(phAudioFile);

                    for (int i = 0; i < stFileSpec.iNumChannels; i++)
                    {
                        delete[] ppfAudioData[i];
                    }
                    
                    delete[] ppfAudioData;
                    
                    ppfAudioData = 0;
                    
                    return;
                }
            }
        }
    }
        cout << "Test 4 Passed" << endl;

    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfAudioData[i];
    }
    
    delete[] ppfAudioData;
    
    ppfAudioData = 0;
}

// check if filter params are 0 - output should equal input
void test5(CCombFilterIf::CombFilterType_t combFilterType)
{
    std::string sInputFilePath = "../../audio/sine440.wav";
                
    std::string sOutputFilePath = "../../audio/sine440_test5.wav";

    int blockSize = 1024;
    
    // params for filter
    float                   fDelayInSec = 0;
    float                   fGain = 0;
    
    filterProcess(sInputFilePath, sOutputFilePath, combFilterType, fDelayInSec, fGain, blockSize);


    // Check Output - should be scaled
    float **ppfAudioData = 0;
    float **ppfAudioOutputData = 0;

    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioOutputFile = 0;
    CAudioFileIf::FileSpec_t stFileSpec;
    
    
    phAudioOutputFile -> create(phAudioOutputFile);
    phAudioOutputFile -> openFile(sOutputFilePath, CAudioFileIf::kFileRead);

    phAudioOutputFile -> getFileSpec(stFileSpec);
    ppfAudioOutputData = new float* [stFileSpec.iNumChannels];
    
    phAudioFile -> create(phAudioFile);
    phAudioFile -> openFile(sInputFilePath, CAudioFileIf::kFileRead);

    phAudioFile -> getFileSpec(stFileSpec);
    ppfAudioData = new float* [stFileSpec.iNumChannels];
    
    // allocate array
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        ppfAudioOutputData[i] = new float[blockSize];
        ppfAudioData[i] = new float[blockSize];
    }

    while (!phAudioOutputFile -> isEof() || !phAudioFile -> isEof())
    {
        long long iNumFrames = blockSize;

        phAudioOutputFile -> readData(ppfAudioOutputData, iNumFrames);
        phAudioFile -> readData(ppfAudioData, iNumFrames);
        
        // check if output is scaled
        int skipDelayLine = int(fDelayInSec * stFileSpec.fSampleRateInHz / iNumFrames);
        for (int i = 0; i < stFileSpec.iNumChannels; i++)
        {
            for (int j = skipDelayLine; j < iNumFrames; j++)
            {
                if (ppfAudioOutputData[i][j] != ppfAudioData[i][j])
                {
                    cout << "Test 5 Failed" << endl;
                    
                    // clean-up (close files and free memory)
                    CAudioFileIf::destroy(phAudioFile);
                    CAudioFileIf::destroy(phAudioOutputFile);

                    for (int i = 0; i < stFileSpec.iNumChannels; i++)
                    {
                        delete[] ppfAudioData[i];
                        delete[] ppfAudioOutputData[i];
                    }
                    
                    delete[] ppfAudioData;
                    delete[] ppfAudioOutputData;
                    
                    ppfAudioData = 0;
                    ppfAudioOutputData = 0;
                    
                    return;
                }
            }
        }
    }
    cout << "Test 5 Passed" << endl;

    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioOutputFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfAudioData[i];
        delete[] ppfAudioOutputData[i];
    }
    
    delete[] ppfAudioData;
    delete[] ppfAudioOutputData;
    
    ppfAudioData = 0;
    ppfAudioOutputData = 0;
}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

