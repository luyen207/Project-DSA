import { html, icon, mount, vnd, dateOnly, dateTime, statusBadge, toast, openModal, confirmDialog, emptyState, debounce } from "/assets/shared.js";
import { api, fold, paginate, pager, loadingBlock, errorBlock } from "../core.js";

const PAGE = 10;

export async function customersView({ main, isCurrent }) {
  const ui = { q: "", page: 1 };
  let customers = [];
  const load = async () => { customers = await api("/admin/customers"); };

  function paint() {
    const needle = fold(ui.q);
    const list = customers.filter((c) => !needle || fold(`${c.name} ${c.email} ${c.phone}`).includes(needle));
    const info = paginate(list, ui.page, PAGE);
    ui.page = info.page;
    mount(main.querySelector("#list"), info.total === 0
      ? emptyState({ title: "Không có khách hàng phù hợp", iconName: "users" })
      : html`<div class="table-wrap"><table class="table"><thead><tr><th>Khách hàng</th><th>Ngày tham gia</th><th class="num">Đơn hàng</th><th class="num">Tổng chi tiêu</th><th>Trạng thái</th><th style="text-align:right">Thao tác</th></tr></thead><tbody>
        ${info.items.map((c) => html`<tr><td><b style="color:var(--ink);font-weight:600">${c.name}</b><br><small class="muted">${c.email} · ${c.phone || "chưa có SĐT"}</small></td><td>${dateOnly(c.joinedAt)}</td><td class="num">${c.orderCount}</td><td class="num">${vnd(c.totalSpent)}</td>
          <td><span class="badge ${c.status === "locked" ? "st-locked" : "st-active"}">${c.status === "locked" ? "Đã khóa" : "Hoạt động"}</span></td>
          <td><div class="row-actions"><button class="btn btn-secondary btn-sm" type="button" data-orders="${c.id}">Đơn hàng</button>
            <button class="btn ${c.status === "locked" ? "btn-soft" : "btn-ghost"} btn-sm" type="button" data-toggle="${c.id}" ${c.status === "locked" ? "" : html`style="color:var(--danger)"`}>${icon(c.status === "locked" ? "unlock" : "lock")}${c.status === "locked" ? "Mở khóa" : "Khóa"}</button></div></td></tr>`)}</tbody></table></div>
        <p class="muted" style="margin-top:.8rem">Hiển thị ${info.items.length} / ${info.total} khách hàng</p>${pager(info)}`);
  }

  mount(main, html`<div class="page-head"><div><h1>Khách hàng</h1><p>Khóa tài khoản sẽ đăng xuất khách ngay lập tức và chặn đăng nhập lại.</p></div></div>
    <div class="toolbar-row"><div class="search-in">${icon("search")}<label class="sr-only" for="cq">Tìm khách hàng</label><input class="input" id="cq" placeholder="Tìm theo tên, email, điện thoại" maxlength="100" autocomplete="off"></div></div>
    <div id="list">${loadingBlock}</div>`);
  try { await load(); } catch (error) { if (isCurrent()) mount(main.querySelector("#list"), errorBlock(error.message)); return; }
  if (!isCurrent()) return;
  paint();
  main.querySelector("#cq").addEventListener("input", debounce((event) => { ui.q = event.target.value.trim(); ui.page = 1; paint(); }, 200));

  main.addEventListener("click", async (event) => {
    const t = event.target;
    const page = t.closest("[data-page]"); if (page) { ui.page = Number(page.dataset.page); return paint(); }
    const orders = t.closest("[data-orders]");
    if (orders) {
      const customer = customers.find((c) => c.id === orders.dataset.orders);
      const dialog = openModal({ title: `Đơn hàng của ${customer.name}`, size: "wide", body: loadingBlock });
      try {
        const list = await api(`/admin/customers/${encodeURIComponent(customer.id)}/orders`);
        dialog.querySelector(".modal-body").innerHTML = (list.length
          ? html`<div class="table-wrap"><table class="table"><thead><tr><th>Mã đơn</th><th>Thời gian</th><th class="num">Tổng tiền</th><th>Trạng thái</th></tr></thead><tbody>${list.map((o) => html`<tr><td class="id"><a href="#/orders?open=${encodeURIComponent(o.id)}" data-modal-close>${o.id}</a></td><td>${dateTime(o.createdAt)}</td><td class="num">${vnd(o.total)}</td><td>${statusBadge(o.status)}</td></tr>`)}</tbody></table></div>`
          : emptyState({ title: "Khách chưa có đơn hàng", iconName: "receipt" })).toString();
      } catch (error) { dialog.querySelector(".modal-body").innerHTML = errorBlock(error.message).toString(); }
      return;
    }
    const toggle = t.closest("[data-toggle]");
    if (toggle) {
      const customer = customers.find((c) => c.id === toggle.dataset.toggle);
      const lock = customer.status !== "locked";
      if (lock && !(await confirmDialog({ title: "Khóa tài khoản", message: `Khóa tài khoản của ${customer.name}? Khách sẽ bị đăng xuất và không thể đăng nhập cho đến khi bạn mở khóa.`, confirmLabel: "Khóa tài khoản", danger: true }))) return;
      try { await api(`/admin/customers/${encodeURIComponent(customer.id)}/status`, { method: "PATCH", body: { status: lock ? "locked" : "active" } }); toast(lock ? "Đã khóa tài khoản." : "Đã mở khóa tài khoản."); await load(); paint(); }
      catch (error) { toast(error.message, "error"); }
    }
  });
}
