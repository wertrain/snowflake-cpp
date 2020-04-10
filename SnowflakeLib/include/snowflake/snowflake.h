#include <cinttypes>

namespace snowflake
{

class Snowflake
{
public:
    Snowflake();
    Snowflake(const int datacenterId, const int workerId);

    int64_t Generate();

private:
    time_t m_lastTimestamp;
    int32_t m_sequence;
    int32_t m_datacenterId;
    int32_t m_workerId;
};

/// algorithm like snowflake to generate id
int64_t generate();
int64_t generate(const int datacenterId, const int workerId);

} // namespace snowflake
