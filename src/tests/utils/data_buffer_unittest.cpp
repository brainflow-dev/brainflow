#include <array>
#include <future>
#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <list>
#include <thread>

#include "data_buffer.h"

using namespace testing;


TEST (DataBufferTest, AddData_AddLessDataThanBufferCapacity_StoreAllData)
{
    DataBuffer buffer (4, 2);
    double values[4] = {1.0, 2.0, 3.0, 4.0};
    double retrieved[4];

    buffer.add_data (values);
    buffer.get_current_data (1, retrieved);

    EXPECT_EQ (buffer.get_data_count (), 1);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], values[i]);
    }
}

TEST (DataBufferTest, AddData_AddEqualDataToBufferCapacity_StoreAllData)
{
    DataBuffer buffer (4, 2);
    double first_values[4] = {1.0, 2.0, 3.0, 4.0};
    double second_values[4] = {5.0, 6.0, 7.0, 8.0};
    double retrieved[8];

    buffer.add_data (first_values);
    buffer.add_data (second_values);

    EXPECT_EQ (buffer.get_data_count (), 2);

    buffer.get_current_data (2, retrieved);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], first_values[i]);
        EXPECT_EQ (retrieved[i + 4], second_values[i]);
    }
}

TEST (DataBufferTest, AddData_AddMoreDataThanBufferCapacity_OverwriteOldestData)
{
    DataBuffer buffer (4, 2);
    double first_values[4] = {1.0, 2.0, 3.0, 4.0};
    double second_values[4] = {5.0, 6.0, 7.0, 8.0};
    double third_values[4] = {9.0, 10.0, 11.0, 12.0};
    double retrieved[4];

    buffer.add_data (first_values);
    buffer.add_data (second_values);
    buffer.add_data (third_values);

    buffer.get_current_data (1, retrieved);

    EXPECT_EQ (buffer.get_data_count (), 2);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], third_values[i]);
    }
}

TEST (DataBufferTest, AddData_BufferIsNotReady_DoNothing)
{
    DataBuffer buffer_zero (4, 0);
    double values[4] = {1.0, 2.0, 3.0, 4.0};
    double retrieved[4];

    buffer_zero.add_data (values);

    EXPECT_EQ (buffer_zero.get_data_count (), 0);

    buffer_zero.get_current_data (1, retrieved);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_NE (retrieved[i], values[i]);
    }
}

TEST (DataBufferTest, AddData_InvokeSimultaneouslyOnMultipleThreads_AddDataWithoutMixing)
{
    DataBuffer buffer (4, 1024);
    double values[4] = {1.0, 2.0, 3.0, 4.0};
    std::thread threads[1024];

    for (int i = 0; i < 1024; i++)
    {
        threads[i] = std::thread ([&] () { buffer.add_data (values); });
    }

    for (int i = 0; i < 1024; i++)
    {
        threads[i].join ();
    }

    double retrieved[4096];
    buffer.get_current_data (1024, retrieved);

    for (int i = 0; i < 1024; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            EXPECT_DOUBLE_EQ (retrieved[i * 4 + j], values[j]);
        }
    }
}

TEST (DataBufferTest, GetData_MaxCountLessThanAvailableCount_ReturnMaxCountBytes)
{
    DataBuffer buffer (4, 2);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    for (int i = 0; i < 2; i++)
    {
        buffer.add_data (values);
    }

    double retrieved[4];
    auto result = buffer.get_data (1, retrieved);

    EXPECT_EQ (result, 1);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], values[i]);
    }
}

TEST (DataBufferTest, GetData_MaxCountEqualToAvailableCount_ReturnAvailableBytes)
{
    DataBuffer buffer (4, 2);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    buffer.add_data (values);

    double retrieved[4];
    auto result = buffer.get_data (1, retrieved);

    EXPECT_EQ (result, 1);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], values[i]);
    }
}

TEST (DataBufferTest, GetData_MaxCountGreaterThanAvailableCount_ReturnAvailableBytes)
{
    DataBuffer buffer (4, 2);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    buffer.add_data (values);

    double retrieved[8];
    auto result = buffer.get_data (2, retrieved);

    EXPECT_EQ (result, 1);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], values[i]);
    }
}

TEST (DataBufferTest, GetData_CalledMultipleTimes_ReturnEachValueSetOnceStartingWithOldest)
{
    DataBuffer buffer (4, 2);
    double first_values[4] = {1.0, 2.0, 3.0, 4.0};
    double second_values[4] = {5.0, 6.0, 7.0, 8.0};

    buffer.add_data (first_values);
    buffer.add_data (second_values);

    double retrieved[4];
    buffer.get_data (1, retrieved);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], first_values[i]);
    }

    buffer.get_data (1, retrieved);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], second_values[i]);
    }
}

TEST (DataBufferTest, GetData_AllValueSetsAlreadyReturned_DataCountIsZero)
{
    DataBuffer buffer (4, 1);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    buffer.add_data (values);

    double retrieved[4];
    buffer.get_data (1, retrieved);

    EXPECT_EQ (buffer.get_data_count (), 0);
}

TEST (DataBufferTest, GetData_InvokedInMultipleThreads_DataReturnedWithoutMixing)
{
    DataBuffer buffer (4, 1024);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    for (int i = 0; i < 1024; i++)
    {
        buffer.add_data (values);
    }

    std::thread threads[1024];
    double retrieved[1024][4];

    for (int i = 0; i < 1024; i++)
    {
        threads[i] =
            std::thread ([&] (double *retrieval_buffer) { buffer.get_data (1, retrieval_buffer); },
                retrieved[i]);
    }

    for (int i = 0; i < 1024; i++)
    {
        threads[i].join ();
    }

    for (int i = 0; i < 1024; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            EXPECT_DOUBLE_EQ (retrieved[i][j], values[j]);
        }
    }
}

TEST (DataBufferTest, GetData_InvokedInMoreThreadsThanAvailableData_SummedReturnsEqualAvailableData)
{
    DataBuffer buffer (4, 1024);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    for (int i = 0; i < 1024; i++)
    {
        buffer.add_data (values);
    }

    std::thread threads[2048];
    double retrieved[2048][4];
    std::promise<size_t> promises[2048];
    std::future<size_t> counts[2048];

    for (int i = 0; i < 2048; i++)
    {
        counts[i] = promises[i].get_future ();
        threads[i] = std::thread ([&] (double *retrieval_buffer, std::promise<size_t> count)
            { count.set_value (buffer.get_data (1, retrieval_buffer)); },
            retrieved[i], std::move (promises[i]));
    }

    for (int i = 0; i < 2048; i++)
    {
        threads[i].join ();
    }

    size_t total = 0;
    for (int i = 0; i < 2048; i++)
    {
        total += counts[i].get ();
    }

    ASSERT_EQ (total, 1024);
}

TEST (DataBufferTest, GetCurrentData_MaxCountLessThanAvailableCount_ReturnMaxCountBytes)
{
    DataBuffer buffer (4, 2);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    for (int i = 0; i < 2; i++)
    {
        buffer.add_data (values);
    }

    double retrieved[4];
    auto result = buffer.get_current_data (1, retrieved);

    EXPECT_EQ (result, 1);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], values[i]);
    }
}

TEST (DataBufferTest, GetCurrentData_MaxCountEqualToAvailableCount_ReturnAvailableBytes)
{
    DataBuffer buffer (4, 2);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    buffer.add_data (values);

    double retrieved[4];
    auto result = buffer.get_current_data (1, retrieved);

    EXPECT_EQ (result, 1);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], values[i]);
    }
}

TEST (DataBufferTest, GetCurrentData_MaxCountGreaterThanAvailableCount_ReturnAvailableBytes)
{
    DataBuffer buffer (4, 2);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    buffer.add_data (values);

    double retrieved[8];
    auto result = buffer.get_current_data (2, retrieved);

    EXPECT_EQ (result, 1);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], values[i]);
    }
}

TEST (DataBufferTest, GetCurrentData_CalledMultipleTimes_ReturnMostRecentValueSetEachTime)
{
    DataBuffer buffer (4, 2);
    double first_values[4] = {1.0, 2.0, 3.0, 4.0};
    double second_values[4] = {5.0, 6.0, 7.0, 8.0};

    buffer.add_data (first_values);
    buffer.add_data (second_values);

    double retrieved[4];
    buffer.get_current_data (1, retrieved);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], second_values[i]);
    }

    buffer.get_current_data (1, retrieved);
    for (int i = 0; i < 4; i++)
    {
        EXPECT_EQ (retrieved[i], second_values[i]);
    }
}

TEST (DataBufferTest, GetCurrentData_Any_DataCountStaysConstant)
{
    DataBuffer buffer (4, 1);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    buffer.add_data (values);

    size_t first = buffer.get_data_count ();

    double retrieved[4];
    buffer.get_current_data (1, retrieved);

    size_t second = buffer.get_data_count ();

    EXPECT_EQ (first, second);
}

TEST (DataBufferTest, GetCurrentData_InvokedInMultipleThreads_DataReturnedWithoutMixing)
{
    DataBuffer buffer (4, 1024);
    double values[4] = {1.0, 2.0, 3.0, 4.0};

    for (int i = 0; i < 1024; i++)
    {
        buffer.add_data (values);
    }

    std::thread threads[1023];
    std::list<std::array<double, 4096>> retrieved (1023);

    auto it = retrieved.begin ();
    for (int i = 0; i < 1023; i++)
    {
        threads[i] = std::thread ([&] (double *retrieval_buffer, int count)
            { buffer.get_current_data (count, retrieval_buffer); },
            &((*it)[0]), i + 1);
        it++;
    }

    for (int i = 0; i < 1023; i++)
    {
        threads[i].join ();
    }

    it = retrieved.begin ();
    for (int i = 0; i < 1023; i++)
    {
        for (int j = 0; j < i + 1; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                ASSERT_EQ ((*it)[4 * j + k], values[k]);
            }
        }
        it++;
    }
}

TEST (DataBufferTest, IsReady_BufferCanFitInMemory_ReturnTrue)
{
    DataBuffer buffer (4, 16);
    EXPECT_EQ (buffer.is_ready (), true);
}

TEST (DataBufferTest, IsReady_BufferCannotFitInMemory_ReturnFalse)
{
    DataBuffer buffer (INT_MAX, SIZE_MAX);
    EXPECT_EQ (buffer.is_ready (), false);
}

TEST (DataBufferTest, IsReady_BufferSizeIsZero_ReturnFalse)
{
    DataBuffer buffer_zero (4, 0);
    EXPECT_EQ (buffer_zero.is_ready (), false);
}