// Trạng thái toàn cục của ứng dụng Cửa hàng: phiên đăng nhập, giỏ hàng, API.
import { createApi, toast, go } from "/assets/shared.js";

const TOKEN_KEY = "shoppro.customer.token";   // khác khóa của Admin để hai giao diện chạy song song cùng một trình duyệt

const storage = {
  get() { try { return localStorage.getItem(TOKEN_KEY) || ""; } catch { return ""; } },
  set(value) { try { localStorage.setItem(TOKEN_KEY, value); } catch { /* chế độ riêng tư */ } },
  clear() { try { localStorage.removeItem(TOKEN_KEY); } catch { /* bỏ qua */ } },
};

export const state = { user: null, cartQuantity: 0, categories: [], store: { storeName: "ShopPro", supportPhone: "1900 1234", supportEmail: "support@shoppro.vn" } };
export const bus = new EventTarget();
const emit = (name) => bus.dispatchEvent(new Event(name));

export const api = createApi({
  getToken: storage.get,
  onUnauthorized() {
    if (!state.user) return;
    clearSession();
    toast("Phiên đăng nhập đã hết hạn. Vui lòng đăng nhập lại.", "info");
    go("/login");
  },
});

export function setSession({ user, token }) {
  storage.set(token);
  state.user = user;
  emit("auth");
}
export function clearSession() {
  storage.clear();
  try { sessionStorage.removeItem("shoppro.coupon"); } catch { /* bỏ qua */ }   // mã giảm giá đang áp dụng (xem coupon.js)
  state.user = null;
  state.cartQuantity = 0;
  emit("auth");
  emit("cart");
}
export function setCartQuantity(quantity) {
  state.cartQuantity = quantity;
  emit("cart");
}

export async function restoreSession() {
  if (!storage.get()) return;
  try {
    state.user = await api("/auth/me");
    const cart = await api("/cart");
    state.cartQuantity = cart.totalQuantity;
  } catch {
    storage.clear();
    state.user = null;
  }
  emit("auth");
  emit("cart");
}

// Trả về true nếu đã đăng nhập; chưa thì chuyển sang trang đăng nhập và nhớ trang cần quay lại
export function requireLogin() {
  if (state.user) return true;
  const here = location.hash.replace(/^#/, "") || "/";
  go(`/login?next=${encodeURIComponent(here)}`);
  return false;
}

export async function addToCart(productId, quantity = 1) {
  if (!state.user) { requireLogin(); return null; }
  const cart = await api("/cart/items", { method: "POST", body: { productId: Number(productId), quantity } });
  setCartQuantity(cart.totalQuantity);
  return cart;
}
