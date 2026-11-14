import { html, icon, mount, safeSrc, vnd, emptyState, toast, withLoading, go } from "/assets/shared.js";
import { state, api, addToCart, requireLogin } from "../state.js";
import { productGrid } from "../components.js";

export async function productView({ main, params, setTitle, isCurrent }) {
  mount(main, html`<div class="loading-block"><div class="spinner" role="status" aria-label="Đang tải"></div></div>`);
  let product;
  try { product = await api(`/products/${params.id}`); }
  catch (error) {
    if (!isCurrent()) return;
    setTitle("Không tìm thấy sản phẩm");
    mount(main, html`<div class="wrap">${emptyState({ title: "Không tìm thấy sản phẩm", text: error.message, action: html`<a class="btn btn-primary" href="#/products">Xem sản phẩm khác</a>` })}</div>`);
    return;
  }
  if (!isCurrent()) return;
  setTitle(product.name);
  const out = product.stock < 1;
  const low = !out && product.stock <= 10;

  mount(main, html`
    <div class="wrap">
      <div class="page-title"><nav class="crumbs" aria-label="Đường dẫn"><a href="#/">Trang chủ</a>${icon("chevron-right")}<a href="#/products?category=${product.categoryId}">${product.category}</a>${icon("chevron-right")}<span>${product.name}</span></nav></div>
      <div class="detail">
        <div class="detail-media"><img src="${safeSrc(product.image)}" alt="${product.name}" width="640" height="640"></div>
        <div class="detail-info">
          <div class="row">${product.badge ? html`<span class="badge plain ${out ? "st-hidden" : "st-pending"}">${product.badge}</span>` : ""}<span class="muted">${product.category}</span></div>
          <h1>${product.name}</h1>
          <p class="detail-price">${vnd(product.price)}</p>
          <p class="stock-line ${out ? "stock-out" : low ? "stock-low" : "stock-ok"}">${icon(out ? "x" : low ? "alert" : "check")}${out ? "Tạm hết hàng" : low ? `Chỉ còn ${product.stock} sản phẩm` : `Còn hàng (${product.stock} sản phẩm)`}</p>
          <p class="desc">${product.description}</p>
          ${out ? "" : html`
            <div class="buy-row">
              <div class="qty" role="group" aria-label="Số lượng">
                <button type="button" class="qty-btn" data-dec aria-label="Giảm số lượng">${icon("minus")}</button>
                <input class="qty-input" id="qty" inputmode="numeric" value="1" aria-label="Số lượng" autocomplete="off">
                <button type="button" class="qty-btn" data-inc aria-label="Tăng số lượng">${icon("plus")}</button>
              </div>
              <button class="btn btn-primary btn-lg" type="button" data-buy="cart">${icon("cart")}Thêm vào giỏ</button>
              <button class="btn btn-secondary btn-lg" type="button" data-buy="now">Mua ngay</button>
            </div>`}
          <div class="assurances">
            <div>${icon("truck")}<span>Giao hàng toàn quốc trong 2–3 ngày làm việc</span></div>
            <div>${icon("shield")}<span>Cam kết hàng chính hãng, đổi trả khi lỗi do nhà sản xuất</span></div>
            <div>${icon("refresh")}<span>Hủy đơn miễn phí khi đơn chưa được xác nhận</span></div>
          </div>
        </div>
      </div>
      <section class="section" aria-labelledby="rel-title" id="related-section" hidden>
        <div class="section-head"><h2 id="rel-title">Sản phẩm cùng danh mục</h2></div><div id="related"></div>
      </section>
    </div>`);

  const input = main.querySelector("#qty");
  if (input) {
    const read = () => Math.max(1, Math.min(product.stock, parseInt(input.value, 10) || 1));
    input.addEventListener("input", () => { input.value = input.value.replace(/\D/g, "").slice(0, 5); });
    input.addEventListener("blur", () => { input.value = read(); });
    main.querySelector("[data-dec]").addEventListener("click", () => { input.value = Math.max(1, read() - 1); });
    main.querySelector("[data-inc]").addEventListener("click", () => {
      if (read() >= product.stock) toast(`Kho chỉ còn ${product.stock} sản phẩm.`, "info");
      input.value = Math.min(product.stock, read() + 1);
    });
    main.querySelectorAll("[data-buy]").forEach((button) => button.addEventListener("click", async () => {
      if (!requireLogin()) { toast("Hãy đăng nhập để mua hàng.", "info"); return; }
      await withLoading(button, async () => {
        try {
          await addToCart(product.id, read());
          if (button.dataset.buy === "now") go("/cart");
          else toast("Đã thêm vào giỏ hàng.", "success", { action: { href: "#/cart", label: "Xem giỏ" } });
        } catch (error) { toast(error.message, "error"); }
      });
    }));
  }

  try {
    const related = await api(`/products/${product.id}/related`);
    if (!isCurrent() || related.length === 0) return;
    main.querySelector("#related-section").hidden = false;
    mount(main.querySelector("#related"), productGrid(related));
  } catch { /* phần gợi ý là tùy chọn */ }
}
