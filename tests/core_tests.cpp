#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <qrMqtt/config.hpp>
#include <qrMqtt/duplicate_filter.hpp>
#include <qrMqtt/event.hpp>
#include <qrMqtt/token_validator.hpp>

namespace
{
void testJson()
{
    qrmqtt::ValidationResult result;
    result.approved = true;
    result.code_id = "PKG-1";
    result.nonce = "nonce";
    result.reason = "verified";
    const std::string event = qrmqtt::scanEventJson("front\"door", result);
    assert(event.find("\"device_id\":\"front\\\"door\"") != std::string::npos);
    assert(event.find("\"result\":\"approved\"") != std::string::npos);
    assert(event.find("nonce") == std::string::npos);
}

void testDuplicateFilter()
{
    qrmqtt::DuplicateFilter filter(10);
    assert(filter.accept("one"));
    assert(!filter.accept("one"));
    assert(filter.accept("two"));
}

void testSignedToken()
{
    qrmqtt::Config::Security security;
    security.require_signed_tokens = true;
    security.hmac_secret = "unit-test-secret";
    security.nonce_file = "/tmp/qrmqtt-core-test-nonces.tsv";
    std::remove(security.nonce_file.c_str());
    qrmqtt::TokenValidator validator(security);

    const std::string token =
        "qrmqtt:v1:PKG-48291:4102444800:nonce-001:"
        "67b3b6764bd43bd90e1a7d7d5820be774e7ff9dd3bdfe426fc7fc0716cbbef26";
    const qrmqtt::ValidationResult valid = validator.validate(token, 1700000000);
    assert(valid.approved);
    assert(valid.code_id == "PKG-48291");
    assert(validator.validate(token, 1700000000).reason == "already_used");

    qrmqtt::TokenValidator restarted_validator(security);
    assert(restarted_validator.validate(token, 1700000000).reason == "already_used");
    qrmqtt::TokenValidator expired_validator(security);
    assert(expired_validator.validate(token, 4102444800).reason == "expired");
    qrmqtt::TokenValidator bad_validator(security);
    assert(bad_validator.validate(
        "qrmqtt:v1:PKG-48291:4102444800:nonce-002:"
        "67b3b6764bd43bd90e1a7d7d5820be774e7ff9dd3bdfe426fc7fc0716cbbef20",
        1700000000).reason
        == "invalid_signature");
    assert(bad_validator.validate(
        "qrmqtt:v1:PKG-48291:4102444800:bad nonce:"
        "67b3b6764bd43bd90e1a7d7d5820be774e7ff9dd3bdfe426fc7fc0716cbbef20",
        1700000000).reason
        == "invalid_format");
    std::remove(security.nonce_file.c_str());
}

void testConfig()
{
    const std::string path = "/tmp/qrmqtt-core-tests.conf";
    std::ofstream output(path);
    output << "device_id = test-door\n"
           << "mqtt.host = localhost\n"
           << "mqtt.port = 1883\n"
           << "mqtt.topic_prefix = test/device\n"
           << "mqtt.client_id = test-client\n"
           << "mqtt.tls_enabled = false\n"
           << "mqtt.qos = 2\n"
           << "mqtt.message_expiry_seconds = 20\n"
           << "camera.duplicate_window_seconds = 3\n"
           << "security.require_signed_tokens = true\n";
    output.close();

    setenv("QRMQTT_HMAC_SECRET", "environment-secret", 1);
    const qrmqtt::Config config = qrmqtt::Config::load(path);
    assert(config.device_id == "test-door");
    assert(config.mqtt.qos == 2);
    assert(config.security.hmac_secret == "environment-secret");
    unsetenv("QRMQTT_HMAC_SECRET");
    std::remove(path.c_str());
}
}

int main()
{
    testJson();
    testDuplicateFilter();
    testSignedToken();
    testConfig();
    std::cout << "qrmqtt core tests passed\n";
    return 0;
}
