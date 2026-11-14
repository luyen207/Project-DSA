// Trạng thái & tiện ích dùng chung cho giao diện Quản trị.
import { createApi, html, icon } from "/assets/shared.js";

const TOKEN_KEY = "shoppro.admin.token";   // khác khóa của Cửa hàng: hai giao diện chạy song song không đè phiên của nhau
export const tokenStore = {
  get() { try { return localStorage.getItem(TOKEN_KEY) || ""; } catch { return ""; } },
  set(value) { try { localStorage.setItem(TOKEN_KEY, value); } catch { /* bỏ qua */ } },
  clear() { try { localStorage.removeItem(TOKEN_KEY); } catch { /* bỏ qua */ } },
};
export const session = { admin: null, pendingOrders: 0 };
export const events = new EventTarget();
export const api = createApi({ getToken: tokenStore.get, onUnauthorized: () => events.dispatchEvent(new Event("unauthorized")) });

// Bỏ dấu tiếng Việt để tìm kiếm không phân biệt dấu
export function fold(text) {
  return String(text ?? "").normalize("NFD").replace(/[̀-ͯ]/g, "").replace(/đ/g, "d").replace(/Đ/g, "D").toLowerCase();
}
export function paginate(list, page, size) {
  const totalPages = Math.max(1, Math.ceil(list.length / size));
  const current = Math.min(Math.max(1, page), totalPages);
  return { items: list.slice((current - 1) * size, current * size), page: current, totalPages, total: list.length };
}
export function pager({ page, totalPages }, attr = "data-page") {
  if (totalPages <= 1) return "";
  const set = new Set([1, totalPages, page - 1, page, page + 1]);
  const list = [...set].filter((p) => p >= 1 && p <= totalPages).sort((a, b) => a - b);
  const parts = [];
  list.forEach((p, i) => {
    if (i > 0 && p - list[i - 1] > 1) parts.push(html`<span class="muted" aria-hidden="true">…</span>`);
    parts.push(html`<button class="btn btn-secondary btn-sm" type="button" ${attr}="${p}" ${p === page ? html`aria-current="page"` : ""}>${p}</button>`);
  });
  return html`<nav class="pagination" aria-label="Phân trang">
    ${page > 1 ? html`<button class="btn btn-secondary btn-sm" type="button" ${attr}="${page - 1}" aria-label="Trang trước">${icon("chevron-left")}</button>` : ""}
    ${parts}
    ${page < totalPages ? html`<button class="btn btn-secondary btn-sm" type="button" ${attr}="${page + 1}" aria-label="Trang sau">${icon("chevron-right")}</button>` : ""}
  </nav>`;
}
export const loadingBlock = html`<div class="loading-block"><div class="spinner" role="status" aria-label="Đang tải"></div></div>`;
export const errorBlock = (message) => html`<div class="form-error">${icon("alert")}<span>${message}</span></div>`;
export const digitsOnly = (value) => String(value ?? "").replace(/\D/g, "");
