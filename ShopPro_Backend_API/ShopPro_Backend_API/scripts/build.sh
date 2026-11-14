#!/usr/bin/env bash
# Biên dịch ShopPro (Linux/macOS). Dùng:  scripts/build.sh          -> chỉ build
#                                        scripts/build.sh --test   -> build + chạy kiểm thử
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -S Backend -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
if [[ "${1:-}" == "--test" ]]; then
  ctest --test-dir build --output-on-failure
fi
echo
echo "Xong. Chạy máy chủ bằng:  scripts/run.sh"
