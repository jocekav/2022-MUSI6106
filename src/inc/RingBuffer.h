#if !defined(__RingBuffer_hdr__)
#define __RingBuffer_hdr__

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstring>

/*! \brief implement a circular buffer of type T
*/
template <class T> 
class CRingBuffer
{
public:
    explicit CRingBuffer(int iBufferLengthInSamples) :
        m_iBuffLength(iBufferLengthInSamples),
        m_iReadIdx(0),
        m_iWriteIdx(0),
        m_ptBuff(0),
        m_fullFlag(false)
    {
        assert(iBufferLengthInSamples > 0);

        // allocate and init

        m_ptBuff = new T[m_iBuffLength];

    }

    virtual ~CRingBuffer()
    {
        // free memory
        delete [] m_ptBuff;
        m_ptBuff = 0;

    }

    /*! add a new value of type T to write index and increment write index
    \param tNewValue the new value
    \return void
    */
    void putPostInc (T tNewValue)
    {

        put(tNewValue);
        
        // if full, increment the read index to overwrite old values
        if (m_fullFlag) {
            m_iReadIdx++;
            if (m_iReadIdx == m_iBuffLength) {
                m_iReadIdx = 0;
            }
        }

        // increment write index
        m_iWriteIdx++;

        // check if out of bounds -> wrap around
        if (m_iWriteIdx == m_iBuffLength) {
            m_iWriteIdx = 0;
        }
        // check if full
        m_fullFlag = m_iReadIdx == m_iWriteIdx;
    }

    /*! add a new value of type T to write index
    \param tNewValue the new value
    \return void
    */
    void put(T tNewValue)
    {
        m_ptBuff[m_iWriteIdx] = tNewValue;
    }
    
    /*! return the value at the current read index and increment the read pointer
    \return float the value from the read index
    */
    T getPostInc()
    {
        // check if empty and return error
        if (!m_fullFlag && m_iReadIdx == m_iWriteIdx) {
            return -1;
        }

        T val = get();

        // increment read index
        m_iReadIdx++;

        // check if out of bounds -> wrap around
        if (m_iReadIdx == m_iBuffLength) {
            m_iReadIdx = 0;
        }

        return val;
    }

    /*! return the value at the current read index
    \return float the value from the read index
    */
    T get() const
    {
        return m_ptBuff[m_iReadIdx];
    }
    
    /*! set buffer content and indices to 0
    \return void
    */
    void reset()
    {
        for (int i = 0; i < m_iBuffLength - 1; i++) {
            m_ptBuff[i] = 0;
        }

        m_iReadIdx = 0;
        m_iWriteIdx = 0;
    }

    /*! return the current index for writing/put
    \return int
    */
    int getWriteIdx() const
    {
        return m_iWriteIdx;
    }

    /*! move the write index to a new position
    \param iNewWriteIdx: new position
    \return void
    */
    void setWriteIdx(int iNewWriteIdx)
    {
        m_iWriteIdx = iNewWriteIdx;
    }

    /*! return the current index for reading/get
    \return int
    */
    int getReadIdx() const
    {
        return m_iReadIdx;
    }

    /*! move the read index to a new position
    \param iNewReadIdx: new position
    \return void
    */
    void setReadIdx(int iNewReadIdx)
    {
        m_iReadIdx = iNewReadIdx;
    }

    /*! returns the number of values currently buffered (note: 0 could also mean the buffer is full!)
    \return int
    */
    int getNumValuesInBuffer() const
    {

        int diff = m_iWriteIdx - m_iReadIdx;

        // when write is ahead of read
        if (diff > 0) {
            return diff;
        } // when read is ahead of write
        else if (diff < 0) {
            return (m_iBuffLength + diff);
        }
        else {
            // return buffer length if full
            if (m_fullFlag) {
                return m_iBuffLength;
            } else {
            // return 0 if empty
                return 0;
            }
        }
    }

    /*! returns the length of the internal buffer
    \return int
    */
    int getLength() const
    {
        return m_iBuffLength;
    }

private:
    CRingBuffer();
    CRingBuffer(const CRingBuffer& that);

    int m_iBuffLength;              //!< length of the internal buffer
    
    // increment a buffer

    int m_iReadIdx;
    int m_iWriteIdx;
    float* m_ptBuff;
    bool m_fullFlag;
};
#endif // __RingBuffer_hdr_