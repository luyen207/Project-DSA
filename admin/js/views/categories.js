import { html, icon, mount, toast, openModal, confirmDialog, withLoading, formValues, showFormError, emptyState } from "/assets/shared.js";
import { api, loadingBlock, errorBlock } from "../core.js";

export async function categoriesView({ main, isCurrent }) {
  let categories = [];
  const paint = () => mount(main.querySelector("#list"), categories.length === 0
    ? emptyState({ title: "Chưa có danh mục", iconName: "tag", action: html`<button class="btn btn-primary" type="button" data-new>Thêm danh mục</button>` })
    : html`<div class="table-wrap"><table class="table"><thead><tr><th>Mã</th><th>Tên danh mục</th><th>Mô tả</th><th class="num">Sản phẩm</th><th style="text-align:right">Thao tác</th></tr></thead><tbody>
      ${categories.map((c) => html`<tr><td class="id">${c.id}</td><td><b style="color:var(--ink)">${c.name}</b></td><td class="muted">${c.description || "—"}</td><td class="num">${c.productCount}</td>
        <td><div class="row-actions"><button class="btn btn-secondary btn-sm" type="button" data-edit="${c.id}">${icon("edit")}Sửa</button><button class="btn btn-ghost btn-sm" type="button" data-delete="${c.id}" aria-label="Xóa ${c.name}" style="color:var(--danger)">${icon("trash")}</button></div></td></tr>`)}</tbody></table></div>`);
  async function load() { categories = await api("/admin/categories"); paint(); }

  mount(main, html`<div class="page-head"><div><h1>Danh mục</h1><p>Nhóm sản phẩm hiển thị ở cửa hàng. Chỉ xóa được danh mục không còn sản phẩm.</p></div><button class="btn btn-primary" type="button" data-new>${icon("plus")}Thêm danh mục</button></div><div id="list">${loadingBlock}</div>`);
  try { await load(); } catch (error) { if (isCurrent()) mount(main.querySelector("#list"), errorBlock(error.message)); return; }

  function form(category) {
    const isNew = !category;
    const c = category || { name: "", description: "" };
    const dialog = openModal({
      title: isNew ? "Thêm danh mục" : "Sửa danh mục", size: "narrow",
      body: html`<form id="cat-form" novalidate class="stack">
        <div class="field"><label for="c-name">Tên danh mục</label><input class="input" id="c-name" name="name" required minlength="2" maxlength="60" value="${c.name}"></div>
        <div class="field"><label for="c-desc">Mô tả ngắn</label><input class="input" id="c-desc" name="description" maxlength="200" value="${c.description}"></div></form>`,
      footer: html`<button class="btn btn-secondary" type="button" data-modal-close>Hủy bỏ</button><button class="btn btn-primary" type="submit" form="cat-form" data-save>${isNew ? "Thêm danh mục" : "Lưu thay đổi"}</button>`,
    });
    const f = dialog.querySelector("#cat-form");
    f.addEventListener("submit", async (event) => {
      event.preventDefault();
      showFormError(f, "");
      const v = formValues(f);
      if (v.name.trim().length < 2) { showFormError(f, "Tên danh mục cần có ít nhất 2 ký tự."); return; }
      await withLoading(dialog.querySelector("[data-save]"), async () => {
        try { await api(isNew ? "/admin/categories" : `/admin/categories/${c.id}`, { method: isNew ? "POST" : "PUT", body: { name: v.name.trim(), description: v.description.trim() } }); dialog.close(); toast(isNew ? "Đã thêm danh mục." : "Đã lưu danh mục."); await load(); }
        catch (error) { showFormError(f, error.message); }
      });
    });
    f.elements["name"].focus();
  }
  main.addEventListener("click", async (event) => {
    if (event.target.closest("[data-new]")) return form(null);
    const edit = event.target.closest("[data-edit]");
    if (edit) return form(categories.find((c) => String(c.id) === edit.dataset.edit));
    const del = event.target.closest("[data-delete]");
    if (!del) return;
    const category = categories.find((c) => String(c.id) === del.dataset.delete);
    if (!(await confirmDialog({ title: "Xóa danh mục", message: `Xóa danh mục “${category.name}”?`, confirmLabel: "Xóa danh mục", danger: true }))) return;
    try { await api(`/admin/categories/${category.id}`, { method: "DELETE" }); toast("Đã xóa danh mục."); await load(); }
    catch (error) { toast(error.message, "error"); }
  });
}
