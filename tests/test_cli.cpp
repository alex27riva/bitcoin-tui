#include <catch2/catch_test_macros.hpp>

#include <cstdlib>
#include <string>

#ifdef _WIN32
#  include <windows.h>
#else
#  include <sys/wait.h>
#endif

// Path to the bitcoin-tui binary, injected via CMake ENVIRONMENT.
static std::string binary() {
    const char* p = std::getenv("BITCOIN_TUI_BINARY");
    REQUIRE(p != nullptr);
    return p;
}

static int exit_code(const std::string& cmd) {
    int status = std::system(cmd.c_str());
#ifdef _WIN32
    return status;
#else
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif
}

// ---------------------------------------------------------------------------
// --help / --version
// Regression: these used to return 0 from configure() while the caller only
// skipped run() on non-zero, so the TUI would launch instead of exiting.
// ---------------------------------------------------------------------------

TEST_CASE("--help exits 0 without launching TUI") {
    CHECK(exit_code(binary() + " --help >/dev/null 2>&1") == 0);
}

TEST_CASE("--version exits 0 without launching TUI") {
    CHECK(exit_code(binary() + " --version >/dev/null 2>&1") == 0);
}

TEST_CASE("-v exits 0 without launching TUI") {
    CHECK(exit_code(binary() + " -v >/dev/null 2>&1") == 0);
}

// ---------------------------------------------------------------------------
// Unknown / invalid options
// ---------------------------------------------------------------------------

TEST_CASE("unknown option exits non-zero") {
    CHECK(exit_code(binary() + " --does-not-exist >/dev/null 2>&1") != 0);
}

// ---------------------------------------------------------------------------
// Network flags
// ---------------------------------------------------------------------------

TEST_CASE("--testnet and --regtest together exit non-zero") {
    CHECK(exit_code(binary() + " --testnet --regtest >/dev/null 2>&1") != 0);
}

TEST_CASE("--testnet and --signet together exit non-zero") {
    CHECK(exit_code(binary() + " --testnet --signet >/dev/null 2>&1") != 0);
}

TEST_CASE("--regtest and --signet together exit non-zero") {
    CHECK(exit_code(binary() + " --regtest --signet >/dev/null 2>&1") != 0);
}

// ---------------------------------------------------------------------------
// Lua flags — parse-time only (no running node required)
// --version short-circuits before any connection attempt, so combining it
// with Lua flags verifies the flags are accepted by the parser.
// ---------------------------------------------------------------------------

TEST_CASE("--tab with --version exits 0") {
    CHECK(exit_code(binary() + " --tab /nonexistent.lua --version >/dev/null 2>&1") == 0);
}

TEST_CASE("--allow-rpc with --version exits 0") {
    CHECK(exit_code(binary() + " --allow-rpc getblockchaininfo --version >/dev/null 2>&1") == 0);
}
