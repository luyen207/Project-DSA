import { html, icon, mount, go, queryString, number, emptyState } from "/assets/shared.js";
import { state, api } from "../state.js";
import { productGrid, productGridSkeleton, pagination } from "../components.js";

const SORTS = [["", "Mặc định"], ["popular", "Bán chạy nhất"], ["newest", "Mới nhất"], ["price-asc", "Giá thấp đến cao"], ["price-desc", "Giá cao đến thấp"], ["name", "Tên A – Z"]];
const PAGE_SIZE = 12;

export async function productsView({ main, query, setTitle, isCurrent }) {
  const filters = {
    search: (query.search || "").trim().slice(0, 100),
    category: /^\d+$/.test(query.category || "") ? query.category : "",
    minPrice: /^\d+$/.test(query.minPrice || "") ? query.minPrice : "",
    maxPrice: /^\d+$/.test(query.maxPrice || "") ? query.maxPrice : "",
    inStock: query.inStock === "1" ? "1" : "",
    sort: SORTS.some(([value]) => value === query.sort) ? query.sort || "" : "",
    page: Math.max(1, Math.min(10000, parseInt(query.page, 10) || 1)),
  };
  const category = state.categories.find((item) => String(item.id) === filters.category);
  const href = (overrides = {}) => `#/products${queryString({ ...filters, page: filters.page, ...overrides })}`;
  const baseHref = (overrides = {}) => href({ page: "", ...overrides });
  setTitle(category ? category.name : filters.search ? `Tìm “${filters.search}”` : "Tất cả sản phẩm");

  mount(main, html`
    <div class="wrap">
      <div class="page-title">
        <nav class="crumbs" aria-label="Đường dẫn"><a href="#/">Trang chủ</a>${icon("chevron-right")}<span>Sản phẩm</span>${category ? html`${icon("chevron-right")}<span>${category.name}</span>` : ""}</nav>
        <h1>${filters.search ? html`Kết quả cho “${filters.search}”` : category ? category.name : "Tất cả sản phẩm"}</h1>
        ${category?.description ? html`<p>${category.description}</p>` : ""}
      </div>
      <div class="catalog">
        <aside class="filters card" aria-label="Bộ lọc">
          <div><h3>Danh mục</h3>
            <div class="filter-list">
              <a href="${baseHref({ category: "" })}" ${!filters.category ? html`aria-current="true"` : ""}><span>Tất cả</span></a>
              ${state.categories.map((item) => html`<a href="${baseHref({ category: item.id })}" ${String(item.id) === filters.category ? html`aria-current="true"` : ""}><span>${item.name}</span><span class="n">${item.productCount}</span></a>`)}
            </div>
          </div>
          <form data-price-form class="stack">
            <h3>Khoảng giá (₫)</h3>
            <div class="price-range">
              <div class="field"><label class="sr-only" for="minPrice">Giá từ</label><input class="input" id="minPrice" name="minPrice" inputmode="numeric" placeholder="Từ" value="${filters.minPrice}" maxlength="11"></div>
              <div class="field"><label class="sr-only" for="maxPrice">Giá đến</label><input class="input" id="maxPrice" name="maxPrice" inputmode="numeric" placeholder="Đến" value="${filters.maxPrice}" maxlength="11"></div>
            </div>
            <label class="check"><input type="checkbox" name="inStock" value="1" ${filters.inStock ? "checked" : ""}>Chỉ hiện hàng còn</label>
            <button class="btn btn-soft btn-block" type="submit">${icon("filter")}Áp dụng lọc</button>
          </form>
        </aside>
        <section aria-live="polite">
          <div class="toolbar">
            <p class="result" id="result-count">Đang tải…</p>
            <div class="field"><label class="sr-only" for="sort">Sắp xếp</label>
              <select class="select" id="sort" data-sort>${SORTS.map(([value, label]) => html`<option value="${value}" ${value === filters.sort ? "selected" : ""}>${label}</option>`)}</select>
            </div>
          </div>
          <div class="active-filters" id="active-filters"></div>
          <div id="grid">${productGridSkeleton(6)}</div>
          <div id="pager"></div>
        </section>
      </div>
    </div>`);

  main.querySelector("[data-sort]").addEventListener("change", (event) => go(baseHref({ sort: event.target.value }).slice(1)));
  main.querySelector("[data-price-form]").addEventListener("submit", (event) => {
    event.preventDefault();
    const data = new FormData(event.target);
    const clean = (value) => String(value || "").replace(/\D/g, "");
    go(baseHref({ minPrice: clean(data.get("minPrice")), maxPrice: clean(data.get("maxPrice")), inStock: data.get("inStock") ? "1" : "" }).slice(1));
  });

  const pills = [];
  if (filters.search) pills.push([`Từ khóa: ${filters.search}`, baseHref({ search: "" })]);
  if (filters.minPrice) pills.push([`Từ ${number(filters.minPrice)} ₫`, baseHref({ minPrice: "" })]);
  if (filters.maxPrice) pills.push([`Đến ${number(filters.maxPrice)} ₫`, baseHref({ maxPrice: "" })]);
  if (filters.inStock) pills.push(["Còn hàng", baseHref({ inStock: "" })]);
  mount(main.querySelector("#active-filters"), pills.map(([label, link]) => html`<a class="pill" href="${link}" aria-label="Bỏ lọc ${label}">${label}${icon("x")}</a>`));

  try {
    const params = { search: filters.search, category: filters.category, minPrice: filters.minPrice, maxPrice: filters.maxPrice, inStock: filters.inStock, sort: filters.sort, page: filters.page, limit: PAGE_SIZE };
    const result = await api(`/products${queryString(params)}`);
    if (!isCurrent()) return;
    main.querySelector("#result-count").textContent = `${result.total} sản phẩm`;
    mount(main.querySelector("#grid"), result.items.length
      ? productGrid(result.items)
      : emptyState({ title: "Không có sản phẩm phù hợp", text: "Thử bỏ bớt bộ lọc hoặc dùng từ khóa khác.", iconName: "search", action: html`<a class="btn btn-primary" href="#/products">Xem tất cả sản phẩm</a>` }));
    mount(main.querySelector("#pager"), pagination({ page: result.page, totalPages: Math.ceil(result.total / PAGE_SIZE), hrefFor: (page) => href({ page }) }));
  } catch (error) {
    if (!isCurrent()) return;
    mount(main.querySelector("#grid"), html`<div class="form-error">${icon("alert")}<span>${error.message}</span></div>`);
  }
}
