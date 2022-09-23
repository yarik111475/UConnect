cmake_minimum_required(VERSION 3.14)

project(spdlog LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_definitions(-D SPDLOG_COMPILED_LIB)

set(SOURCES
    async.cpp
    cfg.cpp
    color_sinks.cpp
    file_sinks.cpp
    fmt.cpp
    spdlog.cpp
    stdout_sinks.cpp
    )

set(HEADERS
    spdlog.h
    spdlog/async.h
    spdlog/async_logger-inl.h
    spdlog/async_logger.h
    spdlog/cfg/argv.h
    spdlog/cfg/env.h
    spdlog/cfg/helpers-inl.h
    spdlog/cfg/helpers.h
    spdlog/common-inl.h
    spdlog/common.h
    spdlog/details/backtracer-inl.h
    spdlog/details/backtracer.h
    spdlog/details/circular_q.h
    spdlog/details/console_globals.h
    spdlog/details/file_helper-inl.h
    spdlog/details/file_helper.h
    spdlog/details/fmt_helper.h
    spdlog/details/log_msg-inl.h
    spdlog/details/log_msg.h
    spdlog/details/log_msg_buffer-inl.h
    spdlog/details/log_msg_buffer.h
    spdlog/details/mpmc_blocking_q.h
    spdlog/details/null_mutex.h
    spdlog/details/os-inl.h
    spdlog/details/os.h
    spdlog/details/periodic_worker-inl.h
    spdlog/details/periodic_worker.h
    spdlog/details/registry-inl.h
    spdlog/details/registry.h
    spdlog/details/synchronous_factory.h
    spdlog/details/tcp_client-windows.h
    spdlog/details/tcp_client.h
    spdlog/details/thread_pool-inl.h
    spdlog/details/thread_pool.h
    spdlog/details/windows_include.h
    spdlog/fmt/bin_to_hex.h
    spdlog/fmt/bundled/args.h
    spdlog/fmt/bundled/chrono.h
    spdlog/fmt/bundled/color.h
    spdlog/fmt/bundled/compile.h
    spdlog/fmt/bundled/core.h
    spdlog/fmt/bundled/format-inl.h
    spdlog/fmt/bundled/format.h
    spdlog/fmt/bundled/locale.h
    spdlog/fmt/bundled/os.h
    spdlog/fmt/bundled/ostream.h
    spdlog/fmt/bundled/printf.h
    spdlog/fmt/bundled/ranges.h
    spdlog/fmt/bundled/xchar.h
    spdlog/fmt/chrono.h
    spdlog/fmt/compile.h
    spdlog/fmt/fmt.h
    spdlog/fmt/ostr.h
    spdlog/fmt/xchar.h
    spdlog/formatter.h
    spdlog/fwd.h
    spdlog/logger-inl.h
    spdlog/logger.h
    spdlog/pattern_formatter-inl.h
    spdlog/pattern_formatter.h
    spdlog/sinks/android_sink.h
    spdlog/sinks/ansicolor_sink-inl.h
    spdlog/sinks/ansicolor_sink.h
    spdlog/sinks/base_sink-inl.h
    spdlog/sinks/base_sink.h
    spdlog/sinks/basic_file_sink-inl.h
    spdlog/sinks/basic_file_sink.h
    spdlog/sinks/daily_file_sink.h
    spdlog/sinks/dist_sink.h
    spdlog/sinks/dup_filter_sink.h
    spdlog/sinks/hourly_file_sink.h
    spdlog/sinks/mongo_sink.h
    spdlog/sinks/msvc_sink.h
    spdlog/sinks/null_sink.h
    spdlog/sinks/ostream_sink.h
    spdlog/sinks/qt_sinks.h
    spdlog/sinks/ringbuffer_sink.h
    spdlog/sinks/rotating_file_sink-inl.h
    spdlog/sinks/rotating_file_sink.h
    spdlog/sinks/sink-inl.h
    spdlog/sinks/sink.h
    spdlog/sinks/stdout_color_sinks-inl.h
    spdlog/sinks/stdout_color_sinks.h
    spdlog/sinks/stdout_sinks-inl.h
    spdlog/sinks/stdout_sinks.h
    spdlog/sinks/syslog_sink.h
    spdlog/sinks/systemd_sink.h
    spdlog/sinks/tcp_sink.h
    spdlog/sinks/uc_syslog_sink.h
    spdlog/sinks/win_eventlog_sink.h
    spdlog/sinks/wincolor_sink-inl.h
    spdlog/sinks/wincolor_sink.h
    spdlog/spdlog-inl.h
    spdlog/spdlog.h
    spdlog/stopwatch.h
    spdlog/tweakme.h
    spdlog/version.h

    )

add_library(spdlog STATIC
    ${HEADERS}
    ${SOURCES}
    )

target_compile_definitions(spdlog PRIVATE SPDLOG_LIBRARY)
