import { html, icon, mount, safeSrc, vnd, go } from "/assets/shared.js";
import { state, api } from "../state.js";
import { productGrid, productGridSkeleton, categoryStyle } from "../components.js";

const SUGGESTIONS = ["Tai nghe dưới 2 triệu", "Quà tặng cho mẹ dưới 500k", "Sách bán chạy", "So sánh giày chạy bộ"];

export async function homeView({ main, setTitle, isCurrent }) {
  setTitle("");
  mount(main, html`
    <section class="hero" aria-labelledby="hero-title">
      <div class="wrap">
        <div>
          <h1 id="hero-title">Mua đúng thứ bạn cần, giá hiển thị rõ ràng.</h1>
          <p class="lead">Hơn 60 sản phẩm thật trong 8 danh mục, tồn kho cập nhật theo thời gian thực. Cứ hỏi trợ lý, chúng tôi gợi ý theo ngân sách của bạn.</p>
          <form class="ask" data-ask role="search" aria-label="Hỏi trợ lý mua sắm">
            <label class="sr-only" for="ask-q">Bạn đang tìm gì?</label>
            <input id="ask-q" name="q" placeholder="Ví dụ: tai nghe dưới 2 triệu" maxlength="200" autocomplete="off">
            <button class="btn btn-primary btn-lg" type="submit">${icon("sparkle")}Gợi ý cho tôi</button>
          </form>
          <div class="chips" aria-label="Gợi ý câu hỏi">${SUGGESTIONS.map((text) => html`<a class="chip" href="#/assistant?q=${encodeURIComponent(text)}">${text}</a>`)}</div>
        </div>
        <div class="hero-art" id="hero-art" aria-hidden="true"></div>
      </div>
    </section>
    <div class="wrap perks">
      <div class="perk">${icon("truck")}<div><b>Giao nhanh 2–3 ngày</b><span>Toàn quốc, kiểm tra hàng khi nhận</span></div></div>
      <div class="perk">${icon("shield")}<div><b>Chính hãng, có bảo hành</b><span>Hoàn tiền nếu hàng lỗi</span></div></div>
      <div class="perk">${icon("refresh")}<div><b>Hủy đơn dễ dàng</b><span>Khi đơn đang chờ xác nhận</span></div></div>
      <div class="perk">${icon("phone")}<div><b>Hỗ trợ ${state.store.supportPhone}</b><span>8:00 – 21:00 mỗi ngày</span></div></div>
    </div>
    <section class="section wrap" aria-labelledby="cat-title">
      <div class="section-head"><div><h2 id="cat-title">Mua theo danh mục</h2><p>Chọn nhóm hàng bạn quan tâm</p></div><a class="btn btn-secondary" href="#/products">Xem tất cả ${icon("arrow-right")}</a></div>
      <div class="cat-grid">${state.categories.map((category) => {
        const style = categoryStyle(category.id);
        return html`<a class="cat-card" href="#/products?category=${category.id}" style="background:${style.bg};color:${style.ink}">
          <b>${category.name}</b><span>${category.productCount} sản phẩm</span><span class="arrow">${icon("arrow-right")}</span></a>`;
      })}</div>
    </section>
    <section class="section wrap" aria-labelledby="best-title">
      <div class="section-head"><div><h2 id="best-title">Bán chạy nhất</h2><p>Được nhiều khách chọn mua trong tháng này</p></div><a class="btn btn-secondary" href="#/products?sort=popular">Xem thêm ${icon("arrow-right")}</a></div>
      <div id="best">${productGridSkeleton(4)}</div>
    </section>
    <section class="section wrap" aria-labelledby="new-title">
      <div class="section-head"><div><h2 id="new-title">Sản phẩm mới</h2><p>Vừa lên kệ gần đây</p></div><a class="btn btn-secondary" href="#/products?sort=newest">Xem thêm ${icon("arrow-right")}</a></div>
      <div id="newest">${productGridSkeleton(4)}</div>
    </section>`);

  main.querySelector("[data-ask]").addEventListener("submit", (event) => {
    event.preventDefault();
    const text = new FormData(event.target).get("q").toString().trim();
    go(text ? `/assistant?q=${encodeURIComponent(text)}` : "/assistant");
  });

  try {
    const [best, newest] = await Promise.all([
      api("/products?bestSelling=1&limit=4"),
      api("/products?sort=newest&inStock=1&limit=4"),
    ]);
    if (!isCurrent()) return;
    mount(main.querySelector("#best"), productGrid(best.items));
    mount(main.querySelector("#newest"), productGrid(newest.items));
    const art = main.querySelector("#hero-art");
    const picks = [...best.items, ...newest.items].slice(0, 2);
    mount(art, picks.map((product, i) => html`
      <a class="tile t${i + 1}" href="#/product/${product.id}" tabindex="-1">
        <img src="${safeSrc(product.image)}" alt="" width="300" height="300"><b>${product.name}</b><span>${vnd(product.price)}</span>
      </a>`));
  } catch (error) {
    if (!isCurrent()) return;
    mount(main.querySelector("#best"), html`<div class="form-error">${icon("alert")}<span>${error.message}</span></div>`);
    mount(main.querySelector("#newest"), "");
  }
}
