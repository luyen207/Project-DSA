// Trang giỏ hàng: mọi thao tác đi qua REST; thứ tự dòng hàng chính là thứ tự các node của danh sách liên kết đôi trên server.
import { html, icon, mount, safeSrc, vnd, number, toast, confirmDialog, emptyState, debounce, withLoading } from "/assets/shared.js";
import { state, api, setCartQuantity, requireLogin } from "../state.js";
import { qtyStepper } from "../components.js";

const SORT_OPTIONS = [["id", "Mã sản phẩm"], ["name", "Tên A – Z"], ["price-asc", "Giá tăng dần"], ["price-desc", "Giá giảm dần"]];

export async function cartView({ main, setTitle, isCurrent }) {
  setTitle("Giỏ hàng");
  if (!requireLogin()) return;
  mount(main, html`<div class="wrap"><div class="page-title"><h1>Giỏ hàng của bạn</h1></div><div id="cart-root"><div class="loading-block"><div class="spinner" role="status" aria-label="Đang tải"></div></div></div></div>`);
  const root = main.querySelector("#cart-root");

  const ui = { cart: null, query: "", matches: null, direction: "forward", structure: null, sortedBy: "" };

  async function load() {
    ui.cart = await api("/cart");
    setCartQuantity(ui.cart.totalQuantity);
  }
  async function loadStructure() {
    try { ui.structure = await api(`/cart/structure?direction=${ui.direction}`); } catch { ui.structure = null; }
  }

  /* FLIP: ghi lại vị trí trước khi vẽ lại rồi trượt các dòng về vị trí mới */
  function renderLines(animate) {
    const before = new Map();
    if (animate) root.querySelectorAll("[data-line]").forEach((node) => before.set(node.dataset.line, node.getBoundingClientRect().top));
    paint();
    if (!animate || window.matchMedia("(prefers-reduced-motion: reduce)").matches) return;
    root.querySelectorAll("[data-line]").forEach((node) => {
      const previous = before.get(node.dataset.line);
      if (previous === undefined) return;
      const delta = previous - node.getBoundingClientRect().top;
      if (!delta) return;
      node.animate([{ transform: `translateY(${delta}px)` }, { transform: "none" }], { duration: 420, easing: "cubic-bezier(.2,.8,.2,1)" });
    });
  }

  function lineHtml(item) {
    const { product } = item;
    const matched = ui.matches ? ui.matches.has(product.id) : false;
    const dim = ui.matches && !matched;
    return html`
      <li class="line ${matched ? "match" : ""} ${dim ? "dim" : ""}" data-line="${product.id}">
        <a href="#/product/${product.id}"><img src="${safeSrc(product.image)}" alt="" width="96" height="96" loading="lazy"></a>
        <div>
          <h3><a href="#/product/${product.id}">${product.name}</a></h3>
          <div class="meta"><span class="price">${vnd(product.price)}</span><span class="muted">Mã SP: ${product.id}</span></div>
          <div class="meta">${qtyStepper({ id: product.id, value: item.quantity, max: Math.max(1, product.stock) })}
            <button class="link-btn" type="button" data-remove="${product.id}">${"Xóa"}</button></div>
          ${item.exceedsStock ? html`<p class="warn">${icon("alert")}Kho chỉ còn ${product.stock} sản phẩm, hãy giảm số lượng.</p>` : ""}
        </div>
        <div class="sub"><span class="muted">Thành tiền</span><span class="price">${vnd(item.lineTotal)}</span></div>
      </li>`;
  }

  function structureHtml() {
    const s = ui.structure;
    if (!s) return html`<p class="muted">Chưa tải được cấu trúc danh sách.</p>`;
    const forward = s.direction === "forward";
    const nodes = s.nodes.map((node, index) => {
      const isHead = node.id === s.head;
      const isTail = node.id === s.tail;
      return html`${index > 0 ? html`<span class="link" aria-hidden="true">${linkArrows()}</span>` : ""}
        <div class="node ${isHead ? "is-head" : ""} ${isTail ? "is-tail" : ""}">
          <div class="tag">${isHead ? html`<span>HEAD</span>` : ""}${isTail ? html`<span>TAIL</span>` : ""}</div>
          <b>${node.name}</b>
          <small>Mã ${node.id} · SL ${node.quantity} · ${vnd(node.price)}</small>
          <small>prev: ${node.prev ?? "NULL"} · next: ${node.next ?? "NULL"}</small>
        </div>`;
    });
    return html`
      <div class="chain-wrap"><div class="chain" role="list" aria-label="Các node của danh sách liên kết đôi">
        <span class="end">NULL</span><span class="link" aria-hidden="true">${linkArrows()}</span>
        ${nodes}
        <span class="link" aria-hidden="true">${linkArrows()}</span><span class="end">NULL</span>
      </div></div>
      <p class="muted">Đang duyệt theo chiều <b>${forward ? "xuôi (head → tail)" : "ngược (tail → head)"}</b> · ${s.size} node.</p>
      <pre class="textview" tabindex="0" aria-label="Dạng văn bản của danh sách">${s.textView}</pre>`;
  }
  const linkArrows = () => html`<svg viewBox="0 0 54 24" fill="none" stroke="currentColor" stroke-width="2.2" stroke-linecap="round" stroke-linejoin="round"><path d="M4 8h46M44 3l6 5-6 5M50 16H4M10 11l-6 5 6 5"/></svg>`;

  function paint() {
    const { cart } = ui;
    if (cart.items.length === 0) {
      mount(root, emptyState({ title: "Giỏ hàng đang trống", text: "Thêm vài sản phẩm bạn thích, rồi quay lại đây để thanh toán.", iconName: "cart", action: html`<a class="btn btn-primary btn-lg" href="#/products">Bắt đầu mua sắm</a>` }));
      return;
    }
    const hasOverstock = cart.items.some((item) => item.exceedsStock);
    mount(root, html`
      <div class="cart-layout">
        <div>
          <section class="card cart-tools" aria-label="Công cụ giỏ hàng">
            <div class="row">
              <div class="search-in"><label class="sr-only" for="cart-q">Tìm trong giỏ</label>${icon("search")}<input class="input" id="cart-q" placeholder="Tìm trong giỏ theo tên sản phẩm" value="${ui.query}" maxlength="100" autocomplete="off"></div>
              ${ui.query ? html`<button class="btn btn-ghost" type="button" data-clear-search>Bỏ tìm</button>` : ""}
            </div>
            ${ui.query ? html`<p role="status" class="muted">${ui.matches ? `Tìm thấy ${ui.matches.size} sản phẩm khớp “${ui.query}”.` : "Đang tìm…"}</p>` : ""}
            <div class="row"><span class="label">Sắp xếp giỏ:</span>
              <div class="sort-group">${SORT_OPTIONS.map(([key, label]) => html`<button class="btn btn-sm ${ui.sortedBy === key ? "btn-primary" : "btn-secondary"}" type="button" data-sort="${key}">${label}</button>`)}</div>
            </div>
          </section>
          <ul class="cart-lines" aria-label="Sản phẩm trong giỏ">${cart.items.map(lineHtml)}</ul>
          <div class="row spread" style="margin-top:1rem"><a class="btn btn-ghost" href="#/products">${icon("arrow-left")}Tiếp tục mua sắm</a><button class="btn btn-secondary" type="button" data-clear-cart>${icon("trash")}Xóa toàn bộ giỏ</button></div>
        </div>
        <aside class="card summary" aria-label="Tóm tắt đơn hàng">
          <h2>Tóm tắt</h2>
          <div class="sum-row"><span>Số loại sản phẩm</span><b class="num">${cart.distinctCount}</b></div>
          <div class="sum-row"><span>Tổng số lượng</span><b class="num">${cart.totalQuantity}</b></div>
          <div class="sum-row"><span>Phí vận chuyển</span><b>Miễn phí</b></div>
          <div class="sum-total"><b>Tạm tính</b><span class="price">${vnd(cart.total)}</span></div>
          ${hasOverstock ? html`<div class="form-error">${icon("alert")}<span>Có sản phẩm vượt tồn kho. Hãy điều chỉnh số lượng trước khi thanh toán.</span></div>` : ""}
          <a class="btn btn-primary btn-lg btn-block" href="#/checkout" ${hasOverstock ? html`aria-disabled="true" data-blocked` : ""}>Tiến hành thanh toán ${icon("arrow-right")}</a>
        </aside>
      </div>
      <section class="dsa" aria-labelledby="dsa-title">
        <div class="dsa-head">
          <h2 id="dsa-title">${icon("link", "")} Giỏ hàng nhìn từ bên trong</h2>
          <div class="row">
            <span class="badge plain ${ui.structure?.valid ? "badge-ok" : "badge-bad"}">${ui.structure ? (ui.structure.valid ? "Liên kết hợp lệ" : "Liên kết lỗi") : "—"}</span>
            <button class="btn btn-secondary btn-sm" type="button" data-direction>${icon("swap")}Duyệt ${ui.direction === "forward" ? "ngược" : "xuôi"}</button>
          </div>
        </div>
        <p class="note">Mỗi sản phẩm là một node của danh sách liên kết đôi (C++), mỗi node giữ con trỏ <code>prev</code> và <code>next</code>. Khi bạn sắp xếp, thứ tự node được đổi lại và các liên kết vẫn được kiểm tra hợp lệ.</p>
        <div id="structure">${structureHtml()}</div>
      </section>`);
  }

  async function mutate(task, { animate = false } = {}) {
    try {
      const result = await task();
      if (result) { ui.cart = result; setCartQuantity(result.totalQuantity); }
      else await load();
      await loadStructure();
      if (!isCurrent()) return;
      if (ui.query) await runSearch(ui.query, false);
      renderLines(animate);
    } catch (error) {
      toast(error.message, "error");
      try { await load(); await loadStructure(); if (isCurrent()) renderLines(false); } catch { /* giữ nguyên màn hình */ }
    }
  }

  async function runSearch(text, repaint = true) {
    ui.query = text;
    if (!text) { ui.matches = null; if (repaint) renderLines(false); return; }
    try {
      const result = await api(`/cart/search?q=${encodeURIComponent(text)}`);
      if (!isCurrent() || ui.query !== text) return;
      ui.matches = new Set(result.items.map((item) => item.product.id));
    } catch (error) { ui.matches = new Set(); toast(error.message, "error"); }
    if (repaint) { const keep = root.querySelector("#cart-q"); const pos = keep?.selectionStart; renderLines(false); const again = root.querySelector("#cart-q"); if (again) { again.focus(); again.setSelectionRange(pos ?? text.length, pos ?? text.length); } }
  }
  const debouncedSearch = debounce((text) => runSearch(text.trim()), 280);

  root.addEventListener("input", (event) => {
    if (event.target.id === "cart-q") debouncedSearch(event.target.value);
    if (event.target.matches("[data-qty-input]")) event.target.value = event.target.value.replace(/\D/g, "").slice(0, 5);
  });
  root.addEventListener("change", (event) => {
    const input = event.target.closest("[data-qty-input]");
    if (!input) return;
    const id = input.dataset.qtyInput;
    const max = Number(input.dataset.max) || 1;
    const quantity = Math.max(1, Math.min(max, parseInt(input.value, 10) || 1));
    mutate(() => api(`/cart/items/${id}`, { method: "PATCH", body: { quantity } }));
  });
  root.addEventListener("click", async (event) => {
    const t = event.target;
    const dec = t.closest("[data-qty-dec]"), inc = t.closest("[data-qty-inc]");
    if (dec || inc) {
      const id = (dec || inc).dataset.qtyDec || (dec || inc).dataset.qtyInc;
      const item = ui.cart.items.find((entry) => String(entry.product.id) === String(id));
      if (!item) return;
      const quantity = item.quantity + (inc ? 1 : -1);
      if (quantity < 1) return;
      if (quantity > item.product.stock) { toast(`Kho chỉ còn ${item.product.stock} sản phẩm.`, "info"); return; }
      return mutate(() => api(`/cart/items/${id}`, { method: "PATCH", body: { quantity } }));
    }
    const remove = t.closest("[data-remove]");
    if (remove) return mutate(() => api(`/cart/items/${remove.dataset.remove}`, { method: "DELETE" }));
    const sort = t.closest("[data-sort]");
    if (sort) {
      ui.sortedBy = sort.dataset.sort;
      return mutate(() => api("/cart/sort", { method: "POST", body: { by: ui.sortedBy } }), { animate: true });
    }
    if (t.closest("[data-direction]")) {
      ui.direction = ui.direction === "forward" ? "backward" : "forward";
      await loadStructure();
      return renderLines(false);
    }
    if (t.closest("[data-clear-search]")) return runSearch("");
    if (t.closest("[data-clear-cart]")) {
      if (await confirmDialog({ title: "Xóa toàn bộ giỏ hàng", message: "Tất cả sản phẩm trong giỏ sẽ bị xóa. Bạn chắc chắn chứ?", confirmLabel: "Xóa giỏ hàng", danger: true })) {
        mutate(() => api("/cart", { method: "DELETE" }));
      }
      return;
    }
    const blocked = t.closest("[data-blocked]");
    if (blocked) { event.preventDefault(); toast("Hãy giảm số lượng các sản phẩm vượt tồn kho trước.", "error"); }
  });

  try {
    await load();
    await loadStructure();
    if (!isCurrent()) return;
    renderLines(false);
  } catch (error) {
    if (!isCurrent()) return;
    mount(root, html`<div class="form-error">${icon("alert")}<span>${error.message}</span></div>`);
  }
}
