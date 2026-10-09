#!/usr/bin/env bash
#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
# Usage: apt_install.sh <package>...
#
# Hosted runners occasionally stall on a package mirror without apt ever giving up, which
# holds the job until GitHub cancels it hours later. Each attempt is time-limited and retried.
set -euo pipefail

readonly attempts=3
readonly apt_options=(
  -o Acquire::Retries=3
  -o Acquire::http::Timeout=30
  -o Acquire::https::Timeout=30
  -o APT::Update::Error-Mode=any
  -o DPkg::Lock::Timeout=60
)

for attempt in $(seq 1 "$attempts"); do
  if sudo timeout --kill-after=10 180 apt-get "${apt_options[@]}" update &&
    sudo timeout --kill-after=10 600 apt-get "${apt_options[@]}" install --yes "$@"; then
    exit 0
  fi
  echo "::warning::apt attempt $attempt of $attempts failed"
  # A killed install leaves dpkg half-configured, which would fail every later attempt.
  sudo dpkg --configure -a || true
  sleep 10
done

echo "::error::Installing packages failed after $attempts attempts: $*"
exit 1
