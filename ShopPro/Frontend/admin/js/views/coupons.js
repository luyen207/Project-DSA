// Quản lý mã giảm giá (theo %): tạo, sửa, bật/tắt, xóa. Mọi luật kiểm tra nằm ở máy chủ; trang này chỉ nhập liệu và hiển thị.
import { html, icon, mount, vnd, toast, openModal, confirmDialog, withLoading, formValues, showFormError, emptyState, dateTime } from "/assets/shared.js";
import { api, loadingBlock, errorBlock, digitsOnly } from "../core.js";

// Trạng thái hiển thị: đang bật / đã tắt / hết lượt
function statusOf(c) {
  if (!c.active) return ["Đã tắt", "st-hidden"];
  if (c.usageLimit > 0 && c.usedCount >= c.usageLimit) return ["Hết lượt", "st-pending"];
  return ["Đang bật", "st-active"];
}

export async function couponsView({ main, isCurrent }) {
  let coupons = [];

  const paint = () => mount(main.querySelector("#list"), coupons.length === 0
    ? emptyState({ title: "Chưa có mã giảm giá", text: "Tạo mã đầu tiên để khách nhập ở giỏ hàng.", iconName: "tag", action: html`<button class="btn btn-primary" type="button" data-new>Tạo mã giảm giá</button>` })
    : html`<div class="table-wrap"><table class="table coupon-table"><thead><tr>
        <th>Mã giảm giá</th><th class="num">Mức giảm</th><th class="num">Lượt dùng</th><th class="num">Đã giảm</th><th style="text-align:right">Thao tác</th></tr></thead><tbody>
      ${coupons.map((c) => { const [label, cls] = statusOf(c); return html`<tr>
        <td class="coupon-main"><div class="code-line"><b class="code-chip">${c.code}</b><span class="badge ${cls}">${label}</span></div><small class="muted">${c.description || "Không có mô tả"}</small></td>
        <td class="num"><b style="color:var(--ink);font-size:1.1rem">${c.percent}%</b><small class="muted">${c.minOrder ? `Đơn từ ${vnd(c.minOrder)}` : "Mọi đơn hàng"}</small></td>
        <td class="num"><b style="color:var(--ink)">${c.usedCount}</b>${c.usageLimit > 0 ? ` / ${c.usageLimit}` : " / ∞"}<small class="muted">${c.oncePerCustomer ? "Mỗi khách 1 lần" : "Dùng nhiều lần"}</small></td>
        <td class="num">${vnd(c.discountGiven)}</td>
        <td><div class="row-actions">
          <button class="btn btn-secondary btn-sm" type="button" data-toggle="${c.code}" title="${c.active ? "Tạm ngưng mã này" : "Cho khách dùng lại mã này"}">${icon(c.active ? "lock" : "unlock")}${c.active ? "Tắt" : "Bật"}</button>
          <button class="btn btn-secondary btn-sm" type="button" data-edit="${c.code}">${icon("edit")}Sửa</button>
          <button class="btn btn-ghost btn-sm" type="button" data-delete="${c.code}" aria-label="Xóa mã ${c.code}" style="color:var(--danger)">${icon("trash")}</button></div></td></tr>`; })}</tbody></table></div>
      <p class="muted" style="margin-top:.8rem">Lượt dùng chỉ tính các đơn chưa hủy. Mã đã có đơn hàng không xóa được, hãy tắt mã thay vì xóa.</p>`);

  async function load() { coupons = await api("/admin/coupons"); if (isCurrent()) paint(); }

  mount(main, html`<div class="page-head"><div><h1>Mã giảm giá</h1><p>Khách nhập mã ở giỏ hàng để được giảm theo phần trăm. Hệ thống tự kiểm tra đơn tối thiểu, số lượt và mỗi khách chỉ dùng 1 lần.</p></div><button class="btn btn-primary" type="button" data-new>${icon("plus")}Tạo mã giảm giá</button></div><div id="list">${loadingBlock}</div>`);
  try { await load(); } catch (error) { if (isCurrent()) mount(main.querySelector("#list"), errorBlock(error.message)); return; }

  function form(coupon) {
    const isNew = !coupon;
    const c = coupon || { code: "", description: "", percent: "", minOrder: 0, usageLimit: 0, oncePerCustomer: true, active: true };
    const dialog = openModal({
      title: isNew ? "Tạo mã giảm giá" : `Sửa mã ${c.code}`, size: "wide",
      body: html`<form id="coupon-form" novalidate class="form-grid">
        <div class="field"><label for="m-code">Mã giảm giá</label><input class="input" id="m-code" name="code" required maxlength="20" autocomplete="off" spellcheck="false" style="text-transform:uppercase" value="${c.code}" ${isNew ? "" : html`disabled`}>
          <span class="hint">${isNew ? "3–20 ký tự: chữ không dấu, số, gạch ngang hoặc gạch dưới. Ví dụ: TET2026." : "Không đổi được mã sau khi tạo."}</span></div>
        <div class="field"><label for="m-percent">Phần trăm giảm (%)</label><input class="input" id="m-percent" name="percent" required inputmode="numeric" maxlength="2" value="${c.percent}"><span class="hint">Từ 1 đến 90.</span></div>
        <div class="field full"><label for="m-desc">Mô tả ngắn (không bắt buộc)</label><input class="input" id="m-desc" name="description" maxlength="200" value="${c.description}" placeholder="Ví dụ: Giảm 10% cho đơn từ 300.000đ"></div>
        <div class="field"><label for="m-min">Đơn hàng tối thiểu (₫)</label><input class="input" id="m-min" name="minOrder" inputmode="numeric" maxlength="11" value="${c.minOrder}"><span class="hint">Nhập 0 nếu áp dụng cho mọi đơn.</span></div>
        <div class="field"><label for="m-limit">Số lượt dùng tối đa</label><input class="input" id="m-limit" name="usageLimit" inputmode="numeric" maxlength="7" value="${c.usageLimit}"><span class="hint">Nhập 0 nếu không giới hạn.</span></div>
        <label class="check-row full"><input type="checkbox" name="oncePerCustomer" ${c.oncePerCustomer ? html`checked` : ""}><span><b>Mỗi khách chỉ dùng 1 lần</b><small>Khách đã dùng mã (và đơn chưa hủy) sẽ không dùng lại được.</small></span></label>
        <label class="check-row full"><input type="checkbox" name="active" ${c.active ? html`checked` : ""}><span><b>Đang bật</b><small>Tắt để tạm ngưng mã mà vẫn giữ lại lịch sử.</small></span></label>
      </form>`,
      footer: html`<button class="btn btn-secondary" type="button" data-modal-close>Hủy bỏ</button><button class="btn btn-primary" type="submit" form="coupon-form" data-save>${isNew ? "Tạo mã" : "Lưu thay đổi"}</button>`,
    });
    const f = dialog.querySelector("#coupon-form");
    f.addEventListener("input", (event) => {
      if (["percent", "minOrder", "usageLimit"].includes(event.target.name)) event.target.value = digitsOnly(event.target.value);
      if (event.target.name === "code") event.target.value = event.target.value.toUpperCase().replace(/[^A-Z0-9_-]/g, "");
    });
    f.addEventListener("submit", async (event) => {
      event.preventDefault();
      showFormError(f, "");
      const v = formValues(f);
      const code = (v.code || c.code || "").trim().toUpperCase();
      const percent = Number(digitsOnly(v.percent));
      const problems = [];
      if (isNew && !/^[A-Z0-9_-]{3,20}$/.test(code)) problems.push("Mã cần 3–20 ký tự: chữ không dấu, số, gạch ngang hoặc gạch dưới.");
      if (!(percent >= 1 && percent <= 90)) problems.push("Phần trăm giảm phải từ 1 đến 90.");
      if (problems.length) { showFormError(f, problems.join(" ")); return; }
      const body = {
        description: (v.description || "").trim(), percent,
        minOrder: Number(digitsOnly(v.minOrder)) || 0, usageLimit: Number(digitsOnly(v.usageLimit)) || 0,
        oncePerCustomer: f.elements["oncePerCustomer"].checked, active: f.elements["active"].checked,
      };
      await withLoading(dialog.querySelector("[data-save]"), async () => {
        try {
          if (isNew) await api("/admin/coupons", { method: "POST", body: { code, ...body } });
          else await api(`/admin/coupons/${encodeURIComponent(c.code)}`, { method: "PUT", body });
          dialog.close();
          toast(isNew ? `Đã tạo mã ${code}.` : `Đã lưu mã ${c.code}.`);
          await load();
        } catch (error) { showFormError(f, error.message); }
      });
    });
    (isNew ? f.elements["code"] : f.elements["percent"]).focus();
  }

  main.addEventListener("click", async (event) => {
    if (event.target.closest("[data-new]")) return form(null);
    const edit = event.target.closest("[data-edit]");
    if (edit) return form(coupons.find((c) => c.code === edit.dataset.edit));
    const toggle = event.target.closest("[data-toggle]");
    if (toggle) {
      const coupon = coupons.find((c) => c.code === toggle.dataset.toggle);
      try {
        await api(`/admin/coupons/${encodeURIComponent(coupon.code)}/status`, { method: "PATCH", body: { active: !coupon.active } });
        toast(coupon.active ? `Đã tắt mã ${coupon.code}.` : `Đã bật mã ${coupon.code}.`);
        await load();
      } catch (error) { toast(error.message, "error"); }
      return;
    }
    const del = event.target.closest("[data-delete]");
    if (!del) return;
    const coupon = coupons.find((c) => c.code === del.dataset.delete);
    if (!(await confirmDialog({ title: "Xóa mã giảm giá", message: `Xóa mã ${coupon.code}? Khách sẽ không dùng được mã này nữa.`, confirmLabel: "Xóa mã", danger: true }))) return;
    try { await api(`/admin/coupons/${encodeURIComponent(coupon.code)}`, { method: "DELETE" }); toast("Đã xóa mã giảm giá."); await load(); }
    catch (error) { toast(error.message, "error"); }
  });
}
