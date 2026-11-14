// Mã giảm giá phía khách. Mã đang áp dụng được nhớ trong phiên trình duyệt (sessionStorage) để đi từ
// Giỏ hàng sang Thanh toán; mọi số tiền giảm đều do MÁY CHỦ tính (POST /api/coupons/validate), trang này chỉ hiển thị.
import { html, icon, vnd, toast } from "/assets/shared.js";
import { api } from "./state.js";

const KEY = "shoppro.coupon";   // state.js xóa cùng khóa này khi đăng xuất
export const savedCode = () => { try { return sessionStorage.getItem(KEY) || ""; } catch { return ""; } };
export const saveCode = (code) => { try { if (code) sessionStorage.setItem(KEY, code); else sessionStorage.removeItem(KEY); } catch { /* chế độ riêng tư */ } };

const validate = (code) => api("/coupons/validate", { method: "POST", body: { code } });

/* Khung nhập mã + danh sách mã đang dùng được (dùng chung cho Giỏ hàng và Thanh toán) */
function boxHtml(c, cartTotal) {
  const { applied, available, input, error, notice, busy } = c;
  return html`
    <section class="coupon" aria-labelledby="coupon-title">
      <h3 id="coupon-title">${icon("tag")}Mã giảm giá</h3>
      ${applied ? html`
        <div class="coupon-applied" role="status">
          ${icon("check")}
          <div><b>${applied.code}</b> · giảm ${applied.percent}%<small>Tiết kiệm ${vnd(applied.discount)}</small></div>
          <button class="link-btn" type="button" data-coupon-remove>Bỏ mã</button>
        </div>` : html`
        <div class="coupon-form">
          <label class="sr-only" for="coupon-code">Nhập mã giảm giá</label>
          <input class="input" id="coupon-code" value="${input}" maxlength="20" placeholder="Nhập mã giảm giá" autocomplete="off" autocapitalize="characters" spellcheck="false">
          <button class="btn btn-secondary" type="button" data-coupon-apply ${busy ? html`disabled` : ""}>Áp dụng</button>
        </div>`}
      ${error ? html`<p class="coupon-msg bad" role="alert">${icon("alert")}<span>${error}</span></p>` : ""}
      ${notice ? html`<p class="coupon-msg info" role="status">${icon("info")}<span>${notice}</span></p>` : ""}
      ${!applied && available.length ? html`
        <p class="coupon-hint">Mã bạn có thể dùng (bấm để áp dụng):</p>
        <ul class="coupon-list">${available.map((item) => {
          const short = Math.max(0, item.minOrder - cartTotal);
          const detail = [item.minOrder ? `Đơn từ ${vnd(item.minOrder)}` : "Mọi đơn hàng"];
          if (short > 0) detail.push(`cần mua thêm ${vnd(short)}`);
          if (item.remaining !== null) detail.push(`còn ${item.remaining} lượt`);
          return html`<li><button class="coupon-chip ${short > 0 ? "is-short" : ""}" type="button" data-coupon-pick="${item.code}">
            <b>${item.code}</b><span>Giảm ${item.percent}%</span><small>${detail.join(" · ")}</small></button></li>`;
        })}</ul>` : ""}
    </section>`;
}

/* Các dòng tiền của khung tóm tắt: tạm tính, giảm giá, tổng thanh toán */
export function totalsHtml(cartTotal, applied) {
  return html`
    <div class="sum-row"><span>Tạm tính</span><b>${vnd(cartTotal)}</b></div>
    ${applied ? html`<div class="sum-row discount"><span>Giảm giá ${applied.code} (−${applied.percent}%)</span><b>−${vnd(applied.discount)}</b></div>` : ""}
    <div class="sum-total"><b>Tổng thanh toán</b><span class="price">${vnd(applied ? applied.total : cartTotal)}</span></div>`;
}

/* Bộ điều khiển: giữ trạng thái và xử lý sự kiện cho một trang. `onChange` vẽ lại phần tóm tắt. */
export function createCoupon({ getTotal, onChange, isCurrent }) {
  const c = { applied: null, available: [], input: "", error: "", notice: "", busy: false };

  // Kiểm tra lại mã đang lưu với giỏ hàng hiện tại (gọi khi vào trang và mỗi khi giỏ thay đổi)
  c.refresh = async ({ withList = false } = {}) => {
    const code = savedCode();
    if (withList) { try { c.available = await api("/coupons"); } catch { c.available = []; } }
    if (!code || !getTotal()) { c.applied = null; return; }   // giỏ trống: giữ mã đã lưu, chưa kiểm tra
    try {
      c.applied = await validate(code);
      c.notice = "";
    } catch (error) {
      c.applied = null;
      saveCode("");
      c.notice = `Mã ${code} đã được gỡ khỏi đơn hàng: ${error.message}`;
    }
  };

  c.apply = async (raw) => {
    const code = String(raw || "").trim();
    c.input = code;
    c.notice = "";
    if (!code) { c.error = "Vui lòng nhập mã giảm giá."; onChange(); return; }
    c.busy = true; c.error = ""; onChange();
    try {
      const result = await validate(code);
      if (!isCurrent()) return;
      saveCode(result.code);
      c.applied = result; c.input = "";
      toast(`Đã áp dụng mã ${result.code}: giảm ${vnd(result.discount)}.`, "success");
    } catch (error) {
      if (!isCurrent()) return;
      c.error = error.message;
    }
    c.busy = false;
    onChange();
    if (c.error) requestAnimationFrame(() => document.getElementById("coupon-code")?.focus());
  };

  c.remove = () => { saveCode(""); c.applied = null; c.error = ""; c.notice = ""; onChange(); };

  /* Gắn vào phần tử cha bằng ủy quyền sự kiện (CSP không cho script/onclick nằm trong HTML) */
  c.attach = (root) => {
    root.addEventListener("click", (event) => {
      const pick = event.target.closest("[data-coupon-pick]");
      if (pick) { event.preventDefault(); c.apply(pick.dataset.couponPick); return; }
      if (event.target.closest("[data-coupon-apply]")) { event.preventDefault(); c.apply(root.querySelector("#coupon-code")?.value); return; }
      if (event.target.closest("[data-coupon-remove]")) { event.preventDefault(); c.remove(); }
    });
    root.addEventListener("input", (event) => { if (event.target.id === "coupon-code") { c.input = event.target.value; c.error = ""; } });
    root.addEventListener("keydown", (event) => {
      if (event.target.id === "coupon-code" && event.key === "Enter") { event.preventDefault(); c.apply(event.target.value); }   // không gửi form đặt hàng
    });
  };

  c.html = () => boxHtml(c, getTotal());
  return c;
}
