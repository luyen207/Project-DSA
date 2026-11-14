// ShopPro – thư viện dùng chung cho Cửa hàng và Quản trị (ES module, không cần build).
// Mọi dữ liệu người dùng đi qua html`...` đều bị escape tự động -> chống XSS.

/* ============================== HTML an toàn ============================== */
class SafeHtml { constructor(value) { this.value = value; } toString() { return this.value; } }
const ESC = { "&": "&amp;", "<": "&lt;", ">": "&gt;", '"': "&quot;", "'": "&#39;", "`": "&#96;" };
export const esc = (value) => String(value ?? "").replace(/[&<>"'`]/g, (ch) => ESC[ch]);
export const raw = (value) => new SafeHtml(String(value));
function render(value) {
  if (value === null || value === undefined || value === false || value === true) return "";
  if (value instanceof SafeHtml) return value.value;
  if (Array.isArray(value)) return value.map(render).join("");
  return esc(value);
}
export function html(strings, ...values) {
  let out = "";
  strings.forEach((chunk, i) => { out += chunk; if (i < values.length) out += render(values[i]); });
  return new SafeHtml(out);
}
export function mount(element, content) { element.innerHTML = render(content); return element; }
// Chỉ chấp nhận đường dẫn nội bộ hoặc http(s) cho ảnh
export function safeSrc(url) {
  const text = String(url ?? "");
  if ((text.startsWith("/") && !text.startsWith("//")) || /^https?:\/\//i.test(text)) return text;
  return "/img/products/placeholder.svg";
}

/* ================================== Biểu tượng ================================== */
const ICONS = {
  search: '<circle cx="11" cy="11" r="7"/><path d="m21 21-4.3-4.3"/>',
  cart: '<circle cx="9" cy="20" r="1.5"/><circle cx="18" cy="20" r="1.5"/><path d="M2 3h3l2.6 12.4a2 2 0 0 0 2 1.6h7.7a2 2 0 0 0 2-1.5L21 8H6"/>',
  user: '<circle cx="12" cy="8" r="4"/><path d="M4 21a8 8 0 0 1 16 0"/>',
  home: '<path d="M3 11 12 3l9 8"/><path d="M5 10v10h14V10"/><path d="M10 20v-6h4v6"/>',
  package: '<path d="M21 8 12 3 3 8v8l9 5 9-5z"/><path d="m3 8 9 5 9-5M12 13v8"/>',
  chart: '<path d="M4 20V10M10 20V4M16 20v-7M22 20H2"/>',
  users: '<circle cx="9" cy="8" r="3.5"/><path d="M2.5 20a6.5 6.5 0 0 1 13 0"/><path d="M16 4.5a3.5 3.5 0 0 1 0 7M18 14.5a6.5 6.5 0 0 1 3.5 5.5"/>',
  tag: '<path d="M3 12V4h8l10 10-8 8z"/><circle cx="7.5" cy="8.5" r="1.3"/>',
  settings: '<path d="M4 6h10M18 6h2M4 12h4M12 12h8M4 18h12M20 18h0"/><circle cx="16" cy="6" r="2"/><circle cx="10" cy="12" r="2"/><circle cx="18" cy="18" r="2"/>',
  log: '<path d="M8 6h13M8 12h13M8 18h13"/><circle cx="3.5" cy="6" r="1"/><circle cx="3.5" cy="12" r="1"/><circle cx="3.5" cy="18" r="1"/>',
  logout: '<path d="M9 21H5a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h4"/><path d="m16 17 5-5-5-5M21 12H9"/>',
  plus: '<path d="M12 5v14M5 12h14"/>',
  minus: '<path d="M5 12h14"/>',
  trash: '<path d="M3 6h18M8 6V4h8v2M6 6l1 14h10l1-14M10 11v6M14 11v6"/>',
  edit: '<path d="M12 20h9"/><path d="M16.5 3.5a2.1 2.1 0 0 1 3 3L7 19l-4 1 1-4z"/>',
  check: '<path d="m4 12 5 5L20 6"/>',
  x: '<path d="M6 6l12 12M18 6 6 18"/>',
  eye: '<path d="M2 12s4-7 10-7 10 7 10 7-4 7-10 7S2 12 2 12z"/><circle cx="12" cy="12" r="3"/>',
  eyeoff: '<path d="M3 3l18 18"/><path d="M10.6 5.1A9.7 9.7 0 0 1 12 5c6 0 10 7 10 7a17 17 0 0 1-3.2 4M6.6 6.6A17 17 0 0 0 2 12s4 7 10 7a9.6 9.6 0 0 0 4-.9"/>',
  lock: '<rect x="4" y="11" width="16" height="10" rx="2"/><path d="M8 11V7a4 4 0 0 1 8 0v4"/>',
  unlock: '<rect x="4" y="11" width="16" height="10" rx="2"/><path d="M8 11V7a4 4 0 0 1 7.5-2"/>',
  "chevron-left": '<path d="m15 6-6 6 6 6"/>',
  "chevron-right": '<path d="m9 6 6 6-6 6"/>',
  "chevron-down": '<path d="m6 9 6 6 6-6"/>',
  "arrow-right": '<path d="M5 12h14M13 6l6 6-6 6"/>',
  "arrow-left": '<path d="M19 12H5M11 6l-6 6 6 6"/>',
  swap: '<path d="M7 7h13l-4-4M17 17H4l4 4"/>',
  bot: '<rect x="4" y="8" width="16" height="12" rx="3"/><path d="M12 4v4M9 14h.01M15 14h.01M9 17.5h6"/><circle cx="12" cy="3.5" r="1"/>',
  truck: '<path d="M2 6h11v10H2zM13 9h5l4 4v3h-9z"/><circle cx="7" cy="18" r="2"/><circle cx="17" cy="18" r="2"/>',
  clock: '<circle cx="12" cy="12" r="9"/><path d="M12 7v5l3 2"/>',
  alert: '<path d="M12 3 2 20h20z"/><path d="M12 10v5M12 18h.01"/>',
  info: '<circle cx="12" cy="12" r="9"/><path d="M12 11v6M12 7.5h.01"/>',
  link: '<path d="M10 14a4 4 0 0 0 5.7 0l3-3a4 4 0 0 0-5.7-5.7l-1 1"/><path d="M14 10a4 4 0 0 0-5.7 0l-3 3a4 4 0 0 0 5.7 5.7l1-1"/>',
  sort: '<path d="M7 4v16M3 8l4-4 4 4M17 20V4M13 16l4 4 4-4"/>',
  menu: '<path d="M3 6h18M3 12h18M3 18h18"/>',
  image: '<rect x="3" y="4" width="18" height="16" rx="2"/><circle cx="9" cy="10" r="1.7"/><path d="m21 16-5-5-8 8"/>',
  refresh: '<path d="M20 11a8 8 0 0 0-14-4L4 9M4 5v4h4M4 13a8 8 0 0 0 14 4l2-2M20 19v-4h-4"/>',
  download: '<path d="M12 3v12M7 11l5 5 5-5M4 21h16"/>',
  shield: '<path d="M12 3 4 6v6c0 5 3.5 8 8 9 4.5-1 8-4 8-9V6z"/><path d="m9 12 2 2 4-4"/>',
  phone: '<path d="M5 3h4l2 5-2.5 1.5a11 11 0 0 0 6 6L16 13l5 2v4a2 2 0 0 1-2 2A16 16 0 0 1 3 5a2 2 0 0 1 2-2z"/>',
  mail: '<rect x="3" y="5" width="18" height="14" rx="2"/><path d="m3 7 9 6 9-6"/>',
  map: '<path d="M12 21s7-6 7-11a7 7 0 0 0-14 0c0 5 7 11 7 11z"/><circle cx="12" cy="10" r="2.5"/>',
  filter: '<path d="M3 5h18l-7 8v6l-4 2v-8z"/>',
  store: '<path d="M3 9 5 4h14l2 5M3 9v11h18V9M3 9h18"/><path d="M9 20v-6h6v6"/>',
  money: '<rect x="2" y="6" width="20" height="12" rx="2"/><circle cx="12" cy="12" r="2.5"/>',
  bell: '<path d="M6 17V11a6 6 0 0 1 12 0v6l2 2H4z"/><path d="M10 21h4"/>',
  calendar: '<rect x="3" y="5" width="18" height="16" rx="2"/><path d="M3 10h18M8 3v4M16 3v4"/>',
  send: '<path d="M22 2 11 13M22 2l-7 20-4-9-9-4z"/>',
  sparkle: '<path d="M12 3v4M12 17v4M3 12h4M17 12h4M6 6l2.5 2.5M15.5 15.5 18 18M18 6l-2.5 2.5M8.5 15.5 6 18"/>',
  box: '<path d="M4 8h16v12H4zM2 4h20v4H2zM10 12h4"/>',
  warehouse: '<path d="M3 21V9l9-5 9 5v12"/><path d="M7 21v-7h10v7M7 17.5h10"/>',
  receipt: '<path d="M5 3h14v18l-3-2-2 2-2-2-2 2-2-2-3 2z"/><path d="M9 8h6M9 12h6"/>',
  ticket: '<path d="M3 9a2.5 2.5 0 0 1 0 6v3a1 1 0 0 0 1 1h16a1 1 0 0 0 1-1v-3a2.5 2.5 0 0 1 0-6V6a1 1 0 0 0-1-1H4a1 1 0 0 0-1 1z"/><path d="M14 5v2.5M14 11v2M14 16.5V19"/>',
  list: '<path d="M4 6h16M4 12h16M4 18h10"/>',
};
export function icon(name, className = "") {
  const body = ICONS[name] || ICONS.info;
  return raw(`<svg class="ico ${esc(className)}" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" focusable="false">${body}</svg>`);
}

/* ================================== Định dạng ================================== */
const vndFormatter = new Intl.NumberFormat("vi-VN");
export const vnd = (value) => `${vndFormatter.format(Math.round(Number(value) || 0))}\u00a0₫`;   // dấu cách không ngắt dòng: ₫ luôn đi liền số
export const number = (value) => vndFormatter.format(Number(value) || 0);
export function compactVnd(value) {
  const n = Number(value) || 0;
  if (n >= 1e9) return `${(n / 1e9).toFixed(1).replace(".", ",")} tỷ`;
  if (n >= 1e6) return `${(n / 1e6).toFixed(n >= 1e7 ? 0 : 1).replace(".", ",")} tr`;
  if (n >= 1e3) return `${Math.round(n / 1e3)}k`;
  return String(n);
}
const pad = (n) => String(n).padStart(2, "0");
export function dateTime(iso) {
  const d = new Date(iso);
  if (Number.isNaN(d.getTime())) return "";
  return `${pad(d.getDate())}/${pad(d.getMonth() + 1)}/${d.getFullYear()} ${pad(d.getHours())}:${pad(d.getMinutes())}`;
}
export function dateOnly(iso) {
  const d = new Date(iso);
  if (Number.isNaN(d.getTime())) return "";
  return `${pad(d.getDate())}/${pad(d.getMonth() + 1)}/${d.getFullYear()}`;
}
export function timeAgo(iso) {
  const seconds = Math.max(0, (Date.now() - new Date(iso).getTime()) / 1000);
  if (seconds < 60) return "vừa xong";
  if (seconds < 3600) return `${Math.floor(seconds / 60)} phút trước`;
  if (seconds < 86400) return `${Math.floor(seconds / 3600)} giờ trước`;
  if (seconds < 86400 * 30) return `${Math.floor(seconds / 86400)} ngày trước`;
  return dateOnly(iso);
}
export const isoDate = (date) => `${date.getFullYear()}-${pad(date.getMonth() + 1)}-${pad(date.getDate())}`;

export const STATUS_LABEL = { pending: "Chờ xác nhận", confirmed: "Đã xác nhận", shipping: "Đang giao", completed: "Hoàn thành", cancelled: "Đã hủy" };
export const statusBadge = (status) => html`<span class="badge st-${status}">${STATUS_LABEL[status] || status}</span>`;
export const PAYMENT_LABEL = { cod: "Thanh toán khi nhận hàng", bank: "Chuyển khoản ngân hàng" };

/* ===================================== API ===================================== */
export class ApiError extends Error {
  constructor(status, message, retryAfter = 0) { super(message); this.status = status; this.retryAfter = retryAfter; }
}
export function createApi({ getToken, onUnauthorized }) {
  return async function api(path, { method = "GET", body, signal } = {}) {
    const headers = { Accept: "application/json" };
    const token = getToken();
    if (token) headers.Authorization = `Bearer ${token}`;
    if (body !== undefined) headers["Content-Type"] = "application/json";
    const controller = new AbortController();
    const timer = setTimeout(() => controller.abort(), 20000);
    if (signal) signal.addEventListener("abort", () => controller.abort());
    let response;
    try {
      response = await fetch(`/api${path}`, { method, headers, body: body === undefined ? undefined : JSON.stringify(body), signal: controller.signal });
    } catch (error) {
      throw new ApiError(0, error.name === "AbortError" ? "Máy chủ phản hồi quá lâu. Vui lòng thử lại." : "Không kết nối được máy chủ. Kiểm tra mạng hoặc chắc chắn backend đang chạy.");
    } finally { clearTimeout(timer); }
    let data = null;
    const text = await response.text();
    if (text) { try { data = JSON.parse(text); } catch { data = null; } }
    if (!response.ok) {
      if (response.status === 401 && token) onUnauthorized?.();
      throw new ApiError(response.status, data?.error || `Lỗi ${response.status}`, Number(response.headers.get("Retry-After")) || 0);
    }
    return data;
  };
}

/* ================================= Giao diện chung ================================= */
export const $ = (selector, root = document) => root.querySelector(selector);
export const $$ = (selector, root = document) => [...root.querySelectorAll(selector)];
export function debounce(fn, wait = 300) {
  let timer;
  return (...args) => { clearTimeout(timer); timer = setTimeout(() => fn(...args), wait); };
}

let toastHost;
export function toast(message, type = "success", { action } = {}) {
  if (!toastHost) {
    toastHost = document.createElement("div");
    toastHost.className = "toasts";
    toastHost.setAttribute("role", "status");
    toastHost.setAttribute("aria-live", "polite");
    document.body.append(toastHost);
  }
  const node = document.createElement("div");
  node.className = `toast ${type}`;
  const iconName = type === "error" ? "alert" : type === "info" ? "info" : "check";
  node.innerHTML = render(html`${icon(iconName)}<div>${message}${action ? html`<a class="t-action" href="${action.href}">${action.label}</a>` : ""}</div>`);
  toastHost.append(node);
  setTimeout(() => { node.style.transition = "opacity .25s"; node.style.opacity = "0"; setTimeout(() => node.remove(), 260); }, type === "error" ? 7000 : 4200);
}

// Hộp thoại dựa trên <dialog> (có sẵn bẫy bàn phím và phím Esc)
export function openModal({ title, body, footer, size = "", onClose }) {
  const dialog = document.createElement("dialog");
  dialog.className = `modal ${size}`;
  dialog.innerHTML = render(html`
    <div class="modal-head"><h2>${title}</h2><button type="button" class="icon-btn" data-modal-close aria-label="Đóng">${icon("x")}</button></div>
    <div class="modal-body">${body}</div>
    ${footer ? html`<div class="modal-foot">${footer}</div>` : ""}`);
  dialog.addEventListener("click", (event) => {
    if (event.target === dialog || event.target.closest("[data-modal-close]")) dialog.close();
  });
  dialog.addEventListener("close", () => { dialog.remove(); onClose?.(); });
  document.body.append(dialog);
  dialog.showModal();
  return dialog;
}
export function confirmDialog({ title, message, confirmLabel = "Xác nhận", danger = false }) {
  return new Promise((resolve) => {
    let answer = false;
    const dialog = openModal({
      title, size: "narrow",
      body: html`<p>${message}</p>`,
      footer: html`<button class="btn btn-secondary" data-modal-close type="button">Hủy bỏ</button>
                   <button class="btn ${danger ? "btn-danger" : "btn-primary"}" data-confirm type="button">${confirmLabel}</button>`,
      onClose: () => resolve(answer),
    });
    dialog.querySelector("[data-confirm]").addEventListener("click", () => { answer = true; dialog.close(); });
    dialog.querySelector("[data-confirm]").focus();
  });
}

export async function withLoading(button, task) {
  if (button) { button.classList.add("is-loading"); button.disabled = true; }
  try { return await task(); }
  finally { if (button) { button.classList.remove("is-loading"); button.disabled = false; } }
}

export const formValues = (form) => Object.fromEntries(new FormData(form).entries());
export function showFormError(form, message) {
  let box = form.querySelector("[data-form-error]");
  if (!message) { box?.remove(); return; }
  if (!box) {
    box = document.createElement("div");
    box.dataset.formError = "";
    box.setAttribute("role", "alert");
    form.prepend(box);
  }
  box.className = "form-error";
  box.innerHTML = render(html`${icon("alert")}<span>${message}</span>`);
  box.scrollIntoView({ block: "nearest", behavior: "smooth" });
}

export function emptyState({ title, text, iconName = "package", action }) {
  return html`<div class="empty">${icon(iconName, "big")}<h3>${title}</h3>${text ? html`<p>${text}</p>` : ""}${action || ""}</div>`;
}
export const skeletonRows = (count, height = 22) => html`<div class="stack">${Array.from({ length: count }, () => html`<div class="skeleton" style="height:${height}px"></div>`)}</div>`;

/* ===================================== Router ===================================== */
// Đường dẫn dạng "#/products?page=2"; mẫu có tham số: "/product/:id"
export function createRouter(routes, { onNavigate, notFound }) {
  const compiled = routes.map(([pattern, handler]) => {
    const names = [];
    const regex = new RegExp("^" + pattern.replace(/:([a-zA-Z]+)/g, (_, name) => { names.push(name); return "([^/]+)"; }) + "/?$");
    return { regex, names, handler };
  });
  const parse = () => {
    const hash = location.hash.replace(/^#/, "") || "/";
    const [path, queryText = ""] = hash.split("?");
    return { path: decodeURIComponent(path) || "/", query: Object.fromEntries(new URLSearchParams(queryText)) };
  };
  let token = 0;
  async function dispatch() {
    const { path, query } = parse();
    const current = ++token;
    for (const route of compiled) {
      const match = route.regex.exec(path);
      if (!match) continue;
      const params = {};
      route.names.forEach((name, i) => { params[name] = decodeURIComponent(match[i + 1]); });
      onNavigate?.(path, query);
      await route.handler(params, query, () => current === token);   // isCurrent(): bỏ kết quả của trang cũ
      return;
    }
    onNavigate?.(path, query);
    notFound?.(path);
  }
  window.addEventListener("hashchange", dispatch);
  return { start: dispatch, refresh: dispatch, current: parse };
}
export const go = (path) => { location.hash = path; };
export function queryString(params) {
  const search = new URLSearchParams();
  Object.entries(params).forEach(([key, value]) => { if (value !== "" && value !== null && value !== undefined) search.set(key, value); });
  const text = search.toString();
  return text ? `?${text}` : "";
}
