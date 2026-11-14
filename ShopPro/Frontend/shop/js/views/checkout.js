import { html, icon, mount, safeSrc, vnd, formValues, showFormError, withLoading, emptyState, toast, PAYMENT_LABEL } from "/assets/shared.js";
import { state, api, setCartQuantity, requireLogin } from "../state.js";
import { createCoupon, totalsHtml, saveCode } from "../coupon.js";

export async function checkoutView({ main, setTitle, isCurrent }) {
  setTitle("Thanh toán");
  if (!requireLogin()) return;
  mount(main, html`<div class="wrap"><div class="loading-block"><div class="spinner" role="status" aria-label="Đang tải"></div></div></div>`);
  let cart;
  try { cart = await api("/cart"); setCartQuantity(cart.totalQuantity); }
  catch (error) { if (isCurrent()) mount(main, html`<div class="wrap"><div class="form-error">${icon("alert")}<span>${error.message}</span></div></div>`); return; }
  if (!isCurrent()) return;
  if (cart.items.length === 0) {
    mount(main, html`<div class="wrap">${emptyState({ title: "Giỏ hàng đang trống", text: "Hãy thêm sản phẩm trước khi thanh toán.", iconName: "cart", action: html`<a class="btn btn-primary" href="#/products">Xem sản phẩm</a>` })}</div>`);
    return;
  }
  const user = state.user;

  // Mã giảm giá: máy chủ kiểm tra lại và tính tiền khi đặt hàng; ở đây chỉ hiển thị để khách biết số tiền phải trả
  const coupon = createCoupon({ getTotal: () => cart.total, isCurrent, onChange: () => { if (isCurrent()) paintSummary(); } });
  await coupon.refresh({ withList: true });
  if (!isCurrent()) return;

  const summaryHtml = () => html`
    <h2>Đơn hàng (${cart.totalQuantity} sản phẩm)</h2>
    <ul class="stack">${cart.items.map((item) => html`
      <li class="mini-line"><img src="${safeSrc(item.product.image)}" alt="" width="56" height="56"><div><b>${item.product.name}</b><small>SL ${item.quantity} × ${vnd(item.product.price)}</small></div><span class="price" style="font-size:1rem">${vnd(item.lineTotal)}</span></li>`)}</ul>
    <div class="sum-row"><span>Phí vận chuyển</span><b>Miễn phí</b></div>
    ${coupon.html()}
    ${totalsHtml(cart.total, coupon.applied)}
    <button class="btn btn-primary btn-lg btn-block" type="submit">Đặt hàng</button>
    <a class="btn btn-ghost btn-block" href="#/cart">${icon("arrow-left")}Quay lại giỏ hàng</a>`;
  const paintSummary = () => mount(main.querySelector("#checkout-summary"), summaryHtml());

  mount(main, html`
    <div class="wrap">
      <div class="page-title"><nav class="crumbs"><a href="#/cart">Giỏ hàng</a>${icon("chevron-right")}<span>Thanh toán</span></nav><h1>Thanh toán</h1></div>
      <form class="checkout" id="checkout-form" novalidate>
        <div class="stack">
          <section class="card panel">
            <h2>Thông tin giao hàng</h2>
            <div class="form-grid">
              <div class="field"><label for="customerName">Họ tên người nhận</label><input class="input" id="customerName" name="customerName" required minlength="2" maxlength="80" autocomplete="name" value="${user.name}"></div>
              <div class="field"><label for="phone">Số điện thoại</label><input class="input" id="phone" name="phone" required inputmode="tel" maxlength="20" autocomplete="tel" value="${user.phone}" placeholder="Ví dụ 0912345678"></div>
              <div class="field full"><label for="shippingAddress">Địa chỉ giao hàng</label><input class="input" id="shippingAddress" name="shippingAddress" required minlength="8" maxlength="300" autocomplete="street-address" value="${user.address}" placeholder="Số nhà, đường, phường/xã, quận/huyện, tỉnh/thành"></div>
              <div class="field full"><label for="note">Ghi chú cho người giao (không bắt buộc)</label><textarea class="textarea" id="note" name="note" maxlength="500" placeholder="Ví dụ: gọi trước khi giao, giao giờ hành chính"></textarea></div>
            </div>
          </section>
          <section class="card panel">
            <h2>Phương thức thanh toán</h2>
            <div class="stack" role="radiogroup" aria-label="Phương thức thanh toán">
              <label class="pay-opt"><input type="radio" name="paymentMethod" value="cod" checked><div><b>${PAYMENT_LABEL.cod}</b><span>Trả tiền mặt cho nhân viên giao hàng sau khi kiểm tra hàng.</span></div></label>
              <label class="pay-opt"><input type="radio" name="paymentMethod" value="bank"><div><b>${PAYMENT_LABEL.bank}</b><span>Cửa hàng sẽ liên hệ gửi thông tin tài khoản sau khi xác nhận đơn.</span></div></label>
            </div>
          </section>
        </div>
        <aside class="card summary" id="checkout-summary" aria-label="Đơn hàng của bạn">${summaryHtml()}</aside>
      </form>
    </div>`);

  const form = main.querySelector("#checkout-form");
  coupon.attach(form);
  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    showFormError(form, "");
    const values = formValues(form);
    const problems = [];
    if ((values.customerName || "").trim().length < 2) problems.push("Họ tên người nhận cần có ít nhất 2 ký tự.");
    if (!/^[0-9+()\-\s]{8,20}$/.test(values.phone || "")) problems.push("Số điện thoại chưa hợp lệ.");
    if ((values.shippingAddress || "").trim().length < 8) problems.push("Địa chỉ giao hàng cần ghi rõ hơn (ít nhất 8 ký tự).");
    if (problems.length) { showFormError(form, problems.join(" ")); return; }
    const button = form.querySelector("button[type=submit]");
    await withLoading(button, async () => {
      try {
        const order = await api("/orders", { method: "POST", body: {
          customerName: values.customerName.trim(), phone: values.phone.trim(), shippingAddress: values.shippingAddress.trim(),
          note: (values.note || "").trim(), paymentMethod: values.paymentMethod,
          couponCode: coupon.applied ? coupon.applied.code : "",
          items: cart.items.map((item) => ({ productId: item.product.id, quantity: item.quantity })),
        } });
        saveCode("");
        setCartQuantity(0);
        showSuccess(order);
      } catch (error) {
        showFormError(form, error.message);
        if (coupon.applied) { await coupon.refresh(); if (isCurrent()) paintSummary(); }   // mã có thể vừa hết lượt hoặc bị tắt
      }
    });
  });

  function showSuccess(order) {
    setTitle("Đặt hàng thành công");
    mount(main, html`
      <div class="wrap"><div class="empty" style="padding-block:4rem">
        <span class="brand-mark" style="background:var(--success-soft);color:var(--success);width:84px;height:84px;border-radius:50%;display:grid;place-items:center">${icon("check")}</span>
        <h1 class="serif">Đặt hàng thành công!</h1>
        <p>Mã đơn hàng của bạn là <b class="num" style="color:var(--ink)">${order.id}</b>. Tổng thanh toán <b>${vnd(order.total)}</b>.</p>
        ${order.discount > 0 ? html`<p>Đã áp dụng mã <b>${order.couponCode}</b>, bạn tiết kiệm <b>${vnd(order.discount)}</b>.</p>` : ""}
        <p>Cửa hàng sẽ xác nhận đơn trong thời gian sớm nhất. Bạn có thể theo dõi trạng thái trong mục đơn hàng.</p>
        <div class="row" style="justify-content:center"><a class="btn btn-primary btn-lg" href="#/orders/${encodeURIComponent(order.id)}">Xem đơn hàng</a><a class="btn btn-secondary btn-lg" href="#/products">Tiếp tục mua sắm</a></div>
      </div></div>`);
    window.scrollTo({ top: 0 });
    toast("Đơn hàng đã được ghi nhận.", "success");
  }
}
