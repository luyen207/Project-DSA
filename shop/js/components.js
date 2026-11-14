// Các thành phần giao diện dùng lại nhiều nơi trong Cửa hàng.
import { html, icon, safeSrc, vnd } from "/assets/shared.js";

export function productCard(product) {
  const out = product.stock < 1;
  return html`
    <article class="p-card">
      <a class="p-media" href="#/product/${product.id}" tabindex="-1" aria-hidden="true">
        <img src="${safeSrc(product.image)}" alt="" loading="lazy" width="400" height="400">
        ${product.badge ? html`<span class="p-badge ${out ? "out" : product.bestSelling ? "best" : ""}">${product.badge}</span>` : ""}
      </a>
      <div class="p-body">
        <p class="p-cat">${product.category}</p>
        <h3 class="p-name"><a href="#/product/${product.id}">${product.name}</a></h3>
        <div class="p-foot">
          <p class="price">${vnd(product.price)}</p>
          ${!out && product.stock <= 10 ? html`<p class="p-low">Chỉ còn ${product.stock} sản phẩm</p>` : ""}
        </div>
        <button class="btn btn-primary btn-block" type="button" data-add="${product.id}" ${out ? "disabled" : ""}>${icon("cart")}${out ? "Hết hàng" : "Thêm vào giỏ"}</button>
      </div>
    </article>`;
}

export const productGrid = (products) => html`<div class="p-grid">${products.map(productCard)}</div>`;

export function qtyStepper({ id, value, max }) {
  return html`
    <div class="qty" role="group" aria-label="Số lượng">
      <button type="button" class="qty-btn" data-qty-dec="${id}" aria-label="Giảm số lượng" ${value <= 1 ? "disabled" : ""}>${icon("minus")}</button>
      <input class="qty-input" inputmode="numeric" autocomplete="off" value="${value}" data-qty-input="${id}" data-max="${max}" aria-label="Số lượng">
      <button type="button" class="qty-btn" data-qty-inc="${id}" aria-label="Tăng số lượng" ${value >= max ? "disabled" : ""}>${icon("plus")}</button>
    </div>`;
}

// Phân trang: href dựng bởi hrefFor(page)
export function pagination({ page, totalPages, hrefFor }) {
  if (totalPages <= 1) return "";
  const pages = new Set([1, totalPages, page - 1, page, page + 1]);
  const list = [...pages].filter((p) => p >= 1 && p <= totalPages).sort((a, b) => a - b);
  const items = [];
  list.forEach((p, i) => {
    if (i > 0 && p - list[i - 1] > 1) items.push(html`<span class="muted" aria-hidden="true">…</span>`);
    items.push(html`<a class="btn btn-secondary btn-sm" href="${hrefFor(p)}" ${p === page ? html`aria-current="page"` : ""}>${p}</a>`);
  });
  return html`<nav class="pagination" aria-label="Phân trang">
    ${page > 1 ? html`<a class="btn btn-secondary btn-sm" href="${hrefFor(page - 1)}" aria-label="Trang trước">${icon("chevron-left")}</a>` : ""}
    ${items}
    ${page < totalPages ? html`<a class="btn btn-secondary btn-sm" href="${hrefFor(page + 1)}" aria-label="Trang sau">${icon("chevron-right")}</a>` : ""}
  </nav>`;
}

export const productGridSkeleton = (count = 8) => html`<div class="p-grid">${Array.from({ length: count }, () => html`
  <div class="p-card"><div class="skeleton" style="aspect-ratio:1/1;border-radius:0"></div>
  <div class="p-body"><div class="skeleton" style="height:16px;width:50%"></div><div class="skeleton" style="height:22px"></div><div class="skeleton" style="height:26px;width:60%"></div><div class="skeleton" style="height:48px"></div></div></div>`)}</div>`;

export const CATEGORY_STYLE = {
  1: { bg: "#d6e7ff", ink: "#0b3e8a", dot: "#1e6fd9" },
  2: { bg: "#ffdccf", ink: "#7d2410", dot: "#e4572e" },
  3: { bg: "#cdf1ea", ink: "#04493f", dot: "#0e9f8e" },
  4: { bg: "#ffeab8", ink: "#5e3a00", dot: "#d98a00" },
  5: { bg: "#d4f1dc", ink: "#0f4b21", dot: "#2e9e4f" },
  6: { bg: "#cdeff8", ink: "#084a60", dot: "#1aa6c9" },
  7: { bg: "#e5f4c8", ink: "#2d4a08", dot: "#6fa61f" },
  8: { bg: "#f1ddc8", ink: "#4e2c0c", dot: "#b5651d" },
};
export const categoryStyle = (id) => CATEGORY_STYLE[id] || { bg: "#e4effc", ink: "#084a9c", dot: "#0b63ce" };
