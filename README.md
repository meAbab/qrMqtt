# qrMqtt v2

![qrMqtt project cover](assets/cover-qrMQTT.JPG)

`qrMqtt` is a door-side QR event publisher for parcel and access workflows. It
captures a QR code, verifies an expiring signed token, and publishes a
structured MQTT event without exposing the raw QR payload.

Version 2 is a production-oriented rewrite. The original demonstration
application is preserved on `legacy/v1` and tagged `v1.0.0-legacy`.

## Features

- Persistent camera scanner with duplicate-scan suppression.
- Signed, expiring, single-use QR tokens verified with HMAC-SHA256.
- Persistent replay protection through a consumed-nonce state file.
- MQTT v5 JSON events with message expiry.
- TLS, optional mutual TLS, and username/password broker authentication.
- Retained availability and camera status topics.
- Disk-backed outgoing queue while the broker is unavailable.
- External configuration, systemd service, broker ACL examples, unit tests,
  and GitHub Actions CI.

This service intentionally does not operate a door or box lock. A physical
actuator must be a separate authorization service that consumes only verified
events and applies its own safety rules.

## Topics

For a device configured with `mqtt.topic_prefix = qrMqtt/devices/front-door`:

```text
qrMqtt/devices/front-door/availability  Retained online/offline presence
qrMqtt/devices/front-door/status        Retained camera/last-decision status
qrMqtt/devices/front-door/events/scan   Approved or rejected scan events
```

Example event:

```json
{
  "device_id": "front-door",
  "event": "qr_scanned",
  "timestamp": "2026-05-25T10:30:00Z",
  "code_id": "PKG-48291",
  "result": "approved",
  "reason": "verified"
}
```

## Requirements

On Debian, Ubuntu, or Raspberry Pi OS:

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config libopencv-dev libmosquitto-dev libzbar-dev libssl-dev mosquitto mosquitto-clients
```

`mosquitto` is only needed on the device when it hosts the broker locally.

## Build And Test

The Make build remains convenient for development:

```bash
make clean
make CXXFLAGS='-Wall -Wextra -Werror -g -std=c++11'
make test CXXFLAGS='-Wall -Wextra -Werror -g -std=c++11'
```

CMake is available for installation and CI:

```bash
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build
cd cmake-build && ctest --output-on-failure
```

## Configuration

Start with [config/qrMqtt.conf](config/qrMqtt.conf). Set a real broker host,
certificate authority file, credentials, and topic prefix. The service reads:

```bash
./qrMqtt --config config/qrMqtt.conf
```

Signed tokens are enabled by default. Keep the signing secret out of the
configuration file by providing it through the environment:

```bash
export QRMQTT_HMAC_SECRET='replace-with-a-long-random-secret'
```

For systemd, store it in `/etc/qrMqtt/qrMqtt.env` with restrictive file
permissions:

```bash
QRMQTT_HMAC_SECRET=replace-with-a-long-random-secret
```

## Generate A QR Token

The accepted token format is:

```text
qrmqtt:v1:DELIVERY_ID:EXPIRY_UNIX:NONCE:HMAC_SHA256
```

Generate a token payload, then encode its output as a QR image:

```bash
QRMQTT_HMAC_SECRET='replace-with-a-long-random-secret' \
  tools/create_token.sh PKG-48291 1780000000 nonce-001
```

Tokens expire at their Unix timestamp and a successfully verified nonce cannot
be used again, including after the service restarts.

## Run

After configuring broker TLS and connecting a camera:

```bash
QRMQTT_HMAC_SECRET='replace-with-a-long-random-secret' \
  ./qrMqtt --config config/qrMqtt.conf
```

Watch scan events from an authorized MQTT client:

```bash
mosquitto_sub --cafile /path/to/ca.crt -h mqtt.example.net -p 8883 \
  -u automation -P 'password' -t 'qrMqtt/devices/+/events/scan' -v
```

For older Raspberry Pi Video4Linux camera setups:

```bash
sudo modprobe bcm2835-v4l2
```

New Raspberry Pi deployments should use the current `libcamera`/`rpicam`
camera stack when selecting camera hardware and integration.

## Deploy

- Application configuration: [config/qrMqtt.conf](config/qrMqtt.conf)
- Broker TLS example: [config/mosquitto.conf.example](config/mosquitto.conf.example)
- Broker ACL example: [config/mosquitto.acl.example](config/mosquitto.acl.example)
- Service unit: [systemd/qrMqtt.service](systemd/qrMqtt.service)
- Security guidance: [docs/SECURITY.md](docs/SECURITY.md)
- Installation guidance: [docs/DEPLOYMENT.md](docs/DEPLOYMENT.md)

## Project Layout

```text
assets/          README images
config/          Application and broker examples
docs/            Deployment and security guidance
include/qrMqtt/  Public C++ headers
src/             Application implementation
systemd/         Production service unit
tests/           Unit tests for security and event behavior
tools/           Operator utilities
```

## License

This project is distributed under the GNU General Public License, version 2.
See [LICENSE](LICENSE).
