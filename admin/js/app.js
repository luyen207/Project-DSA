// Điểm vào của giao diện Quản trị: đăng nhập, khung điều hướng, định tuyến, thăm dò đơn mới.
import { $, html, icon, mount, createRouter, toast, go, confirmDialog, withLoading, formValues, showFormError } from "/assets/shared.js";
import { api, session, tokenStore, events } from "./core.js";
import { wireChartInteractions } from "./charts.js";
import { dashboardView } from "./views/dashboard.js";
import { productsView } from "./views/products.js";
import { categoriesView } from "./views/categories.js";
import { ordersView } from "./views/orders.js";
import { customersView } from "./views/customers.js";
import { reportsView } from "./views/reports.js";
import { logsView } from "./views/logs.js";
import { settingsView } from "./views/settings.js";

const root = $("#root");
const NAV = [
  ["/", "Tổng quan", "chart"], ["/orders", "Đơn hàng", "receipt"], ["/products", "Sản phẩm", "package"], ["/categories", "Danh mục", "tag"],
  ["/customers", "Khách hàng", "users"], ["/reports", "Báo cáo", "money"], ["/logs", "Nhật ký hoạt động", "log"], ["/settings", "Cài đặt", "settings"],
];
const TITLES = { "/": "Tổng quan", "/orders": "Đơn hàng", "/products": "Sản phẩm", "/categories": "Danh mục", "/customers": "Khách hàng", "/reports": "Báo cáo", "/logs": "Nhật ký hoạt động", "/settings": "Cài đặt" };
const DEMO = { email: "admin@shoppro.vn", password: "Admin@12345" };
let router = null;
let pollTimer = null;

/* ------------------------------- Đăng nhập ------------------------------- */
function showLogin(message) {
  stopPolling();
  document.title = "Đăng nhập quản trị – ShopPro";
  mount(root, html`
    <div class="login-page">
      <section class="login-art" aria-hidden="false">
        <div class="row" style="gap:.8rem"><span class="mark" style="width:46px;height:46px;border-radius:12px;background:var(--teal-accent);color:var(--side);display:grid;place-items:center">${icon("cart")}</span><b style="font-family:var(--font-display);font-size:1.5rem">ShopPro</b></div>
        <div><h1>Quản lý cửa hàng trong một màn hình.</h1><p>Theo dõi doanh thu, xử lý đơn hàng, cập nhật kho và xem nhật ký thao tác của mọi tài khoản.</p>
          <ul><li>${icon("chart")}Biểu đồ doanh thu và trạng thái đơn theo thời gian thực</li><li>${icon("receipt")}Chuyển trạng thái đơn đúng quy trình</li><li>${icon("log")}Nhật ký mọi thao tác quản trị</li></ul></div>
        <small style="color:#9fc3c7">Khu vực dành riêng cho quản trị viên.</small>
      </section>
      <section class="login-form"><div class="card">
        <h2>Đăng nhập quản trị</h2><p class="muted">Dùng tài khoản quản trị viên của cửa hàng.</p>
        ${message ? html`<div class="form-error" role="alert" style="margin-top:1rem">${icon("alert")}<span>${message}</span></div>` : ""}
        <div class="demo-box"><div><b>Tài khoản demo</b><br><span class="num">${DEMO.email}</span> · <span class="num">${DEMO.password}</span></div><button class="btn btn-secondary btn-sm" type="button" data-demo>Điền sẵn</button></div>
        <form class="stack" id="login-form" novalidate>
          <div class="field"><label for="email">Email</label><input class="input" id="email" name="email" type="email" required maxlength="160" autocomplete="username"></div>
          <div class="field"><label for="password">Mật khẩu</label><div class="pw"><input class="input" id="password" name="password" type="password" required maxlength="128" autocomplete="current-password"><button type="button" data-toggle-pw aria-label="Hiện hoặc ẩn mật khẩu">${icon("eye")}</button></div></div>
          <button class="btn btn-primary btn-lg btn-block" type="submit">Đăng nhập</button>
        </form>
      </div></section>
    </div>`);
  const form = $("#login-form");
  $("[data-demo]").addEventListener("click", () => { form.email.value = DEMO.email; form.password.value = DEMO.password; form.password.focus(); });
  $("[data-toggle-pw]").addEventListener("click", (event) => {
    const show = form.password.type === "password";
    form.password.type = show ? "text" : "password";
    event.currentTarget.innerHTML = icon(show ? "eyeoff" : "eye").toString();
  });
  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    showFormError(form, "");
    const { email, password } = formValues(form);
    if (!email.trim() || !password) { showFormError(form, "Vui lòng nhập email và mật khẩu."); return; }
    await withLoading(form.querySelector("button[type=submit]"), async () => {
      try {
        const result = await api("/admin/auth/login", { method: "POST", body: { email: email.trim(), password } });
        tokenStore.set(result.token);
        session.admin = result.user;
        startShell();
      } catch (error) { showFormError(form, error.retryAfter ? `${error.message} (thử lại sau ${error.retryAfter} giây)` : error.message); }
    });
  });
  form.email.focus();
}

/* ---------------------------------- Khung ---------------------------------- */
function renderNav() {
  const here = (location.hash.replace(/^#/, "") || "/").split("?")[0];
  const isCurrent = (path) => (path === "/" ? here === "/" : here === path || here.startsWith(`${path}/`));
  const nav = $("#side-nav");
  if (!nav) return;
  mount(nav, NAV.map(([path, label, ic]) => html`<a href="#${path}" ${isCurrent(path) ? html`aria-current="page"` : ""}>${icon(ic)}<span>${label}</span>${path === "/orders" && session.pendingOrders ? html`<span class="count" aria-label="${session.pendingOrders} đơn chờ xác nhận">${session.pendingOrders}</span>` : ""}</a>`));
}

function startShell() {
  mount(root, html`
    <div class="mobile-bar"><button class="icon-btn" type="button" data-menu aria-label="Mở menu" aria-expanded="false">${icon("menu")}</button><b>ShopPro Quản trị</b></div>
    <div class="shell">
      <aside class="sidebar" id="sidebar" aria-label="Menu quản trị">
        <a class="brand" href="#/"><span class="mark">${icon("cart")}</span><span>ShopPro<small>Khu vực quản trị</small></span></a>
        <nav class="side-nav" id="side-nav" aria-label="Điều hướng"></nav>
        <div class="side-foot">
          <div class="who"><b>${session.admin.name}</b><span>${session.admin.email}</span></div>
          <button class="btn btn-sm" type="button" data-logout>${icon("logout")}Đăng xuất</button>
        </div>
      </aside>
      <main class="content" id="content" tabindex="-1"></main>
    </div>`);
  $("[data-menu]").addEventListener("click", (event) => {
    const open = $("#sidebar").classList.toggle("open");
    event.currentTarget.setAttribute("aria-expanded", String(open));
  });
  $("#sidebar").addEventListener("click", (event) => { if (event.target.closest("a")) $("#sidebar").classList.remove("open"); });
  $("[data-logout]").addEventListener("click", logout);
  renderNav();
  startPolling();
  router.start();
}

async function logout() {
  if (!(await confirmDialog({ title: "Đăng xuất", message: "Bạn muốn đăng xuất khỏi khu vực quản trị?", confirmLabel: "Đăng xuất" }))) return;
  try { await api("/admin/auth/logout", { method: "POST" }); } catch { /* phiên có thể đã hết hạn */ }
  endSession();
}
function endSession(message) {
  tokenStore.clear();
  session.admin = null;
  session.pendingOrders = 0;
  location.hash = "";
  showLogin(message);
}

/* --------------- Thăm dò đơn mới: nhắc khi số đơn chờ xác nhận tăng --------------- */
async function poll(first = false) {
  try {
    const stats = await api("/admin/dashboard");
    if (!first && stats.pendingOrders > session.pendingOrders) {
      const diff = stats.pendingOrders - session.pendingOrders;
      toast(`Có ${diff} đơn hàng mới đang chờ xác nhận.`, "info", { action: { href: "#/orders?status=pending", label: "Xem ngay" } });
    }
    session.pendingOrders = stats.pendingOrders;
    renderNav();
  } catch { /* bỏ qua lỗi mạng tạm thời */ }
}
function startPolling() { stopPolling(); poll(true); pollTimer = setInterval(poll, 15000); }
function stopPolling() { if (pollTimer) clearInterval(pollTimer); pollTimer = null; }
document.addEventListener("orders-changed", () => poll(true));

/* ------------------------------- Định tuyến ------------------------------- */
const content = () => $("#content");
// Mỗi lần chuyển trang dùng một phần tử #content mới để không dồn trình nghe sự kiện của các trang trước
const run = (view) => (params, query, isCurrent) => {
  const stale = content();
  if (!stale) return;
  const main = stale.cloneNode(false);
  stale.replaceWith(main);
  document.title = `${TITLES[currentPath()] || "Quản trị"} – ShopPro Quản trị`;
  return view({ main, params, query, isCurrent });
};
const currentPath = () => "/" + ((location.hash.replace(/^#\/?/, "").split("?")[0]) || "");
router = createRouter([
  ["/", run(dashboardView)], ["/orders", run(ordersView)], ["/products", run(productsView)], ["/categories", run(categoriesView)],
  ["/customers", run(customersView)], ["/reports", run(reportsView)], ["/logs", run(logsView)], ["/settings", run(settingsView)],
], {
  onNavigate() { document.querySelectorAll("dialog.modal[open]").forEach((d) => d.close()); renderNav(); $("#tooltip").hidden = true; window.scrollTo({ top: 0 }); },
  notFound() { const main = content(); if (main) mount(main, html`<div class="empty"><h2>Không tìm thấy trang</h2><a class="btn btn-primary" href="#/">Về tổng quan</a></div>`); },
});

events.addEventListener("unauthorized", () => { if (session.admin) { toast("Phiên đăng nhập đã hết hạn.", "info"); endSession("Phiên đăng nhập đã hết hạn. Vui lòng đăng nhập lại."); } });

/* ------------------------------- Khởi động ------------------------------- */
(async function boot() {
  wireChartInteractions();
  if (tokenStore.get()) {
    try { session.admin = await api("/admin/auth/me"); startShell(); return; }
    catch { tokenStore.clear(); }
  }
  showLogin();
})();
