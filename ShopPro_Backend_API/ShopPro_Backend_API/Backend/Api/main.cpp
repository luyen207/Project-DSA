// ShopPro REST API + may chu web. Chay: shoppro_api [--port 8080] [--host 127.0.0.1] ...
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include "ApiContext.h"
#include "http/HttpServer.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace fs = std::filesystem;

namespace {

std::atomic<bool> gStop{false};

extern "C" void onSignal(int) { gStop.store(true); }

fs::path executableDirectory() {
#ifdef _WIN32
    wchar_t buffer[MAX_PATH * 4] = {};
    const DWORD length = GetModuleFileNameW(nullptr, buffer, static_cast<DWORD>(sizeof buffer / sizeof buffer[0]));
    return length ? fs::path(buffer).parent_path() : fs::current_path();
#else
    std::error_code error;
    const fs::path exe = fs::read_symlink("/proc/self/exe", error);
    return error ? fs::current_path() : exe.parent_path();
#endif
}

// Tim thu muc goc du an (co Backend/data/seed.json) di len tu thu muc chay va thu muc cua file .exe
fs::path findProjectRoot() {
    for (const fs::path& start : {fs::current_path(), executableDirectory()}) {
        fs::path dir = start;
        for (int depth = 0; depth < 7; ++depth) {
            if (fs::exists(dir / "Backend" / "data" / "seed.json")) return dir;
            if (fs::exists(dir / "data" / "seed.json") && fs::exists(dir / ".." / "Frontend")) return fs::weakly_canonical(dir / "..");
            if (!dir.has_parent_path() || dir.parent_path() == dir) break;
            dir = dir.parent_path();
        }
    }
    return fs::current_path();
}

std::string environment(const char* name) {
    const char* value = std::getenv(name);
    return value ? value : "";
}

void printHelp() {
    std::cout <<
        "ShopPro API - may chu web + REST API cho cua hang truc tuyen\n\n"
        "Cach dung: shoppro_api [tuy chon]\n"
        "  --port N           Cong lang nghe (mac dinh 8080, hoac bien moi truong PORT)\n"
        "  --host ADDR        Dia chi lang nghe (mac dinh 127.0.0.1; dung 0.0.0.0 de mo ra mang)\n"
        "  --data FILE        Tep du lieu chay that (mac dinh Backend/data/runtime/shoppro.json)\n"
        "  --seed FILE        Du lieu mau (mac dinh Backend/data/seed.json)\n"
        "  --web DIR          Thu muc Frontend (mac dinh Frontend/)\n"
        "  --trust-proxy      Tin header X-Forwarded-For (chi bat khi chay sau reverse proxy)\n"
        "  --quiet            Khong in nhat ky yeu cau\n"
        "  --create-admin EMAIL TEN   Tao tai khoan Admin (mat khau lay tu bien moi truong\n"
        "                             SHOPPRO_ADMIN_PASSWORD, toi thieu 8 ky tu) roi thoat\n"
        "  --help             Hien thi tro giup\n\n"
        "Bien moi truong tuong ung: PORT, HOST, SHOPPRO_DATA, SHOPPRO_SEED, SHOPPRO_WEB, TRUST_PROXY=1\n";
}

}  // namespace

int main(int argc, char** argv) {
#ifndef _WIN32
    std::signal(SIGPIPE, SIG_IGN);
#endif
    std::signal(SIGINT, onSignal);
    std::signal(SIGTERM, onSignal);

    const fs::path root = findProjectRoot();
    shoppro::ServerConfig config;
    fs::path seedFile = root / "Backend" / "data" / "seed.json";
    fs::path dataFile = root / "Backend" / "data" / "runtime" / "shoppro.json";
    fs::path webRoot = root / "Frontend";
    std::string adminEmail, adminName;
    bool createAdmin = false;

    if (const auto v = environment("PORT"); !v.empty()) config.port = static_cast<unsigned short>(std::atoi(v.c_str()));
    if (const auto v = environment("HOST"); !v.empty()) config.host = v;
    if (const auto v = environment("SHOPPRO_DATA"); !v.empty()) dataFile = v;
    if (const auto v = environment("SHOPPRO_SEED"); !v.empty()) seedFile = v;
    if (const auto v = environment("SHOPPRO_WEB"); !v.empty()) webRoot = v;
    if (environment("TRUST_PROXY") == "1") config.trustProxy = true;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const auto next = [&](const char* what) -> std::string {
            if (i + 1 >= argc) {
                std::cerr << "Thieu gia tri cho " << what << "\n";
                std::exit(2);
            }
            return argv[++i];
        };
        if (arg == "--help" || arg == "-h") { printHelp(); return 0; }
        else if (arg == "--port") {
            const int port = std::atoi(next("--port").c_str());
            if (port < 1 || port > 65535) { std::cerr << "Cong khong hop le.\n"; return 2; }
            config.port = static_cast<unsigned short>(port);
        }
        else if (arg == "--host") config.host = next("--host");
        else if (arg == "--data") dataFile = next("--data");
        else if (arg == "--seed") seedFile = next("--seed");
        else if (arg == "--web") webRoot = next("--web");
        else if (arg == "--trust-proxy") config.trustProxy = true;
        else if (arg == "--quiet") config.logRequests = false;
        else if (arg == "--create-admin") { createAdmin = true; adminEmail = next("--create-admin"); adminName = next("--create-admin"); }
        else { std::cerr << "Tuy chon khong biet: " << arg << "\n"; printHelp(); return 2; }
    }
    if (config.port == 0) { std::cerr << "Cong khong hop le (PORT).\n"; return 2; }
    config.webRoot = fs::exists(webRoot) ? webRoot : fs::path();

    try {
        shoppro::JsonStore store(dataFile, seedFile);
        const std::int64_t shift = store.seedShiftSeconds();
        shoppro::Json data = store.load();
        shoppro::ActivityLog activity(dataFile.parent_path() / "activity-log.jsonl",
                                      seedFile.parent_path() / "seed-activity.jsonl", shift);

        if (createAdmin) {   // che do dong lenh: tao Admin roi thoat
            using namespace shoppro;
            const std::string password = environment("SHOPPRO_ADMIN_PASSWORD");
            const std::string email = text::lower(text::trim(adminEmail));
            if (!text::validEmail(email) || text::trim(adminName).size() < 2) { std::cerr << "Email hoac ten khong hop le.\n"; return 2; }
            if (password.size() < 8 || password.size() > 128) { std::cerr << "Dat SHOPPRO_ADMIN_PASSWORD (8-128 ky tu) truoc khi chay.\n"; return 2; }
            for (const auto& user : data.at("users")) {
                if (text::lower(user.value("email", "")) == email) { std::cerr << "Email nay da ton tai.\n"; return 2; }
            }
            const std::string salt = crypto::randomHex(16);
            data["users"].push_back({{"id", "adm-" + crypto::randomHex(6)}, {"name", text::trim(adminName)}, {"email", email},
                                     {"phone", ""}, {"address", ""}, {"role", "admin"}, {"status", "active"},
                                     {"joinedAt", timeutil::todayIso()}, {"passwordSalt", salt},
                                     {"passwordHash", crypto::hashPassword(password, salt, kPasswordIterations)},
                                     {"passwordIterations", kPasswordIterations}});
            store.persist(data);
            std::cout << "Da tao Admin " << email << "\n";
            return 0;
        }

        shoppro::Ctx ctx(store, activity, std::move(data));
        const shoppro::Router router = shoppro::buildRouter();
        const shoppro::ApiHandler api = [&](const shoppro::HttpRequest& request) { return router.dispatch(ctx, request); };

        std::cout << "============================================================\n"
                  << "  ShopPro dang chay\n"
                  << "  Trang chon giao dien : http://" << config.host << ":" << config.port << "/\n"
                  << "  Cua hang (khach)     : http://" << config.host << ":" << config.port << "/shop/\n"
                  << "  Quan tri (Admin)     : http://" << config.host << ":" << config.port << "/admin/\n"
                  << "  Du lieu              : " << dataFile.string() << "\n"
                  << "  Frontend             : " << (config.webRoot.empty() ? std::string("(khong tim thay)") : config.webRoot.string()) << "\n"
                  << "  Nhan Ctrl+C de dung.\n"
                  << "============================================================\n" << std::flush;
        const int code = shoppro::runHttpServer(config, api, gStop);
        std::cout << "Da dung may chu.\n";
        return code;
    } catch (const std::exception& error) {
        std::cerr << "Loi khoi dong: " << error.what() << "\n";
        return 1;
    }
}
