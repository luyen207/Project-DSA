#!/usr/bin/env bash
# Xóa dữ liệu chạy thật để lần chạy sau nạp lại từ Backend/data/seed.json (về trạng thái demo ban đầu).
# Dùng:  scripts/reset-data.sh        (hỏi xác nhận)      scripts/reset-data.sh -y   (không hỏi)
set -euo pipefail
cd "$(dirname "$0")/.."
target="Backend/data/runtime"
if [[ ! -d "$target" ]]; then echo "Chưa có dữ liệu chạy thật, không cần reset."; exit 0; fi
if [[ "${1:-}" != "-y" ]]; then
  read -r -p "Xóa toàn bộ dữ liệu trong $target (đơn hàng, tài khoản mới đăng ký...)? [y/N] " answer
  [[ "$answer" =~ ^[yY]$ ]] || { echo "Đã hủy."; exit 0; }
fi
rm -rf "$target"
echo "Đã xóa. Lần chạy tới sẽ nạp lại dữ liệu mẫu."
