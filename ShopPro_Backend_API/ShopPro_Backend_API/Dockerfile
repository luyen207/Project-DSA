# ShopPro – đóng gói để chạy trên máy chủ/đám mây (Render, Railway, Fly.io, VPS…).
# Xây dựng:  docker build -t shoppro .
# Chạy thử:  docker run --rm -p 8080:8080 -v shoppro-data:/data shoppro
#
# Một tiến trình duy nhất phục vụ cả cửa hàng (/shop/), quản trị (/admin/) và REST API (/api/).

# ---------- Giai đoạn 1: biên dịch ----------
FROM ubuntu:24.04 AS build
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential cmake libssl-dev \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY Backend ./Backend
RUN cmake -S Backend -B build -DCMAKE_BUILD_TYPE=Release -DSHOPPRO_BUILD_TESTS=OFF \
 && cmake --build build --target shoppro_api --parallel

# ---------- Giai đoạn 2: chạy ----------
FROM ubuntu:24.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends libssl3t64 libstdc++6 ca-certificates \
 && rm -rf /var/lib/apt/lists/* \
 && useradd --system --no-create-home --shell /usr/sbin/nologin shoppro \
 && mkdir -p /app/Backend/data /app/Frontend /data \
 && chown shoppro:shoppro /data
WORKDIR /app
COPY --from=build /src/build/shoppro_api /app/shoppro_api
COPY Backend/data/seed.json Backend/data/seed-activity.jsonl /app/Backend/data/
COPY Frontend /app/Frontend

# HOST=0.0.0.0 để nhận kết nối từ bên ngoài container. Nền tảng đám mây thường tự đặt PORT.
# Đặt TRUST_PROXY=1 khi chạy sau reverse proxy/HTTPS của nền tảng (để ghi đúng IP khách).
ENV HOST=0.0.0.0 \
    PORT=8080 \
    SHOPPRO_DATA=/data/shoppro.json \
    SHOPPRO_SEED=/app/Backend/data/seed.json \
    SHOPPRO_WEB=/app/Frontend
VOLUME ["/data"]
EXPOSE 8080
USER shoppro
CMD ["/app/shoppro_api", "--quiet"]
