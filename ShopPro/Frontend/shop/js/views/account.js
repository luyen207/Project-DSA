import { html, icon, mount, go, formValues, showFormError, withLoading, toast } from "/assets/shared.js";
import { state, api, setSession, setCartQuantity, requireLogin } from "../state.js";

const DEMO = { email: "khach@shoppro.vn", password: "Demo@12345" };

function safeNext(query) {
  const next = query.next || "/";
  return next.startsWith("/") && !next.startsWith("//") ? next : "/";
}
function passwordField(id, label, { autocomplete, hint } = {}) {
  return html`<div class="field"><label for="${id}">${label}</label>
    <div class="pw"><input class="input" id="${id}" name="${id}" type="password" required maxlength="128" autocomplete="${autocomplete}">
    <button type="button" data-toggle-pw="${id}" aria-label="Hiện hoặc ẩn mật khẩu">${icon("eye")}</button></div>${hint ? html`<span class="hint">${hint}</span>` : ""}</div>`;
}
function wirePasswordToggles(root) {
  root.addEventListener("click", (event) => {
    const button = event.target.closest("[data-toggle-pw]");
    if (!button) return;
    const input = root.querySelector(`#${button.dataset.togglePw}`);
    const show = input.type === "password";
    input.type = show ? "text" : "password";
    button.innerHTML = icon(show ? "eyeoff" : "eye").toString();
  });
}
async function afterAuth(session, query) {
  setSession(session);
  try { const cart = await api("/cart"); setCartQuantity(cart.totalQuantity); } catch { /* giỏ trống */ }
  go(safeNext(query));
}

export function loginView({ main, query, setTitle }) {
  setTitle("Đăng nhập");
  if (state.user) { go(safeNext(query)); return; }
  mount(main, html`
    <div class="wrap auth"><div class="card">
      <h1>Đăng nhập</h1>
      <p class="muted">Đăng nhập để dùng giỏ hàng và theo dõi đơn hàng.</p>
      <div class="demo-box"><div><b>Tài khoản demo</b><br><span class="num">${DEMO.email}</span> · <span class="num">${DEMO.password}</span></div><button class="btn btn-secondary btn-sm" type="button" data-demo>Điền sẵn</button></div>
      <form class="stack" id="login-form" novalidate>
        <div class="field"><label for="email">Email</label><input class="input" id="email" name="email" type="email" required maxlength="160" autocomplete="username" inputmode="email"></div>
        ${passwordField("password", "Mật khẩu", { autocomplete: "current-password" })}
        <button class="btn btn-primary btn-lg btn-block" type="submit">Đăng nhập</button>
      </form>
      <p style="margin-top:1.2rem">Chưa có tài khoản? <a href="#/register${query.next ? `?next=${encodeURIComponent(query.next)}` : ""}">Đăng ký miễn phí</a></p>
    </div></div>`);
  const form = main.querySelector("#login-form");
  wirePasswordToggles(main);
  main.querySelector("[data-demo]").addEventListener("click", () => { form.email.value = DEMO.email; form.password.value = DEMO.password; form.password.focus(); });
  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    showFormError(form, "");
    const { email, password } = formValues(form);
    if (!email.trim() || !password) { showFormError(form, "Vui lòng nhập email và mật khẩu."); return; }
    await withLoading(form.querySelector("button[type=submit]"), async () => {
      try { await afterAuth(await api("/auth/login", { method: "POST", body: { email: email.trim(), password } }), query); toast("Đăng nhập thành công.", "success"); }
      catch (error) { showFormError(form, error.retryAfter ? `${error.message} (thử lại sau ${error.retryAfter} giây)` : error.message); }
    });
  });
}

export function registerView({ main, query, setTitle }) {
  setTitle("Đăng ký");
  if (state.user) { go("/"); return; }
  mount(main, html`
    <div class="wrap auth"><div class="card">
      <h1>Tạo tài khoản</h1>
      <p class="muted">Mất chưa đến một phút.</p>
      <form class="stack" id="register-form" novalidate style="margin-top:1.2rem">
        <div class="field"><label for="name">Họ và tên</label><input class="input" id="name" name="name" required minlength="2" maxlength="80" autocomplete="name"></div>
        <div class="field"><label for="email">Email</label><input class="input" id="email" name="email" type="email" required maxlength="160" autocomplete="email" inputmode="email"></div>
        <div class="field"><label for="phone">Số điện thoại</label><input class="input" id="phone" name="phone" required maxlength="20" autocomplete="tel" inputmode="tel"></div>
        ${passwordField("password", "Mật khẩu", { autocomplete: "new-password", hint: "Ít nhất 8 ký tự, gồm cả chữ và số." })}
        <button class="btn btn-primary btn-lg btn-block" type="submit">Đăng ký</button>
      </form>
      <p style="margin-top:1.2rem">Đã có tài khoản? <a href="#/login${query.next ? `?next=${encodeURIComponent(query.next)}` : ""}">Đăng nhập</a></p>
    </div></div>`);
  const form = main.querySelector("#register-form");
  wirePasswordToggles(main);
  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    showFormError(form, "");
    const values = formValues(form);
    const problems = [];
    if (values.name.trim().length < 2) problems.push("Họ tên cần có ít nhất 2 ký tự.");
    if (!/^[^@\s]+@[^@\s]+\.[^@\s]+$/.test(values.email.trim())) problems.push("Email chưa hợp lệ.");
    if (!/^[0-9+()\-\s]{8,20}$/.test(values.phone)) problems.push("Số điện thoại chưa hợp lệ.");
    if (values.password.length < 8 || !/[A-Za-z]/.test(values.password) || !/[0-9]/.test(values.password)) problems.push("Mật khẩu cần ít nhất 8 ký tự, gồm cả chữ và số.");
    if (problems.length) { showFormError(form, problems.join(" ")); return; }
    await withLoading(form.querySelector("button[type=submit]"), async () => {
      try {
        const session = await api("/auth/register", { method: "POST", body: { name: values.name.trim(), email: values.email.trim(), phone: values.phone.trim(), password: values.password } });
        await afterAuth(session, query);
        toast("Tạo tài khoản thành công. Chào mừng bạn!", "success");
      } catch (error) { showFormError(form, error.message); }
    });
  });
}

export async function accountView({ main, setTitle }) {
  setTitle("Tài khoản của tôi");
  if (!requireLogin()) return;
  const user = state.user;
  mount(main, html`
    <div class="wrap">
      <div class="page-title"><h1>Tài khoản của tôi</h1><p>Thông tin này được điền sẵn khi bạn thanh toán.</p></div>
      <div class="cart-layout">
        <form class="card panel" id="profile-form" novalidate>
          <h2>Thông tin cá nhân</h2>
          <div class="form-grid">
            <div class="field full"><label for="name">Họ và tên</label><input class="input" id="name" name="name" required minlength="2" maxlength="80" value="${user.name}" autocomplete="name"></div>
            <div class="field"><label for="email">Email</label><input class="input" id="email" value="${user.email}" disabled aria-describedby="email-hint"><span class="hint" id="email-hint">Email dùng để đăng nhập nên không thể đổi.</span></div>
            <div class="field"><label for="phone">Số điện thoại</label><input class="input" id="phone" name="phone" required maxlength="20" value="${user.phone}" autocomplete="tel" inputmode="tel"></div>
            <div class="field full"><label for="address">Địa chỉ giao hàng mặc định</label><input class="input" id="address" name="address" maxlength="300" value="${user.address}" autocomplete="street-address"></div>
          </div>
          <div class="row" style="margin-top:1.2rem"><button class="btn btn-primary btn-lg" type="submit">Lưu thay đổi</button></div>
        </form>
        <aside class="card panel stack"><h2>Lối tắt</h2>
          <a class="btn btn-secondary btn-block" href="#/orders">${icon("receipt")}Đơn hàng của tôi</a>
          <a class="btn btn-secondary btn-block" href="#/cart">${icon("cart")}Giỏ hàng</a>
          <button class="btn btn-ghost btn-block" type="button" data-logout>${icon("logout")}Đăng xuất</button>
        </aside>
      </div>
    </div>`);
  const form = main.querySelector("#profile-form");
  form.addEventListener("submit", async (event) => {
    event.preventDefault();
    showFormError(form, "");
    const values = formValues(form);
    if (values.name.trim().length < 2) { showFormError(form, "Họ tên cần có ít nhất 2 ký tự."); return; }
    if (!/^[0-9+()\-\s]{8,20}$/.test(values.phone)) { showFormError(form, "Số điện thoại chưa hợp lệ."); return; }
    await withLoading(form.querySelector("button[type=submit]"), async () => {
      try {
        const updated = await api("/users/me", { method: "PATCH", body: { name: values.name.trim(), phone: values.phone.trim(), address: values.address.trim() } });
        state.user = updated;
        document.dispatchEvent(new CustomEvent("profile-saved"));
        toast("Đã lưu thông tin.", "success");
      } catch (error) { showFormError(form, error.message); }
    });
  });
}
