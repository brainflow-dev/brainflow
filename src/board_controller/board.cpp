#include "board.h"
#include "board_controller.h"

std::shared_ptr<spdlog::logger> Board::board_logger = spdlog::stderr_logger_mt ("board_logger");

int Board::set_log_level (int level)
{
    int log_level = level;
    if (level > 6)
        log_level = 6;
    if (level < 0)
        log_level = 0;
    Board::board_logger->set_level (spdlog::level::level_enum (log_level));
    Board::board_logger->flush_on (spdlog::level::level_enum (log_level));
    return STATUS_OK;
}
