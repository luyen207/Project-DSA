import { html, icon, mount, safeSrc, vnd, number, toast, openModal, confirmDialog, withLoading, formValues, showFormError, emptyState, debounce } from "/assets/shared.js";
import { api, fold, paginate, pager, loadingBlock, errorBlock, digitsOnly } from "../core.js";

const PAGE = 10;
const STOCK_FILTERS = [["", "Mọi tồn kho"], ["low", "Sắp hết hàng"], ["out", "Hết hàng"], ["hidden", "Đang ẩn"]];

export async function productsView({ main, query, isCurrent }) {
  const ui = { q: query.q || "", category: query.category || "", stock: query.stock || "", page: 1 };
  let products = [], categories = [], threshold = 10;

  async function load() {
    const [p, c, s] = await Promise.all([api("/admin/products"), api("/admin/categories"), api("/admin/settings")]);
    products = p; categories = c; threshold = s.lowStockThreshold ?? 10;
  }
  const matches = () => products.filter((p) => {
    if (ui.q && !fold(`${p.name} ${p.id}`).includes(fold(ui.q))) return false;
    if (ui.category && String(p.categoryId) !== ui.category) return false;
    if (ui.stock === "low" && !(p.stock > 0 && p.stock <= threshold)) return false;
    if (ui.stock === "out" && p.stock !== 0) return false;
    if (ui.stock === "hidden" && p.visible !== false) return false;
    return true;
  });

  function paint() {
    const info = paginate(matches(), ui.page, PAGE);
    ui.page = info.page;
    mount(main.querySelector("#list"), info.total === 0
      ? emptyState({ title: "Không có sản phẩm nào", text: "Thử đổi từ khóa hoặc bộ lọc.", iconName: "package" })
      : html`<div class="table-wrap"><table class="table"><thead><tr><th>Sản phẩm</th><th class="num">Giá</th><th class="num">Tồn kho</th><th>Hiển thị</th><th class="actions" style="text-align:right">Thao tác</th></tr></thead><tbody>
        ${info.items.map((p) => html`<tr>
          <td><div class="cell-prod"><img class="thumb" src="${safeSrc(p.image)}" alt="" width="52" height="52" loading="lazy"><div><b>${p.name}</b><small>Mã ${p.id} · ${p.categoryName}${p.featured ? " · Nổi bật" : ""}${p.bestSelling ? " · Bán chạy" : ""}</small></div></div></td>
          <td class="num">${vnd(p.price)}</td>
          <td class="num"><span class="stock-pill ${p.stock === 0 ? "out" : p.stock <= threshold ? "low" : ""}">${p.stock === 0 ? icon("x") : p.stock <= threshold ? icon("alert") : ""}${number(p.stock)}</span></td>
          <td>${p.visible === false ? html`<span class="badge st-hidden">Đang ẩn</span>` : html`<span class="badge st-active">Đang bán</span>`}</td>
          <td><div class="row-actions">
            <button class="btn btn-soft btn-sm" type="button" data-stock="${p.id}">${icon("warehouse")}Kho</button>
            <button class="btn btn-secondary btn-sm" type="button" data-edit="${p.id}">${icon("edit")}Sửa</button>
            <button class="btn btn-ghost btn-sm" type="button" data-visible="${p.id}">${icon(p.visible === false ? "eye" : "eyeoff")}${p.visible === false ? "Hiện" : "Ẩn"}</button>
            <button class="btn btn-ghost btn-sm" type="button" data-delete="${p.id}" aria-label="Xóa ${p.name}" style="color:var(--danger)">${icon("trash")}</button>
          </div></td></tr>`)}</tbody></table></div>
        <p class="muted" style="margin-top:.8rem">Hiển thị ${info.items.length} / ${info.total} sản phẩm</p>${pager(info)}`);
  }

  mount(main, html`
    <div class="page-head"><div><h1>Sản phẩm</h1><p>Thêm, sửa, nhập kho và ẩn hiện sản phẩm.</p></div><button class="btn btn-primary" type="button" data-new>${icon("plus")}Thêm sản phẩm</button></div>
    <div class="toolbar-row">
      <div class="search-in">${icon("search")}<label class="sr-only" for="pq">Tìm sản phẩm</label><input class="input" id="pq" placeholder="Tìm theo tên hoặc mã sản phẩm" value="${ui.q}" maxlength="100" autocomplete="off"></div>
      <label class="sr-only" for="pc">Danh mục</label><select class="select" id="pc"><option value="">Mọi danh mục</option></select>
      <label class="sr-only" for="ps">Tồn kho</label><select class="select" id="ps">${STOCK_FILTERS.map(([value, label]) => html`<option value="${value}" ${value === ui.stock ? "selected" : ""}>${label}</option>`)}</select>
    </div>
    <div id="list">${loadingBlock}</div>`);
  try { await load(); } catch (error) { if (isCurrent()) mount(main.querySelector("#list"), errorBlock(error.message)); return; }
  if (!isCurrent()) return;
  mount(main.querySelector("#pc"), html`<option value="">Mọi danh mục</option>${categories.map((c) => html`<option value="${c.id}" ${String(c.id) === ui.category ? "selected" : ""}>${c.name}</option>`)}`);
  paint();

  main.querySelector("#pq").addEventListener("input", debounce((event) => { ui.q = event.target.value.trim(); ui.page = 1; paint(); }, 200));
  main.querySelector("#pc").addEventListener("change", (event) => { ui.category = event.target.value; ui.page = 1; paint(); });
  main.querySelector("#ps").addEventListener("change", (event) => { ui.stock = event.target.value; ui.page = 1; paint(); });

  const find = (id) => products.find((p) => String(p.id) === String(id));
  const refresh = async () => { await load(); paint(); };

  function productForm(product) {
    const isNew = !product;
    const p = product || { name: "", categoryId: categories[0]?.id, price: "", stock: "", description: "", image: "/img/products/placeholder.svg", visible: true, featured: false, bestSelling: false };
    const dialog = openModal({
      title: isNew ? "Thêm sản phẩm" : `Sửa sản phẩm #${p.id}`, size: "wide",
      body: html`<form id="product-form" novalidate class="form-grid">
        <div class="field full"><label for="f-name">Tên sản phẩm</label><input class="input" id="f-name" name="name" required minlength="2" maxlength="120" value="${p.name}"></div>
        <div class="field"><label for="f-cat">Danh mục</label><select class="select" id="f-cat" name="categoryId">${categories.map((c) => html`<option value="${c.id}" ${c.id === p.categoryId ? "selected" : ""}>${c.name}</option>`)}</select></div>
        <div class="field"><label for="f-price">Giá bán (₫)</label><input class="input" id="f-price" name="price" required inputmode="numeric" maxlength="11" value="${p.price}"><span class="hint">Từ 0 đến 10.000.000.000.</span></div>
        <div class="field"><label for="f-stock">Tồn kho</label><input class="input" id="f-stock" name="stock" required inputmode="numeric" maxlength="6" value="${p.stock}"><span class="hint">Từ 0 đến 100.000.</span></div>
        <div class="field"><label for="f-image">Đường dẫn ảnh</label><input class="input" id="f-image" name="image" maxlength="500" value="${p.image}" placeholder="/img/products/101.svg hoặc https://…"></div>
        <div class="field full"><div class="preview"><img id="f-preview" src="${safeSrc(p.image)}" alt="Xem trước ảnh sản phẩm" width="84" height="84"><span class="hint">Ảnh nội bộ bắt đầu bằng / hoặc địa chỉ http(s). Để trống sẽ dùng ảnh mặc định.</span></div></div>
        <div class="field full"><label for="f-desc">Mô tả</label><textarea class="textarea" id="f-desc" name="description" maxlength="2000">${p.description}</textarea></div>
        <div class="field full checks">
          <label class="check"><input type="checkbox" name="visible" ${p.visible !== false ? "checked" : ""}>Đang bán (hiển thị với khách)</label>
          <label class="check"><input type="checkbox" name="featured" ${p.featured ? "checked" : ""}>Nổi bật</label>
          <label class="check"><input type="checkbox" name="bestSelling" ${p.bestSelling ? "checked" : ""}>Bán chạy</label>
        </div></form>`,
      footer: html`<button class="btn btn-secondary" type="button" data-modal-close>Hủy bỏ</button><button class="btn btn-primary" type="submit" form="product-form" data-save>${isNew ? "Thêm sản phẩm" : "Lưu thay đổi"}</button>`,
    });
    const form = dialog.querySelector("#product-form");
    const image = form.querySelector("#f-image");
    image.addEventListener("input", () => { dialog.querySelector("#f-preview").src = safeSrc(image.value.trim() || "/img/products/placeholder.svg"); });
    form.addEventListener("input", (event) => { if (event.target.name === "price" || event.target.name === "stock") event.target.value = digitsOnly(event.target.value); });
    form.addEventListener("submit", async (event) => {
      event.preventDefault();
      showFormError(form, "");
      const v = formValues(form);
      const body = {
        name: v.name.trim(), categoryId: Number(v.categoryId), price: Number(digitsOnly(v.price)), stock: Number(digitsOnly(v.stock)),
        description: v.description.trim(), image: v.image.trim() || "/img/products/placeholder.svg",
        visible: form.visible.checked, featured: form.featured.checked, bestSelling: form.bestSelling.checked,
      };
      if (body.name.length < 2) { showFormError(form, "Tên sản phẩm cần có ít nhất 2 ký tự."); return; }
      if (v.price === "" || v.stock === "") { showFormError(form, "Hãy nhập giá bán và tồn kho."); return; }
      await withLoading(dialog.querySelector("[data-save]"), async () => {
        try {
          await api(isNew ? "/admin/products" : `/admin/products/${p.id}`, { method: isNew ? "POST" : "PUT", body });
          dialog.close();
          toast(isNew ? "Đã thêm sản phẩm." : "Đã lưu thay đổi.");
          await refresh();
        } catch (error) { showFormError(form, error.message); }
      });
    });
    form.elements["name"].focus();
  }

  function stockForm(product) {
    const dialog = openModal({
      title: "Cập nhật tồn kho", size: "narrow",
      body: html`<form id="stock-form" novalidate class="stack"><p><b>${product.name}</b><br><span class="muted">Tồn kho hiện tại: ${number(product.stock)}</span></p>
        <div class="field"><label for="f-stock2">Số lượng tồn kho mới</label><input class="input" id="f-stock2" name="stock" inputmode="numeric" maxlength="6" value="${product.stock}" required></div></form>`,
      footer: html`<button class="btn btn-secondary" type="button" data-modal-close>Hủy bỏ</button><button class="btn btn-primary" type="submit" form="stock-form" data-save>Cập nhật</button>`,
    });
    const form = dialog.querySelector("#stock-form");
    form.addEventListener("input", (event) => { event.target.value = digitsOnly(event.target.value); });
    form.addEventListener("submit", async (event) => {
      event.preventDefault();
      showFormError(form, "");
      if (form.stock.value === "") { showFormError(form, "Hãy nhập số lượng."); return; }
      await withLoading(dialog.querySelector("[data-save]"), async () => {
        try { await api(`/admin/products/${product.id}/stock`, { method: "PATCH", body: { stock: Number(form.stock.value) } }); dialog.close(); toast("Đã cập nhật tồn kho."); await refresh(); }
        catch (error) { showFormError(form, error.message); }
      });
    });
    form.stock.select();
  }

  main.addEventListener("click", async (event) => {
    const t = event.target;
    const pageButton = t.closest("[data-page]");
    if (pageButton) { ui.page = Number(pageButton.dataset.page); paint(); main.querySelector("#list").scrollIntoView({ behavior: "smooth", block: "start" }); return; }
    if (t.closest("[data-new]")) return productForm(null);
    const edit = t.closest("[data-edit]"); if (edit) return productForm(find(edit.dataset.edit));
    const stock = t.closest("[data-stock]"); if (stock) return stockForm(find(stock.dataset.stock));
    const visible = t.closest("[data-visible]");
    if (visible) {
      const product = find(visible.dataset.visible);
      const next = product.visible === false;
      try { await api(`/admin/products/${product.id}/visibility`, { method: "PATCH", body: { visible: next } }); toast(next ? "Đã hiện sản phẩm." : "Đã ẩn sản phẩm khỏi cửa hàng."); await refresh(); }
      catch (error) { toast(error.message, "error"); }
      return;
    }
    const del = t.closest("[data-delete]");
    if (del) {
      const product = find(del.dataset.delete);
      if (!(await confirmDialog({ title: "Xóa sản phẩm", message: `Xóa vĩnh viễn “${product.name}”? Sản phẩm sẽ bị gỡ khỏi giỏ hàng của khách. Đơn hàng cũ vẫn giữ nguyên thông tin đã mua.`, confirmLabel: "Xóa sản phẩm", danger: true }))) return;
      try { await api(`/admin/products/${product.id}`, { method: "DELETE" }); toast("Đã xóa sản phẩm."); await refresh(); }
      catch (error) { toast(error.message, "error"); }
    }
  });
}
