import { html, icon, mount, safeSrc, vnd, toast, withLoading } from "/assets/shared.js";
import { state, api, requireLogin } from "../state.js";

const EXAMPLES = ["Tai nghe dưới 2 triệu", "Quà tặng cho mẹ dưới 500k", "Sách bán chạy", "So sánh giày chạy bộ", "Đồ gia dụng giá rẻ nhất"];

const pickCard = (product) => html`
  <div class="pick"><a href="#/product/${product.id}"><img src="${safeSrc(product.image)}" alt="" loading="lazy"></a>
    <div><b>${product.name}</b><small>${product.category} · còn ${product.stock}</small><span class="price">${vnd(product.price)}</span>
      <button class="btn btn-primary btn-sm" type="button" data-add="${product.id}">${icon("cart")}Thêm vào giỏ</button></div></div>`;

export async function assistantView({ main, query, setTitle }) {
  setTitle("Trợ lý mua sắm");
  if (!state.user) { requireLogin(); return; }
  mount(main, html`
    <div class="wrap">
      <div class="page-title"><h1>Trợ lý mua sắm</h1><p>Nói ngân sách và nhu cầu của bạn, trợ lý lọc sản phẩm còn hàng phù hợp. Trợ lý dùng quy tắc trên dữ liệu cửa hàng, không gọi dịch vụ bên ngoài.</p></div>
      <div class="assistant">
        <section class="card chat" aria-label="Hội thoại với trợ lý">
          <div class="chat-log" id="log" role="log" aria-live="polite">
            <div class="msg bot"><div class="bubble">Xin chào ${state.user.name.split(" ").slice(-1)[0]}! Bạn đang tìm gì hôm nay? Ví dụ: “tai nghe dưới 2 triệu” hoặc “so sánh giày chạy bộ”.</div></div>
          </div>
          <form class="chat-form" id="chat-form">
            <label class="sr-only" for="chat-input">Câu hỏi của bạn</label>
            <input class="input" id="chat-input" name="message" maxlength="300" placeholder="Nhập nhu cầu của bạn…" autocomplete="off" required>
            <button class="btn btn-primary" type="submit">${icon("send")}Gửi</button>
          </form>
        </section>
        <aside class="card tips"><h3>Thử hỏi nhanh</h3>${EXAMPLES.map((text) => html`<button type="button" data-example="${text}">${text}</button>`)}</aside>
      </div>
    </div>`);
  const log = main.querySelector("#log");
  const form = main.querySelector("#chat-form");
  const input = form.querySelector("input");

  async function ask(text) {
    const message = text.trim();
    if (!message) return;
    const me = document.createElement("div");
    me.className = "msg me";
    me.innerHTML = html`<div class="bubble">${message}</div>`.toString();
    log.append(me);
    const waiting = document.createElement("div");
    waiting.className = "msg bot";
    waiting.innerHTML = '<div class="bubble"><span class="typing" role="status" aria-label="Trợ lý đang trả lời"><i></i><i></i><i></i></span></div>';
    log.append(waiting);
    log.scrollTop = log.scrollHeight;
    try {
      const result = await api("/ai/chat", { method: "POST", body: { message } });
      const filters = [];
      if (result.filters?.category) filters.push(`danh mục “${result.filters.category}”`);
      if (result.filters?.maxPrice) filters.push(`giá tối đa ${vnd(result.filters.maxPrice)}`);
      waiting.innerHTML = html`<div class="bubble">${result.answer}${filters.length ? html`<br><small class="muted">Đã lọc theo: ${filters.join(", ")}.</small>` : ""}</div>
        ${result.products.length ? html`<div class="picks">${result.products.map(pickCard)}</div>` : ""}`.toString();
    } catch (error) {
      waiting.innerHTML = html`<div class="bubble">Rất tiếc, trợ lý chưa trả lời được: ${error.message}</div>`.toString();
      toast(error.message, "error");
    }
    log.scrollTop = log.scrollHeight;
  }
  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    const text = input.value;
    input.value = "";
    await ask(text);
    input.focus();
  });
  main.querySelectorAll("[data-example]").forEach((button) => button.addEventListener("click", () => ask(button.dataset.example)));
  if (query.q) ask(query.q);
}
