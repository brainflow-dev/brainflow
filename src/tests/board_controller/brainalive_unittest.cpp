#include <array>
#include <chrono>
#include <future>
#include <vector>

#include <gtest/gtest.h>

#include "brainalive.h"


class BrainAliveTestBoard : public BrainAlive
{
public:
    BrainAliveTestBoard () : BrainAlive (BrainFlowInputParams ())
    {
        params.timeout = 1;
    }

    ~BrainAliveTestBoard () override
    {
        // Tests do not connect a BLE peripheral.
        initialized = false;
    }

    using Board::board_descr;
    using Board::prepare_for_acquisition;

    void mark_prepared ()
    {
        initialized = true;
    }
};

class BrainAliveTest : public testing::Test
{
protected:
    BrainAliveTestBoard board;
    std::array<uint8_t, BrainAlive::brainalive_packet_size> packet {};

    void SetUp () override
    {
        ASSERT_EQ (board.prepare_for_acquisition (100, nullptr), 0);
        for (int sample = 0; sample < BrainAlive::num_of_packets; sample++)
        {
            uint8_t *data = packet.data () + sample * BrainAlive::brainalive_single_packet_size;
            // First EEG channel: -1; second EEG channel: +1; first accelerometer axis: -1.
            data[4] = data[5] = data[6] = 0xFF;
            data[9] = 1;
            data[28] = data[29] = 0xFF;
        }
    }

    void receive_packet ()
    {
        board.read_data ({}, {}, packet.data (), packet.size (), 0);
    }

    int sample_count ()
    {
        int count = -1;
        EXPECT_EQ (board.get_board_data_count (0, &count), 0);
        return count;
    }
};

TEST_F (BrainAliveTest, MissingHandshakeDiscardsSamples)
{
    receive_packet ();
    EXPECT_EQ (sample_count (), 0);
}

TEST_F (BrainAliveTest, InvalidHandshakeDiscardsSamples)
{
    const int configs[][3] = {{0, 1, 2400}, {24, 0, 2400}, {24, 1, 0}};
    for (const auto &config : configs)
    {
        board.set_eeg_config (config[0], config[1], config[2]);
        receive_packet ();
        EXPECT_EQ (sample_count (), 0);
    }
}

TEST_F (BrainAliveTest, ValidHandshakePublishesSignedSamplesWithoutGainOverflow)
{
    receive_packet ();
    ASSERT_EQ (sample_count (), 0);
    board.set_eeg_config (24, 24, 2400);
    receive_packet ();
    ASSERT_EQ (sample_count (), 5);

    json descr = board.board_descr["default"];
    std::vector<double> data (descr["num_rows"].get<int> () * 5);
    ASSERT_EQ (board.get_board_data (5, 0, data.data ()), 0);
    int eeg_negative = descr["eeg_channels"][0];
    int eeg_positive = descr["eeg_channels"][1];
    int accel_negative = descr["accel_channels"][0];
    double expected_scale = 2400000.0 / (576.0 * 8388607.0);
    for (int sample = 0; sample < 5; sample++)
    {
        EXPECT_DOUBLE_EQ (data[eeg_negative * 5 + sample], -expected_scale);
        EXPECT_DOUBLE_EQ (data[eeg_positive * 5 + sample], expected_scale);
        EXPECT_DOUBLE_EQ (data[accel_negative * 5 + sample], -1.0);
    }
}

TEST_F (BrainAliveTest, InvalidHandshakeClearsPreviousCalibration)
{
    board.set_eeg_config (24, 1, 2400);
    board.set_eeg_config (24, 1, 0);
    receive_packet ();
    EXPECT_EQ (sample_count (), 0);
}

TEST_F (BrainAliveTest, ReleaseSessionClearsCalibration)
{
    board.set_eeg_config (24, 1, 2400);
    ASSERT_EQ (board.release_session (), 0);
    ASSERT_EQ (board.prepare_for_acquisition (100, nullptr), 0);
    receive_packet ();
    EXPECT_EQ (sample_count (), 0);
}

TEST_F (BrainAliveTest, StartStreamTimesOutWithoutHandshake)
{
    board.mark_prepared ();
    EXPECT_EQ (board.start_stream (100, nullptr), (int)BrainFlowExitCodes::BOARD_NOT_READY_ERROR);
    EXPECT_EQ (sample_count (), 0);
}

TEST_F (BrainAliveTest, StartStreamWaitsForHandshake)
{
    board.mark_prepared ();
    // An invalid buffer size stops acquisition after the handshake, before any BLE write.
    auto started =
        std::async (std::launch::async, [this] { return board.start_stream (0, nullptr); });
    EXPECT_EQ (started.wait_for (std::chrono::milliseconds (20)), std::future_status::timeout);
    board.set_eeg_config (24, 1, 2400);
    EXPECT_EQ (started.get (), (int)BrainFlowExitCodes::INVALID_BUFFER_SIZE_ERROR);
}
