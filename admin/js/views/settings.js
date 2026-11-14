import { html, mount, toast, withLoading, formValues, showFormError } from "/assets/shared.js";
import { api, loadingBlock, errorBlock, digitsOnly } from "../core.js";

export async function settingsView({ main, isCurrent }) {
  mount(main, html`<div class="page-head"><div><h1>Cài đặt cửa hàng</h1><p>Thông tin hiển thị cho khách và ngưỡng cảnh báo tồn kho.</p></div></div>${loadingBlock}`);
  let s;
  try { s = await api("/admin/settings"); } catch (error) { if (isCurrent()) mount(main, html`<div class="page-head"><h1>Cài đặt cửa hàng</h1></div>${errorBlock(error.message)}`); return; }
  if (!isCurrent()) return;
  mount(main, html`
    <div class="page-head"><div><h1>Cài đặt cửa hàng</h1><p>Thông tin hiển thị cho khách và ngưỡng cảnh báo tồn kho.</p></div></div>
    <form class="panel-card" id="settings-form" novalidate style="max-width:820px">
      <div class="form-grid">
        <div class="field full"><label for="storeName">Tên cửa hàng</label><input class="input" id="storeName" name="storeName" required maxlength="100" value="${s.storeName}"></div>
        <div class="field"><label for="supportEmail">Email hỗ trợ</label><input class="input" id="supportEmail" name="supportEmail" type="email" required maxlength="160" value="${s.supportEmail}"></div>
        <div class="field"><label for="supportPhone">Hotline</label><input class="input" id="supportPhone" name="supportPhone" required maxlength="40" value="${s.supportPhone}"></div>
        <div class="field"><label for="lowStockThreshold">Ngưỡng cảnh báo sắp hết hàng</label><input class="input" id="lowStockThreshold" name="lowStockThreshold" inputmode="numeric" maxlength="5" required value="${s.lowStockThreshold}"><span class="hint">Sản phẩm có tồn kho từ 1 đến ngưỡng này được coi là sắp hết.</span></div>
        <div class="field"><label for="currency">Đơn vị tiền tệ</label><input class="input" id="currency" name="currency" required maxlength="3" value="${s.currency}"><span class="hint">Mã 3 ký tự, ví dụ VND.</span></div>
      </div>
      <div class="row" style="margin-top:1.4rem"><button class="btn btn-primary btn-lg" type="submit">Lưu cài đặt</button></div>
    </form>`);
  const form = main.querySelector("#settings-form");
  form.lowStockThreshold.addEventListener("input", (event) => { event.target.value = digitsOnly(event.target.value); });
  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    showFormError(form, "");
    const v = formValues(form);
    if (!v.storeName.trim() || !v.supportEmail.trim() || !v.supportPhone.trim() || v.lowStockThreshold === "" || v.currency.trim().length !== 3) { showFormError(form, "Hãy điền đủ các trường; đơn vị tiền tệ gồm đúng 3 ký tự."); return; }
    await withLoading(form.querySelector("button[type=submit]"), async () => {
      try {
        await api("/admin/settings", { method: "PUT", body: { storeName: v.storeName.trim(), supportEmail: v.supportEmail.trim(), supportPhone: v.supportPhone.trim(), lowStockThreshold: Number(v.lowStockThreshold), currency: v.currency.trim().toUpperCase() } });
        toast("Đã lưu cài đặt.");
      } catch (error) { showFormError(form, error.message); }
    });
  });
}
