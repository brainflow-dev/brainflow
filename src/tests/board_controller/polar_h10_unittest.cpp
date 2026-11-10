#include "polar_h10.h"
#include "polar_protocol.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <gtest/gtest.h>
#include <thread>

namespace
{
    const std::vector<uint8_t> ecg_packet = {
        0, 0, 0x94, 0x35, 0x77, 0, 0, 0, 0, 0, 0x02, 0x80, 0xff, 0x02, 0x80, 0};

    class FakePolar : public PolarH10
    {
        std::atomic<bool> running;
        std::thread producer;

    protected:
        int connect_device () override
        {
            return 0;
        }
        void disconnect_device () override
        {
            running = false;
            if (producer.joinable ())
                producer.join ();
        }
        bool write_control (const std::vector<uint8_t> &command) override
        {
            if (drop_response)
                return true;
            if (command[0] == 1)
            {
                const uint8_t first[] = {0xf0, 1, 0, 0, 1, 0, 1, 130, 0};
                const uint8_t last[] = {0xf0, 1, 0, 0, 0, 1, 1, 14, 0};
                on_control (first, sizeof (first));
                on_control (last, sizeof (last));
            }
            else
            {
                uint8_t unrelated[] = {0xf0, command[0], 2, 0};
                on_control (
                    unrelated, sizeof (unrelated)); // Wrong measurement type must be ignored.
                uint8_t response[] = {0xf0, command[0], 0, uint8_t (reject ? 8 : 0)};
                on_control (response, sizeof (response));
                if (command[0] == 3)
                    disconnect_device ();
                else if (command[0] == 2 && !reject && produce)
                {
                    const std::vector<uint8_t> expected = {2, 0, 0, 1, 130, 0, 1, 1, 14, 0};
                    EXPECT_EQ (command, expected);
                    running = true;
                    producer = std::thread (
                        [this]
                        {
                            uint64_t timestamp = 2000000000;
                            while (running)
                            {
                                std::this_thread::sleep_for (std::chrono::milliseconds (10));
                                auto packet = ecg_packet;
                                for (size_t i = 0; i < 8; ++i)
                                    packet[i + 1] = uint8_t (timestamp >> (8 * i));
                                const uint8_t hr[] = {0x16, 60, 0, 4, 0, 2};
                                on_heart_rate (hr, sizeof (hr));
                                on_ecg (packet.data (), packet.size ());
                                timestamp += 1000000000;
                            }
                        });
                }
            }
            return true;
        }

    public:
        bool reject = false;
        bool produce = true;
        bool drop_response = false;
        explicit FakePolar (BrainFlowInputParams params) : PolarH10 (params), running (false)
        {
        }
        ~FakePolar () override
        {
            release_session ();
            disconnect_device ();
        }
        void late_sample ()
        {
            on_ecg (ecg_packet.data (), ecg_packet.size ());
        }
    };
}

TEST (PolarH10, OfficialEcgFixtureAndMalformedFrames)
{
    polar::Ecg result;
    ASSERT_TRUE (polar::ecg (ecg_packet.data (), ecg_packet.size (), result));
    ASSERT_EQ (result.samples.size (), 2u);
    EXPECT_EQ (result.timestamp, 2000000000u);
    EXPECT_EQ (result.samples[0], -32766);
    EXPECT_EQ (result.samples[1], 32770);
    for (size_t size = 0; size < 13; ++size)
        EXPECT_FALSE (polar::ecg (ecg_packet.data (), size, result));
    EXPECT_FALSE (polar::ecg (ecg_packet.data (), 14, result));
    auto unsupported = ecg_packet;
    unsupported[9] = 0x80;
    EXPECT_FALSE (polar::ecg (unsupported.data (), unsupported.size (), result));
}

TEST (PolarH10, ParsesSettingsAndRejectsTruncation)
{
    EXPECT_TRUE (polar::supports_ecg_profile ({0, 1, 130, 0, 1, 1, 14, 0}));
    EXPECT_FALSE (polar::supports_ecg_profile ({0, 1, 100, 0, 1, 1, 14, 0}));
    EXPECT_FALSE (polar::supports_ecg_profile ({0, 1, 130, 0, 1, 1, 14}));
    EXPECT_FALSE (polar::supports_ecg_profile ({0, 1, 130, 0, 1, 1, 14, 0, 0xff, 1}));
}

TEST (PolarH10, PreservesEveryRrIntervalAndOptionalHrFields)
{
    polar::HeartRate result;
    const uint8_t data[] = {0x1f, 0x2c, 1, 0, 0, 0, 4, 0, 2};
    ASSERT_TRUE (polar::heart_rate (data, sizeof (data), result));
    EXPECT_EQ (result.bpm, 300);
    EXPECT_EQ (result.contact, 1);
    EXPECT_EQ (result.rr_ms, (std::vector<double> {1000, 500}));
    EXPECT_FALSE (polar::heart_rate (data, sizeof (data) - 1, result));
    const uint8_t no_rr[] = {0, 60};
    ASSERT_TRUE (polar::heart_rate (no_rr, sizeof (no_rr), result));
    EXPECT_TRUE (result.rr_ms.empty ());
    EXPECT_TRUE (std::isnan (result.contact));
}

TEST (PolarH10, EmulatedHandshakeAcquisitionAndRestart)
{
    BrainFlowInputParams params;
    params.timeout = 1;
    FakePolar board (params);
    ASSERT_EQ (board.prepare_session (), 0);
    for (int iteration = 0; iteration < 2; ++iteration)
    {
        ASSERT_EQ (board.start_stream (128, ""), 0);
        EXPECT_EQ (board.start_stream (128, ""), 8);
        ASSERT_EQ (board.stop_stream (), 0);
        int count = 0;
        ASSERT_EQ (board.get_board_data_count (0, &count), 0);
        ASSERT_GE (count, 2);
        std::vector<double> data (6 * count);
        ASSERT_EQ (board.get_board_data (count, 0, data.data ()), 0);
        EXPECT_EQ (data[count], -32766);
        EXPECT_EQ (data[count + 1], 32770);
        EXPECT_TRUE (std::isnan (data[3 * count]));
        EXPECT_NEAR (data[4 * count + 1] - data[4 * count], 1.0 / 130, 1e-6);
        ASSERT_EQ (board.get_board_data_count (1, &count), 0);
        ASSERT_GE (count, 2);
        data.resize (6 * count);
        ASSERT_EQ (board.get_board_data (count, 1, data.data ()), 0);
        EXPECT_EQ (data[count], 60);
        EXPECT_EQ (data[2 * count], 1000);
        EXPECT_EQ (data[2 * count + 1], 500);
        EXPECT_NEAR (data[4 * count + 1] - data[4 * count], 0.5, 1e-6);
        board.late_sample ();
        ASSERT_EQ (board.get_board_data_count (0, &count), 0);
        EXPECT_EQ (count, 0);
    }
    EXPECT_EQ (board.release_session (), 0);
    EXPECT_EQ (board.prepare_session (), 0);
    EXPECT_EQ (board.start_stream (128, ""), 0);
    EXPECT_EQ (board.release_session (), 0);
}

TEST (PolarH10, DataTimeoutAndControlRejectionRecoverCleanly)
{
    BrainFlowInputParams params;
    params.timeout = 1;
    FakePolar board (params);
    ASSERT_EQ (board.prepare_session (), 0);
    board.produce = false;
    EXPECT_EQ (board.start_stream (128, ""), 18);
    board.produce = true;
    ASSERT_EQ (board.start_stream (128, ""), 0);
    ASSERT_EQ (board.stop_stream (), 0);
    board.reject = true;
    EXPECT_EQ (board.start_stream (128, ""), 4);
    EXPECT_EQ (board.start_stream (128, ""), 7);
    board.reject = false;
    ASSERT_EQ (board.prepare_session (), 0);
    ASSERT_EQ (board.start_stream (128, ""), 0);
    ASSERT_EQ (board.stop_stream (), 0);
    ASSERT_EQ (board.release_session (), 0);
    board.drop_response = true;
    EXPECT_EQ (board.prepare_session (), 18);
    board.drop_response = false;
    EXPECT_EQ (board.prepare_session (), 0);
}
