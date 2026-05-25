#!/bin/sh
set -eu

if [ "$#" -ne 3 ] || [ -z "${QRMQTT_HMAC_SECRET:-}" ]; then
    echo "Usage: QRMQTT_HMAC_SECRET=secret $0 DELIVERY_ID EXPIRY_UNIX NONCE" >&2
    exit 1
fi

unsigned="qrmqtt:v1:$1:$2:$3"
signature="$(printf '%s' "$unsigned" | openssl dgst -sha256 -hmac "$QRMQTT_HMAC_SECRET" | sed 's/^.*= //')"
printf '%s:%s\n' "$unsigned" "$signature"
