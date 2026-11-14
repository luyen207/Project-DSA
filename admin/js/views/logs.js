import { html, icon, mount, dateTime, emptyState, queryString } from "/assets/shared.js";
import { api, pager, loadingBlock, errorBlock } from "../core.js";

// Khớp với các khóa hành động mà backend ghi
export const ACTIONS = {
  "auth.login.succeeded": "Đăng nhập thành công", "auth.login.failed": "Đăng nhập thất bại", "auth.login.blocked": "Đăng nhập bị chặn",
  "auth.logout": "Đăng xuất", "auth.registered": "Đăng ký tài khoản", "auth.register.blocked": "Đăng ký bị chặn",
  "customer.profile.updated": "Khách cập nhật hồ sơ",
  "cart.item_added": "Thêm vào giỏ", "cart.item_updated": "Sửa số lượng trong giỏ", "cart.item_removed": "Xóa khỏi giỏ", "cart.cleared": "Xóa toàn bộ giỏ", "cart.sorted": "Sắp xếp giỏ hàng",
  "order.placed": "Đặt hàng", "order.cancelled": "Khách hủy đơn",
  "admin.order.status_changed": "Đổi trạng thái đơn",
  "admin.product.created": "Thêm sản phẩm", "admin.product.updated": "Sửa sản phẩm", "admin.product.deleted": "Xóa sản phẩm",
  "admin.product.stock_updated": "Cập nhật tồn kho", "admin.product.visibility_updated": "Ẩn/hiện sản phẩm",
  "admin.category.created": "Thêm danh mục", "admin.category.updated": "Sửa danh mục", "admin.category.deleted": "Xóa danh mục",
  "admin.customer.status_changed": "Khóa/mở khóa khách", "admin.settings.updated": "Cập nhật cài đặt",
};
const RESULT = { success: ["Thành công", "st-success"], failure: ["Thất bại", "st-failure"], blocked: ["Bị chặn", "st-blocked"] };
const ROLE = { admin: "Quản trị", customer: "Khách", anonymous: "Ẩn danh", system: "Hệ thống" };
const PAGE = 20;

function detailText(details) {
  if (!details || typeof details !== "object") return "";
  return Object.entries(details).map(([key, value]) => `${key}: ${typeof value === "object" ? JSON.stringify(value) : value}`).join(" · ");
}

export async function logsView({ main, isCurrent }) {
  const ui = { action: "", result: "", from: "", to: "", page: 1 };
  mount(main, html`
    <div class="page-head"><div><h1>Nhật ký hoạt động</h1><p>Mọi thao tác của khách và quản trị viên, mới nhất ở trên.</p></div><button class="btn btn-secondary" type="button" data-refresh>${icon("refresh")}Tải lại</button></div>
    <form class="toolbar-row" id="log-filters" novalidate>
      <label class="sr-only" for="la">Hành động</label><select class="select" id="la" name="action"><option value="">Mọi hành động</option>${Object.entries(ACTIONS).map(([key, label]) => html`<option value="${key}">${label}</option>`)}</select>
      <label class="sr-only" for="lr">Kết quả</label><select class="select" id="lr" name="result"><option value="">Mọi kết quả</option>${Object.entries(RESULT).map(([key, [label]]) => html`<option value="${key}">${label}</option>`)}</select>
      <label class="sr-only" for="lf">Từ ngày</label><input class="input w" id="lf" name="from" type="date" title="Từ ngày">
      <label class="sr-only" for="lt">Đến ngày</label><input class="input w" id="lt" name="to" type="date" title="Đến ngày">
      <button class="btn btn-ghost" type="button" data-reset>Xóa lọc</button>
    </form>
    <div id="list">${loadingBlock}</div>`);
  const form = main.querySelector("#log-filters");

  async function load() {
    try {
      const params = { page: ui.page, limit: PAGE, action: ui.action, result: ui.result, from: ui.from, to: ui.to };
      const data = await api(`/admin/activity-logs${queryString(params)}`);
      if (!isCurrent()) return;
      const totalPages = Math.max(1, Math.ceil(data.total / data.limit));
      mount(main.querySelector("#list"), data.items.length === 0
        ? emptyState({ title: "Không có bản ghi phù hợp", text: "Thử bỏ bớt bộ lọc.", iconName: "log" })
        : html`<div class="table-wrap"><table class="table"><thead><tr><th>Thời gian</th><th>Người thực hiện</th><th>Hành động</th><th>Đối tượng</th><th>Kết quả</th><th>Chi tiết</th></tr></thead><tbody>
          ${data.items.map((entry) => { const [label, cls] = RESULT[entry.result] || [entry.result, "st-hidden"]; return html`<tr>
            <td style="white-space:nowrap">${dateTime(entry.createdAt)}</td>
            <td><b style="color:var(--ink);font-weight:600">${ROLE[entry.actorRole] || entry.actorRole}</b><br><small class="muted">${entry.actorId || "—"}${entry.ip ? ` · ${entry.ip}` : ""}</small></td>
            <td><b style="color:var(--ink);font-weight:600">${ACTIONS[entry.action] || entry.action}</b><br><small class="muted detail-json">${entry.action}</small></td>
            <td>${entry.targetType ? html`${entry.targetType}<br><small class="muted">${entry.targetId}</small>` : "—"}</td>
            <td><span class="badge ${cls}">${label}</span></td>
            <td class="detail-json" style="min-width:200px">${detailText(entry.details) || "—"}</td></tr>`; })}</tbody></table></div>
          <p class="muted" style="margin-top:.8rem">${data.total} bản ghi · trang ${data.page}/${totalPages}</p>${pager({ page: data.page, totalPages })}`);
    } catch (error) { if (isCurrent()) mount(main.querySelector("#list"), errorBlock(error.message)); }
  }

  form.addEventListener("change", () => { Object.assign(ui, { action: form.elements["action"].value, result: form.elements["result"].value, from: form.elements["from"].value, to: form.elements["to"].value, page: 1 }); load(); });
  main.addEventListener("click", (event) => {
    const page = event.target.closest("[data-page]");
    if (page) { ui.page = Number(page.dataset.page); load(); main.scrollIntoView({ behavior: "smooth" }); return; }
    if (event.target.closest("[data-reset]")) { form.reset(); Object.assign(ui, { action: "", result: "", from: "", to: "", page: 1 }); load(); return; }
    if (event.target.closest("[data-refresh]")) load();
  });
  await load();
}
