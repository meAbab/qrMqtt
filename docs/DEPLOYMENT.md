# Deployment

## Build And Install

Install the packages listed in the README, then build and install:

```bash
cmake -S . -B cmake-build -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build
cd cmake-build && ctest --output-on-failure
sudo cmake --install cmake-build
```

## Service User And Configuration

Create a non-login account and protected configuration directory:

```bash
sudo useradd --system --home /var/lib/qrMqtt --shell /usr/sbin/nologin qrmqtt
sudo install -d -m 0750 -o qrmqtt -g qrmqtt /etc/qrMqtt /etc/qrMqtt/certs
sudo install -m 0640 -o qrmqtt -g qrmqtt config/qrMqtt.conf /etc/qrMqtt/qrMqtt.conf
sudo install -m 0600 -o qrmqtt -g qrmqtt /dev/null /etc/qrMqtt/qrMqtt.env
```

Add `QRMQTT_HMAC_SECRET=...` to `/etc/qrMqtt/qrMqtt.env`, configure broker
details and certificate paths in `/etc/qrMqtt/qrMqtt.conf`, then install and
start the service:

```bash
sudo install -m 0644 systemd/qrMqtt.service /etc/systemd/system/qrMqtt.service
sudo systemctl daemon-reload
sudo systemctl enable --now qrMqtt
sudo journalctl -u qrMqtt -f
```

The service unit creates `/var/lib/qrMqtt` automatically for the outgoing
message queue and consumed-token state.

## Broker

Use `config/mosquitto.conf.example` and `config/mosquitto.acl.example` as a
starting point for a TLS listener and least-privilege MQTT identities. Generate
unique credentials or client certificates for each deployed device.
