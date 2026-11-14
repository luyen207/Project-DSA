import { html, icon, mount, safeSrc, vnd, dateTime, statusBadge, emptyState, toast, confirmDialog, PAYMENT_LABEL, STATUS_LABEL } from "/assets/shared.js";
import { api, requireLogin } from "../state.js";

const STEPS = ["pending", "confirmed", "shipping", "completed"];

function timeline(status) {
  if (status === "cancelled") return html`<ol class="timeline cancelled" aria-label="Đơn đã hủy">${STEPS.map((step) => html`<li>${STATUS_LABEL[step]}</li>`)}</ol>`;
  const at = STEPS.indexOf(status);
  return html`<ol class="timeline" aria-label="Tiến trình đơn hàng">${STEPS.map((step, i) => html`<li class="${i < at || status === "completed" ? "done" : ""} ${i === at && status !== "completed" ? "now" : ""}" ${i === at ? html`aria-current="step"` : ""}>${STATUS_LABEL[step]}</li>`)}</ol>`;
}
const thumbs = (order) => html`<div class="thumbs">${order.items.slice(0, 4).map((item) => html`<img src="${safeSrc(item.product.image)}" alt="${item.product.name}" width="56" height="56">`)}${order.items.length > 4 ? html`<span class="more">+${order.items.length - 4}</span>` : ""}</div>`;

export async function ordersView({ main, setTitle, isCurrent }) {
  setTitle("Đơn hàng của tôi");
  if (!requireLogin()) return;
  mount(main, html`<div class="wrap"><div class="page-title"><h1>Đơn hàng của tôi</h1><p>Theo dõi trạng thái và hủy đơn đang chờ xác nhận.</p></div><div id="orders"><div class="loading-block"><div class="spinner" role="status" aria-label="Đang tải"></div></div></div></div>`);
  const host = main.querySelector("#orders");

  async function load() {
    try {
      const orders = await api("/orders");
      if (!isCurrent()) return;
      mount(host, orders.length === 0
        ? emptyState({ title: "Bạn chưa có đơn hàng nào", text: "Khi đặt hàng, đơn sẽ xuất hiện tại đây.", iconName: "receipt", action: html`<a class="btn btn-primary" href="#/products">Mua sắm ngay</a>` })
        : html`<div class="order-list">${orders.map((order) => html`
          <article class="card order-card">
            <div class="order-top"><div><b>${order.id}</b><div class="muted">Đặt lúc ${dateTime(order.createdAt)}</div></div>${statusBadge(order.status)}</div>
            ${timeline(order.status)}
            <div class="row spread">${thumbs(order)}<div style="text-align:right"><div class="muted">${order.items.reduce((sum, i) => sum + i.quantity, 0)} sản phẩm</div><div class="price" style="font-size:1.4rem">${vnd(order.total)}</div>${order.discount > 0 ? html`<div class="muted">Đã giảm ${vnd(order.discount)} (mã ${order.couponCode})</div>` : ""}</div></div>
            <div class="row"><a class="btn btn-secondary" href="#/orders/${encodeURIComponent(order.id)}">Xem chi tiết</a>
              ${order.status === "pending" ? html`<button class="btn btn-ghost" type="button" data-cancel="${order.id}">Hủy đơn</button>` : ""}</div>
          </article>`)}</div>`);
    } catch (error) { if (isCurrent()) mount(host, html`<div class="form-error">${icon("alert")}<span>${error.message}</span></div>`); }
  }
  host.addEventListener("click", (event) => cancelHandler(event, load));
  await load();
}

async function cancelHandler(event, reload) {
  const button = event.target.closest("[data-cancel]");
  if (!button) return;
  if (!(await confirmDialog({ title: "Hủy đơn hàng", message: `Bạn muốn hủy đơn ${button.dataset.cancel}? Hàng sẽ được trả lại kho và thao tác này không hoàn tác được.`, confirmLabel: "Hủy đơn", danger: true }))) return;
  try { await api(`/orders/${encodeURIComponent(button.dataset.cancel)}/cancel`, { method: "POST" }); toast("Đã hủy đơn hàng.", "success"); }
  catch (error) { toast(error.message, "error"); }
  await reload();
}

export async function orderDetailView({ main, params, setTitle, isCurrent }) {
  setTitle(`Đơn ${params.id}`);
  if (!requireLogin()) return;
  mount(main, html`<div class="wrap"><div class="loading-block"><div class="spinner" role="status" aria-label="Đang tải"></div></div></div>`);
  async function load() {
    try {
      const order = await api(`/orders/${encodeURIComponent(params.id)}`);
      if (!isCurrent()) return;
      mount(main, html`
        <div class="wrap">
          <div class="page-title"><nav class="crumbs"><a href="#/orders">Đơn hàng</a>${icon("chevron-right")}<span>${order.id}</span></nav>
            <div class="row spread"><h1>Đơn ${order.id}</h1>${statusBadge(order.status)}</div></div>
          <div class="card panel stack" style="margin-bottom:1.2rem">${timeline(order.status)}</div>
          <div class="cart-layout">
            <section class="card panel"><h2>Sản phẩm</h2>
              <ul class="stack">${order.items.map((item) => html`
                <li class="mini-line" style="grid-template-columns:72px 1fr auto"><img src="${safeSrc(item.product.image)}" alt="" width="72" height="72" style="width:72px;height:72px"><div><b>${item.product.name}</b><small>SL ${item.quantity} × ${vnd(item.product.price)}</small></div><span class="price" style="font-size:1.05rem">${vnd(item.product.price * item.quantity)}</span></li>`)}</ul>
              ${order.discount > 0 ? html`
                <div class="sum-row" style="margin-top:1.2rem"><span>Tạm tính</span><b>${vnd(order.subtotal)}</b></div>
                <div class="sum-row discount" style="margin-top:.5rem"><span>Giảm giá mã ${order.couponCode} (−${order.couponPercent}%)</span><b>−${vnd(order.discount)}</b></div>` : ""}
              <div class="sum-total" style="margin-top:1.2rem"><b>Tổng cộng</b><span class="price">${vnd(order.total)}</span></div>
            </section>
            <aside class="card panel"><h2>Thông tin</h2>
              <dl class="kv"><dt>Đặt lúc</dt><dd>${dateTime(order.createdAt)}</dd><dt>Cập nhật</dt><dd>${dateTime(order.updatedAt)}</dd><dt>Số điện thoại</dt><dd>${order.phone}</dd><dt>Địa chỉ</dt><dd>${order.shippingAddress}</dd><dt>Thanh toán</dt><dd>${PAYMENT_LABEL[order.paymentMethod] || order.paymentMethod}</dd><dt>Ghi chú</dt><dd>${order.note || "—"}</dd></dl>
              ${order.status === "pending" ? html`<button class="btn btn-danger btn-block" style="margin-top:1.2rem" type="button" data-cancel="${order.id}">Hủy đơn hàng</button>` : ""}
              <a class="btn btn-ghost btn-block" style="margin-top:.6rem" href="#/orders">${icon("arrow-left")}Tất cả đơn hàng</a>
            </aside>
          </div>
        </div>`);
      main.firstElementChild.addEventListener("click", (event) => cancelHandler(event, load));   // phần tử mới mỗi lần vẽ -> không tích lũy trình nghe
    } catch (error) {
      if (isCurrent()) mount(main, html`<div class="wrap">${emptyState({ title: "Không tìm thấy đơn hàng", text: error.message, action: html`<a class="btn btn-primary" href="#/orders">Về danh sách đơn</a>` })}</div>`);
    }
  }
  await load();
}
