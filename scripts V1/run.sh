#!/usr/bin/env bash
# Chạy ShopPro (tự biên dịch nếu chưa có). Mọi tham số được chuyển cho shoppro_api, ví dụ:
#   scripts/run.sh --port 9000
#   scripts/run.sh --host 0.0.0.0        # cho máy khác trong mạng LAN truy cập
set -euo pipefail
cd "$(dirname "$0")/.."
[[ -x build/shoppro_api ]] || scripts/build.sh
exec build/shoppro_api "$@"
