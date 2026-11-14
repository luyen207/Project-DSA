import { html, icon, mount, safeSrc, vnd, number, dateTime, statusBadge, timeAgo, emptyState } from "/assets/shared.js";
import { api, loadingBlock, errorBlock } from "../core.js";
import { barChart, statusBars, chartPanel, dataTable } from "../charts.js";

export async function dashboardView({ main, isCurrent }) {
  mount(main, html`<div class="page-head"><div><h1>Tổng quan</h1><p>Tình hình cửa hàng 14 ngày gần nhất.</p></div></div>${loadingBlock}`);
  let d;
  try { d = await api("/admin/dashboard"); }
  catch (error) { if (isCurrent()) mount(main, html`<div class="page-head"><h1>Tổng quan</h1></div>${errorBlock(error.message)}`); return; }
  if (!isCurrent()) return;

  const series = d.revenueSeries.map((day) => ({ label: day.label, value: day.revenue, tipTitle: `Ngày ${day.label}`, tipBody: `${vnd(day.revenue)} · ${day.orders} đơn` }));
  const total14 = d.revenueSeries.reduce((sum, day) => sum + day.revenue, 0);
  const orders14 = d.revenueSeries.reduce((sum, day) => sum + day.orders, 0);

  mount(main, html`
    <div class="page-head"><div><h1>Tổng quan</h1><p>Tình hình cửa hàng 14 ngày gần nhất.</p></div><a class="btn btn-secondary" href="#/reports">${icon("download")}Xem báo cáo chi tiết</a></div>
    <div class="kpis">
      <a class="kpi" href="#/reports"><span class="k-label">${icon("money")}Doanh thu</span><span class="k-value">${vnd(d.revenue)}</span><span class="k-sub">Hôm nay: ${vnd(d.todayRevenue)}</span></a>
      <a class="kpi" href="#/orders"><span class="k-label">${icon("receipt")}Đơn hôm nay</span><span class="k-value">${d.todayOrders}</span><span class="k-sub">Tổng ${number(d.orderCount)} đơn trong hệ thống</span></a>
      <a class="kpi ${d.pendingOrders ? "warn" : ""}" href="#/orders?status=pending"><span class="k-label">${icon("clock")}Chờ xác nhận</span><span class="k-value">${d.pendingOrders}</span><span class="k-sub">${d.pendingOrders ? "Cần xử lý sớm" : "Không có đơn tồn đọng"}</span></a>
      <a class="kpi ${d.lowStockCount || d.outOfStockCount ? "warn" : ""}" href="#/products?stock=low"><span class="k-label">${icon("warehouse")}Cảnh báo tồn kho</span><span class="k-value">${d.lowStockCount}</span><span class="k-sub">Sắp hết (≤ ${d.lowStockThreshold}) · ${d.outOfStockCount} hết hàng · ${d.productCount} sản phẩm</span></a>
    </div>
    <div class="grid-2">
      ${chartPanel({
        title: "Doanh thu theo ngày", sub: `${vnd(total14)} từ ${orders14} đơn (không tính đơn đã hủy)`,
        chart: barChart({ data: series, ariaLabel: "Biểu đồ cột doanh thu 14 ngày gần nhất" }),
        table: dataTable([{ label: "Ngày", render: (r) => r.date }, { label: "Số đơn", num: true, render: (r) => r.orders }, { label: "Doanh thu", num: true, render: (r) => vnd(r.revenue) }], [...d.revenueSeries].reverse()),
      })}
      <section class="panel-card"><header><div><h2>Trạng thái đơn hàng</h2><p class="sub">Toàn bộ đơn trong hệ thống</p></div></header>${statusBars(d.statusSeries)}</section>
    </div>
    <div class="grid-2">
      <section class="panel-card"><header><div><h2>Đơn hàng mới nhất</h2></div><a class="btn btn-ghost btn-sm" href="#/orders">Xem tất cả</a></header>
        <div class="table-wrap"><table class="table compact"><thead><tr><th>Mã đơn</th><th>Khách hàng</th><th class="num">Tổng tiền</th><th>Trạng thái</th></tr></thead>
          <tbody>${d.recentOrders.map((o) => html`<tr><td class="id"><a href="#/orders?open=${encodeURIComponent(o.id)}">${o.id}</a><span class="sub-line">${timeAgo(o.createdAt)}</span></td><td>${o.customerName}</td><td class="num" style="white-space:nowrap">${vnd(o.total)}</td><td>${statusBadge(o.status)}</td></tr>`)}</tbody></table></div>
      </section>
      <div class="stack">
        <section class="panel-card"><header><h2>Bán chạy nhất</h2></header>
          ${d.bestSellers.length ? html`<div class="list-plain">${d.bestSellers.map((p, i) => html`<div class="link-row"><div class="row" style="flex-wrap:nowrap;min-width:0"><span class="rank">${i + 1}</span><img class="thumb" src="${safeSrc(p.image)}" alt="" width="52" height="52" style="width:44px;height:44px"><b style="color:var(--ink);font-weight:600;line-height:1.3">${p.name}</b></div><div style="text-align:right;white-space:nowrap"><b>${p.quantity}</b> <span class="muted">đã bán</span></div></div>`)}</div>` : emptyState({ title: "Chưa có dữ liệu bán hàng" })}
        </section>
        <section class="panel-card"><header><h2>Sắp hết hàng</h2><a class="btn btn-ghost btn-sm" href="#/products?stock=low">Nhập kho</a></header>
          ${d.lowStock.length ? html`<div class="list-plain">${d.lowStock.map((p) => html`<div class="link-row"><div class="row" style="flex-wrap:nowrap;min-width:0"><img class="thumb" src="${safeSrc(p.image)}" alt="" width="44" height="44" style="width:44px;height:44px"><a href="#/products?q=${encodeURIComponent(p.name)}" style="line-height:1.3">${p.name}</a></div><span class="stock-pill ${p.stock === 0 ? "out" : "low"}">${icon(p.stock === 0 ? "x" : "alert")}${p.stock}</span></div>`)}</div>` : html`<p class="muted">Tất cả sản phẩm đều đủ hàng.</p>`}
        </section>
      </div>
    </div>`);
}
