import { html, icon, mount, safeSrc, vnd, number, dateTime, statusBadge, toast, openModal, confirmDialog, emptyState, debounce, PAYMENT_LABEL, STATUS_LABEL } from "/assets/shared.js";
import { api, fold, paginate, pager, loadingBlock, errorBlock } from "../core.js";

const PAGE = 10;
const STATUSES = ["pending", "confirmed", "shipping", "completed", "cancelled"];
// Quy trình hợp lệ phải khớp với máy trạng thái ở backend
const NEXT = { pending: ["confirmed", "cancelled"], confirmed: ["shipping", "cancelled"], shipping: ["completed"], completed: [], cancelled: [] };
const ACTION = { confirmed: ["Xác nhận đơn", "btn-primary", "check"], shipping: ["Bắt đầu giao hàng", "btn-primary", "truck"], completed: ["Hoàn thành đơn", "btn-primary", "check"], cancelled: ["Hủy đơn", "btn-danger", "x"] };

export async function ordersView({ main, query, isCurrent }) {
  const ui = { status: STATUSES.includes(query.status) ? query.status : "", q: "", page: 1 };
  let orders = [];
  const load = async () => { orders = await api("/admin/orders"); };
  const counts = () => Object.fromEntries(STATUSES.map((s) => [s, orders.filter((o) => o.status === s).length]));

  function paint() {
    const c = counts();
    mount(main.querySelector("#tabs"), html`<div class="tabs" role="tablist" aria-label="Lọc theo trạng thái">
      <button class="tab" role="tab" type="button" data-status="" aria-selected="${String(ui.status === "")}">Tất cả<span class="count">${orders.length}</span></button>
      ${STATUSES.map((s) => html`<button class="tab" role="tab" type="button" data-status="${s}" aria-selected="${String(ui.status === s)}">${STATUS_LABEL[s]}<span class="count">${c[s]}</span></button>`)}</div>`);
    const needle = fold(ui.q);
    const list = orders.filter((o) => (!ui.status || o.status === ui.status) && (!needle || fold(`${o.id} ${o.customerName} ${o.email} ${o.phone}`).includes(needle)));
    const info = paginate(list, ui.page, PAGE);
    ui.page = info.page;
    mount(main.querySelector("#list"), info.total === 0
      ? emptyState({ title: "Không có đơn hàng nào", text: "Thử đổi trạng thái hoặc từ khóa tìm kiếm.", iconName: "receipt" })
      : html`<div class="table-wrap"><table class="table"><thead><tr><th>Mã đơn</th><th>Khách hàng</th><th>Thời gian đặt</th><th class="num">Tổng tiền</th><th>Trạng thái</th><th style="text-align:right">Thao tác</th></tr></thead><tbody>
        ${info.items.map((o) => html`<tr><td class="id">${o.id}</td><td><b style="color:var(--ink);font-weight:600">${o.customerName}</b><br><small class="muted">${o.phone}</small></td><td>${dateTime(o.createdAt)}</td><td class="num">${vnd(o.total)}${o.discount > 0 ? html`<br><small class="muted">Mã ${o.couponCode}: −${vnd(o.discount)}</small>` : ""}</td><td>${statusBadge(o.status)}</td>
          <td><div class="row-actions"><button class="btn btn-secondary btn-sm" type="button" data-open="${o.id}">Chi tiết</button>
            ${NEXT[o.status].filter((s) => s !== "cancelled").map((s) => html`<button class="btn btn-soft btn-sm" type="button" data-quick="${o.id}" data-to="${s}">${ACTION[s][0]}</button>`)}</div></td></tr>`)}</tbody></table></div>
        <p class="muted" style="margin-top:.8rem">Hiển thị ${info.items.length} / ${info.total} đơn</p>${pager(info)}`);
  }

  async function changeStatus(order, to) {
    if (to === "cancelled" && !(await confirmDialog({ title: "Hủy đơn hàng", message: `Hủy đơn ${order.id}? Hàng sẽ được trả lại kho và không thể hoàn tác.`, confirmLabel: "Hủy đơn", danger: true }))) return false;
    try {
      await api(`/admin/orders/${encodeURIComponent(order.id)}/status`, { method: "PATCH", body: { status: to } });
      toast(`Đơn ${order.id}: ${STATUS_LABEL[to].toLowerCase()}.`);
      await load(); paint();
      document.dispatchEvent(new Event("orders-changed"));
      return true;
    } catch (error) { toast(error.message, "error"); return false; }
  }

  function detail(order) {
    const dialog = openModal({
      title: `Đơn ${order.id}`, size: "wide",
      body: html`<div class="stack">
        <div class="row spread">${statusBadge(order.status)}<span class="muted">Cập nhật ${dateTime(order.updatedAt)}</span></div>
        <div class="grid-2e" style="margin:0">
          <dl class="kv"><dt>Khách hàng</dt><dd>${order.customerName}</dd><dt>Email</dt><dd>${order.email || "—"}</dd><dt>Điện thoại</dt><dd>${order.phone}</dd><dt>Địa chỉ</dt><dd>${order.address}</dd></dl>
          <dl class="kv"><dt>Đặt lúc</dt><dd>${dateTime(order.createdAt)}</dd><dt>Thanh toán</dt><dd>${PAYMENT_LABEL[order.paymentMethod] || order.paymentMethod}</dd><dt>Ghi chú</dt><dd>${order.note || "—"}</dd></dl>
        </div>
        <div class="table-wrap"><table class="table"><thead><tr><th>Sản phẩm</th><th class="num">Đơn giá</th><th class="num">SL</th><th class="num">Thành tiền</th></tr></thead><tbody>
          ${order.items.map((i) => html`<tr><td><div class="cell-prod" style="min-width:0"><img class="thumb" src="${safeSrc(i.image)}" alt="" width="52" height="52"><b>${i.productName}</b></div></td><td class="num">${vnd(i.unitPrice)}</td><td class="num">${i.quantity}</td><td class="num">${vnd(i.unitPrice * i.quantity)}</td></tr>`)}</tbody></table></div>
        ${order.discount > 0 ? html`
          <div class="row spread"><span>Tạm tính</span><b>${vnd(order.subtotal)}</b></div>
          <div class="row spread"><span>Mã giảm giá <b class="code-chip">${order.couponCode}</b> (−${order.couponPercent}%)</span><b style="color:var(--success)">−${vnd(order.discount)}</b></div>` : ""}
        <div class="row spread"><b>Tổng khách thanh toán</b><span class="price" style="font-size:1.6rem;color:var(--brand-strong)">${vnd(order.total)}</span></div>
      </div>`,
      footer: NEXT[order.status].length
        ? html`<span class="muted" style="margin-right:auto;align-self:center">Chuyển trạng thái:</span>${NEXT[order.status].map((s) => html`<button class="btn ${ACTION[s][1]}" type="button" data-to="${s}">${icon(ACTION[s][2])}${ACTION[s][0]}</button>`)}`
        : html`<span class="muted" style="margin-right:auto;align-self:center">Đơn đã kết thúc, không thể đổi trạng thái.</span><button class="btn btn-secondary" type="button" data-modal-close>Đóng</button>`,
    });
    dialog.addEventListener("click", async (event) => {
      const button = event.target.closest("[data-to]");
      if (button && (await changeStatus(order, button.dataset.to))) dialog.close();
    });
  }

  mount(main, html`<div class="page-head"><div><h1>Đơn hàng</h1><p>Xử lý đơn theo quy trình: chờ xác nhận → xác nhận → giao hàng → hoàn thành.</p></div></div>
    <div id="tabs"></div>
    <div class="toolbar-row" style="margin-top:1rem"><div class="search-in">${icon("search")}<label class="sr-only" for="oq">Tìm đơn hàng</label><input class="input" id="oq" placeholder="Tìm theo mã đơn, tên, email, điện thoại" maxlength="100" autocomplete="off"></div></div>
    <div id="list">${loadingBlock}</div>`);
  try { await load(); } catch (error) { if (isCurrent()) mount(main.querySelector("#list"), errorBlock(error.message)); return; }
  if (!isCurrent()) return;
  paint();
  if (query.open) { const target = orders.find((o) => o.id === query.open); if (target) detail(target); }

  main.querySelector("#oq").addEventListener("input", debounce((event) => { ui.q = event.target.value.trim(); ui.page = 1; paint(); }, 200));
  main.addEventListener("click", async (event) => {
    const t = event.target;
    const tab = t.closest("[data-status]"); if (tab) { ui.status = tab.dataset.status; ui.page = 1; return paint(); }
    const page = t.closest("[data-page]"); if (page) { ui.page = Number(page.dataset.page); return paint(); }
    const open = t.closest("[data-open]"); if (open) return detail(orders.find((o) => o.id === open.dataset.open));
    const quick = t.closest("[data-quick]"); if (quick) return changeStatus(orders.find((o) => o.id === quick.dataset.quick), quick.dataset.to);
  });
}
