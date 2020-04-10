#include "snowflake/snowflake.h"

#include <ctime>
#include <chrono>

namespace
{

// ���� kTwitterEpoch�i Twitter �̍̔ԊJ�n���� UNIX time�j �̓~���b���܂߂� 13 ���� UNIX time �ł�����
// time() �Ŏ擾�ł���o�ߕb���� 10 ���� UNIX time�i Windows ���̂݁H�j�ł͐������v�Z�ł��Ȃ��̂Œ���
static constexpr long long kTwitterEpoch = 1288834974657L; // Thu Nov 04 10:42:54 JST 2010

// �}�V�� ID �̒�`
// �f�[�^�Z���^�[ ID �ƃ��[�J�[ ID ���킹�� 10 �r�b�g
static constexpr int kWorkerIdBits = 5;       ///< ���[�J�[ID �̃r�b�g��
static constexpr int kDatacenterIdBits = 5;   ///< �f�[�^�Z���^�[ID �̃r�b�g��
static constexpr long kMaxWorkerId = -1L ^ (-1L << kWorkerIdBits);          ///< ���[�J�[ ID �̍ő�l
static constexpr long kMaxDatacenterId = -1L ^ (-1L << kDatacenterIdBits);  ///< �f�[�^�Z���^�[ ID �̍ő�l

// �V�[�P���X�l�̒�`
static const int kSequenceBits = 12; ///< �V�[�P���X�̃r�b�g��

static const int kWorkerIdShift = kSequenceBits;
static const int kDatacenterIdShift = kSequenceBits + kWorkerIdBits;
static const int kTimestampLeftShift = kSequenceBits + kWorkerIdBits + kDatacenterIdBits;
static const long kSequenceMask = -1L ^ (-1L << kSequenceBits);

int64_t Timestamp()
{
    timespec ts;
    timespec_get(&ts, TIME_UTC);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::nanoseconds{ ts.tv_nsec });
    return (ts.tv_sec * 1000) + ms.count();
}

}

namespace snowflake
{

Snowflake::Snowflake()
    : Snowflake(0, 0)
{

}

Snowflake::Snowflake(const int datacenterId, const int workerId)
    : m_lastTimestamp(0)
    , m_sequence(0)
    , m_datacenterId(datacenterId)
    , m_workerId(workerId)
{

}

int64_t Snowflake::Generate()
{
    int64_t timestamp = Timestamp();

    // ���Ԃ̊����߂���`�F�b�N
    if (timestamp < m_lastTimestamp)
    {
        return 0;
    }

    // ���ꎞ�Ԃɕ����� ID �𐶐����Ă��邩
    if (m_lastTimestamp == timestamp)
    {
        m_sequence = (m_sequence + 1) & kSequenceMask;
        
        // ���ꎞ�Ԃɐ����ł���V�[�P���X���𒴂���
        if (m_sequence == 0)
        {
            timestamp = Timestamp();
        }
    }
    else
    {
        m_sequence = 0;
    }

    m_lastTimestamp = timestamp;

    return ((timestamp - kTwitterEpoch) << kTimestampLeftShift) | (m_datacenterId << kDatacenterIdShift) | (m_workerId << kWorkerIdShift) | m_sequence;
}

//-----------------------------------------------------------------------------

int64_t generate()
{
    Snowflake snowflake;
    return snowflake.Generate();
}

int64_t generate(const int datacenterId, const int workerId)
{
    Snowflake snowflake(datacenterId, workerId);
    return snowflake.Generate();
}

} // namespace snowflake
