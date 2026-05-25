# Security Model

`qrMqtt` treats every QR image as untrusted input. It publishes verification
results; it does not directly unlock a door or parcel box.

## Token Format

The v2 token format is:

```text
qrmqtt:v1:DELIVERY_ID:EXPIRY_UNIX:NONCE:HMAC_SHA256
```

The HMAC input is the first five colon-delimited fields. Use a long,
randomly-generated signing secret and provision it outside source control
through `QRMQTT_HMAC_SECRET`.

The application:

- Rejects malformed, expired, or incorrectly signed tokens.
- Restricts delivery IDs and nonces to compact identifier characters.
- Records a verified nonce in `security.nonce_file`.
- Rejects any previously accepted nonce after scans or restarts.
- Publishes the delivery identifier and decision, never the raw QR token or
  signature.

## MQTT

Production configuration must enable TLS and verify the broker hostname. Use a
unique broker identity per physical device and ACLs limiting the device to its
own topics. The included broker examples allow `front-door` to publish only
availability, status, and scan events under its own prefix.

MQTT scan events have an expiry interval so stale events are not useful as
delayed commands. Availability is retained to show service state.

## Physical Actions

Do not attach a lock-opening relay directly to QR scan output. A future
actuator service should independently authenticate to MQTT, consume approved
events, confirm policy and sensor state, enforce timeouts, and keep an audit
log. For a door rather than a parcel box, require an additional authorization
factor.

## Secret And State Files

- Store `/etc/qrMqtt/qrMqtt.env` with mode `0600`, owned by the service user.
- Protect `/etc/qrMqtt/certs/` private keys similarly.
- Preserve `/var/lib/qrMqtt/consumed-nonces.tsv`; deleting it removes replay
  history for otherwise unexpired tokens.
- The MQTT outbox may contain event identifiers and decisions; protect it as
  operational data.
