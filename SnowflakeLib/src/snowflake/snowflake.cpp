#include "snowflake/snowflake.h"

#include <ctime>
#include <chrono>

namespace
{

// この kTwitterEpoch（ Twitter の採番開始時の UNIX time） はミリ秒も含めた 13 桁の UNIX time であって
// time() で取得できる経過秒数の 10 桁の UNIX time（ Windows 環境のみ？）では正しく計算できないので注意
static constexpr long long kTwitterEpoch = 1288834974657L; // Thu Nov 04 10:42:54 JST 2010

// マシン ID の定義
// データセンター ID とワーカー ID 合わせて 10 ビット
static constexpr int kWorkerIdBits = 5;       ///< ワーカーID のビット数
static constexpr int kDatacenterIdBits = 5;   ///< データセンターID のビット数
static constexpr long kMaxWorkerId = -1L ^ (-1L << kWorkerIdBits);          ///< ワーカー ID の最大値
static constexpr long kMaxDatacenterId = -1L ^ (-1L << kDatacenterIdBits);  ///< データセンター ID の最大値

// シーケンス値の定義
static const int kSequenceBits = 12; ///< シーケンスのビット数

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

    // 時間の巻き戻りをチェック
    if (timestamp < m_lastTimestamp)
    {
        return 0;
    }

    // 同一時間に複数の ID を生成しているか
    if (m_lastTimestamp == timestamp)
    {
        m_sequence = (m_sequence + 1) & kSequenceMask;
        
        // 同一時間に生成できるシーケンス数を超えた
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
