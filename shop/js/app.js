// Điểm vào của ứng dụng Cửa hàng: khung trang, định tuyến, sự kiện toàn cục.
import { $, $$, html, icon, mount, createRouter, toast, go, withLoading, confirmDialog, ApiError } from "/assets/shared.js";
import { state, bus, api, restoreSession, clearSession, addToCart, requireLogin } from "./state.js";
import { homeView } from "./views/home.js";
import { productsView } from "./views/products.js";
import { productView } from "./views/product.js";
import { cartView } from "./views/cart.js";
import { checkoutView } from "./views/checkout.js";
import { ordersView, orderDetailView } from "./views/orders.js";
import { loginView, registerView, accountView } from "./views/account.js";
import { assistantView } from "./views/assistant.js";

const main = $("#main");
const header = $("#site-header");
const footer = $("#site-footer");

/* ----------------------------- Khung trang ----------------------------- */
function renderHeader() {
  const navItems = [
    ["/", "Trang chủ", "home"],
    ["/products", "Tất cả sản phẩm", "store"],
    ["/assistant", "Trợ lý mua sắm", "bot"],
    ["/orders", "Đơn hàng của tôi", "receipt"],
  ];
  const here = (location.hash.replace(/^#/, "") || "/").split("?")[0];
  const isCurrent = (path) => (path === "/" ? here === "/" : here === path || here.startsWith(`${path}/`));
  mount(header, html`
    <div class="topbar"><div class="wrap"><span>Miễn phí giao hàng cho đơn từ 500.000 ₫</span><span>Hotline: <a href="tel:${state.store.supportPhone.replace(/\s/g, "")}">${state.store.supportPhone}</a></span></div></div>
    <div class="wrap header-main">
      <a class="brand" href="#/" aria-label="ShopPro – về trang chủ">
        <span class="brand-mark">${icon("cart")}</span>
        <span>${state.store.storeName}<small>Mua sắm rõ ràng, giao nhanh</small></span>
      </a>
      <form class="search" role="search" data-search-form>
        <label class="sr-only" for="q">Tìm sản phẩm</label>
        <input id="q" name="q" type="search" placeholder="Tìm tai nghe, áo thun, sách…" autocomplete="off" maxlength="100" value="${currentSearch()}">
        <button type="submit" aria-label="Tìm kiếm">${icon("search")}</button>
      </form>
      <div class="header-actions">
        <a class="hbtn" href="#/cart" aria-label="Giỏ hàng${state.cartQuantity ? `, ${state.cartQuantity} sản phẩm` : ""}">${icon("cart")}<span class="t">Giỏ hàng</span>${state.cartQuantity ? html`<span class="count">${state.cartQuantity > 99 ? "99+" : state.cartQuantity}</span>` : ""}</a>
        ${state.user
          ? html`<a class="hbtn" href="#/account">${icon("user")}<span class="t">${firstName(state.user.name)}</span></a>
                 <button class="hbtn" type="button" data-logout aria-label="Đăng xuất">${icon("logout")}<span class="t">Thoát</span></button>`
          : html`<a class="hbtn" href="#/login">${icon("user")}<span class="t">Đăng nhập</span></a>`}
      </div>
    </div>
    <nav class="nav" aria-label="Điều hướng chính"><div class="wrap">
      ${navItems.map(([path, label, ic]) => html`<a href="#${path}" ${isCurrent(path) ? html`aria-current="page"` : ""}>${icon(ic)}${label}</a>`)}
    </div></nav>`);
}
const firstName = (name) => String(name || "").trim().split(/\s+/).slice(-1)[0] || "Tài khoản";
function currentSearch() {
  const hash = location.hash.replace(/^#/, "");
  if (!hash.startsWith("/products")) return "";
  return new URLSearchParams(hash.split("?")[1] || "").get("search") || "";
}

function renderFooter() {
  mount(footer, html`
    <div class="wrap">
      <div><h3>${state.store.storeName}</h3><p>Cửa hàng trực tuyến demo cho môn Cấu trúc dữ liệu &amp; Giải thuật. Giỏ hàng được quản lý bằng danh sách liên kết đôi cài đặt bằng C++.</p></div>
      <div><h3>Mua sắm</h3><ul><li><a href="#/products">Tất cả sản phẩm</a></li><li><a href="#/assistant">Trợ lý mua sắm</a></li><li><a href="#/orders">Theo dõi đơn hàng</a></li></ul></div>
      <div><h3>Hỗ trợ</h3><ul><li>Hotline: <a href="tel:${state.store.supportPhone.replace(/\s/g, "")}">${state.store.supportPhone}</a></li><li><a href="mailto:${state.store.supportEmail}">${state.store.supportEmail}</a></li><li>8:00 – 21:00 mỗi ngày</li></ul></div>
    </div>
    <div class="legal"><div class="wrap">© ${new Date().getFullYear()} ${state.store.storeName}. Dữ liệu trong hệ thống chỉ dùng để trình diễn.</div></div>`);
}

/* ---------------------------- Định tuyến ---------------------------- */
function setTitle(text) { document.title = text ? `${text} – ShopPro` : "ShopPro – Cửa hàng trực tuyến"; }
const run = (view) => (params, query, isCurrent) => view({ params, query, isCurrent, main, setTitle });

const router = createRouter([
  ["/", run(homeView)],
  ["/products", run(productsView)],
  ["/product/:id", run(productView)],
  ["/cart", run(cartView)],
  ["/checkout", run(checkoutView)],
  ["/orders", run(ordersView)],
  ["/orders/:id", run(orderDetailView)],
  ["/login", run(loginView)],
  ["/register", run(registerView)],
  ["/account", run(accountView)],
  ["/assistant", run(assistantView)],
], {
  onNavigate() { document.querySelectorAll("dialog.modal[open]").forEach((d) => d.close()); renderHeader(); window.scrollTo({ top: 0 }); },
  notFound() {
    setTitle("Không tìm thấy trang");
    mount(main, html`<div class="wrap"><div class="empty"><h2>Không tìm thấy trang này</h2><p>Đường dẫn có thể đã bị đổi hoặc nhập sai.</p><a class="btn btn-primary" href="#/">Về trang chủ</a></div></div>`);
  },
});

/* ------------------------------ Sự kiện ------------------------------ */
document.addEventListener("click", async (event) => {
  const addButton = event.target.closest("[data-add]");
  if (addButton) {
    event.preventDefault();
    if (!requireLogin()) { toast("Hãy đăng nhập để thêm sản phẩm vào giỏ.", "info"); return; }
    await withLoading(addButton, async () => {
      try {
        await addToCart(addButton.dataset.add, Number(addButton.dataset.qty) || 1);
        toast("Đã thêm vào giỏ hàng.", "success", { action: { href: "#/cart", label: "Xem giỏ" } });
      } catch (error) { toast(error.message, "error"); }
    });
    return;
  }
  if (event.target.closest("[data-logout]")) {
    if (!(await confirmDialog({ title: "Đăng xuất", message: "Bạn có chắc muốn đăng xuất khỏi tài khoản?", confirmLabel: "Đăng xuất" }))) return;
    try { await api("/auth/logout", { method: "POST" }); } catch { /* phiên có thể đã hết hạn */ }
    clearSession();
    toast("Đã đăng xuất.", "info");
    go("/");
  }
});
document.addEventListener("submit", (event) => {
  const form = event.target.closest("[data-search-form]");
  if (!form) return;
  event.preventDefault();
  const term = new FormData(form).get("q").toString().trim();
  go(term ? `/products?search=${encodeURIComponent(term)}` : "/products");
});
bus.addEventListener("cart", renderHeader);
bus.addEventListener("auth", renderHeader);
document.addEventListener("profile-saved", renderHeader);
window.addEventListener("error", () => { /* lỗi giao diện bất ngờ không được làm trắng trang */ });

/* ------------------------------- Khởi động ------------------------------- */
(async function boot() {
  mount(main, html`<div class="loading-block"><div class="spinner" role="status" aria-label="Đang tải"></div></div>`);
  renderHeader();
  try { Object.assign(state.store, await api("/store")); } catch { /* dùng giá trị mặc định */ }
  try { state.categories = await api("/categories"); } catch (error) { if (!(error instanceof ApiError)) throw error; }
  await restoreSession();
  renderHeader();
  renderFooter();
  router.start();
})();
