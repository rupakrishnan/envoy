#pragma once

#include "common/common/logger.h"
#include "common/common/logger_delegates.h"
#include "common/common/thread.h"
#include "common/event/libevent.h"
#include "common/http/http2/codec_impl.h"

#include "test/mocks/access_log/mocks.h"
#include "test/test_common/environment.h"
#include "test/test_listener.h"

#include "gmock/gmock.h"

namespace Envoy {
class TestRunner {
public:
  static int RunTests(int argc, char** argv) {
    ::testing::InitGoogleMock(&argc, argv);
    Event::Libevent::Global::initialize();
    Http::Http2::initializeNghttp2Logging();

    // Add a test-listener so we can call a hook where we can do a quiescence
    // check after each method. See
    // https://github.com/google/googletest/blob/master/googletest/docs/advanced.md
    // for details.
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new TestListener);

    // Use the recommended, but not default, "threadsafe" style for the Death Tests.
    // See: https://github.com/google/googletest/commit/84ec2e0365d791e4ebc7ec249f09078fb5ab6caa
    ::testing::FLAGS_gtest_death_test_style = "threadsafe";

    // Set gtest properties
    // (https://github.com/google/googletest/blob/master/googletest/docs/AdvancedGuide.md#logging-additional-information),
    // they are available in the test XML.
    // TODO(htuch): Log these as well?
    testing::Test::RecordProperty("TemporaryDirectory", TestEnvironment::temporaryDirectory());
    testing::Test::RecordProperty("RunfilesDirectory", TestEnvironment::runfilesDirectory());

    TestEnvironment::setEnvVar("TEST_UDSDIR", TestEnvironment::unixDomainSocketDirectory(), 1);

    TestEnvironment::initializeOptions(argc, argv);
    Thread::MutexBasicLockable lock;
    Logger::Context logging_state(TestEnvironment::getOptions().logLevel(),
                                  TestEnvironment::getOptions().logFormat(), lock);

    // Allocate fake log access manager.
    testing::NiceMock<AccessLog::MockAccessLogManager> access_log_manager;
    std::unique_ptr<Logger::FileSinkDelegate> file_logger;

    // Redirect all logs to fake file when --log-path arg is specified in command line.
    if (!TestEnvironment::getOptions().logPath().empty()) {
      file_logger = std::make_unique<Logger::FileSinkDelegate>(
          TestEnvironment::getOptions().logPath(), access_log_manager, Logger::Registry::getSink());
    }
    return RUN_ALL_TESTS();
  }
};
} // namespace Envoy
