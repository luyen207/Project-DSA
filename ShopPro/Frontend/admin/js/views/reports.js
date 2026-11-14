import { html, icon, mount, safeSrc, vnd, number, toast, isoDate, STATUS_LABEL, withLoading } from "/assets/shared.js";
import { api, loadingBlock, errorBlock } from "../core.js";
import { barChart, statusBars, rankBars, chartPanel, dataTable } from "../charts.js";

const PRESETS = [["7", "7 ngày"], ["30", "30 ngày"], ["90", "90 ngày"], ["month", "Tháng này"]];

function rangeFor(preset) {
  const to = new Date();
  const from = new Date();
  if (preset === "month") from.setDate(1); else from.setDate(from.getDate() - (Number(preset) - 1));
  return { from: isoDate(from), to: isoDate(to) };
}
const vnDate = (iso) => iso.split("-").reverse().join("/");
const csvCell = (value) => {
  let text = String(value ?? "");
  if (/^[=+\-@\t\r]/.test(text)) text = `'${text}`;           // chống chèn công thức khi mở bằng Excel
  return /[",\n\r]/.test(text) ? `"${text.replace(/"/g, '""')}"` : text;
};
function downloadCsv(report) {
  const lines = [];
  const add = (...cells) => lines.push(cells.map(csvCell).join(","));
  add("Báo cáo ShopPro", `${report.from} đến ${report.to}`);
  add("Doanh thu (đơn hoàn thành, sau giảm giá)", report.revenue); add("Tiền giảm giá bằng mã (đơn hoàn thành)", report.discountTotal); add("Số đơn dùng mã giảm giá", report.couponOrders); add("Số đơn (không tính đã hủy)", report.orderCount); add("Đơn hoàn thành", report.completedCount);
  add("Giá trị đơn trung bình", report.averageOrderValue); add("Số khách mua hàng", report.customerCount);
  lines.push(""); add("Doanh thu theo ngày"); add("Ngày", "Số đơn", "Doanh thu");
  report.revenueSeries.forEach((d) => add(d.date, d.orders, d.revenue));
  lines.push(""); add("Doanh thu theo danh mục"); add("Danh mục", "Số lượng bán", "Doanh thu");
  report.categorySeries.forEach((c) => add(c.name, c.value, c.revenue));
  lines.push(""); add("Trạng thái đơn"); add("Trạng thái", "Số đơn");
  report.statusSeries.forEach((s) => add(STATUS_LABEL[s.status], s.value));
  lines.push(""); add("Sản phẩm bán chạy"); add("Sản phẩm", "Số lượng", "Doanh thu");
  report.topProducts.forEach((p) => add(p.name, p.quantity, p.revenue));
  lines.push(""); add("Khách chi tiêu nhiều"); add("Khách hàng", "Chi tiêu");
  report.topCustomers.forEach((c) => add(c.name, c.spent));
  const blob = new Blob(["﻿" + lines.join("\r\n")], { type: "text/csv;charset=utf-8" });
  const link = document.createElement("a");
  link.href = URL.createObjectURL(blob);
  link.download = `bao-cao-shoppro_${report.from}_${report.to}.csv`;
  document.body.append(link);
  link.click();
  link.remove();
  setTimeout(() => URL.revokeObjectURL(link.href), 1000);
}

export async function reportsView({ main, isCurrent }) {
  const ui = { preset: "30", ...rangeFor("30") };
  let report = null;

  mount(main, html`
    <div class="page-head"><div><h1>Báo cáo</h1><p>Doanh thu chỉ tính các đơn đã hoàn thành. Số đơn không gồm đơn đã hủy.</p></div><button class="btn btn-secondary" type="button" data-export disabled>${icon("download")}Xuất CSV</button></div>
    <form class="toolbar-row" id="range-form" novalidate>
      <div class="tabs" role="group" aria-label="Khoảng thời gian nhanh">${PRESETS.map(([key, label]) => html`<button class="tab" type="button" data-preset="${key}" aria-selected="${String(ui.preset === key)}">${label}</button>`)}</div>
      <label class="sr-only" for="from">Từ ngày</label><input class="input w" id="from" name="from" type="date" value="${ui.from}" max="${isoDate(new Date())}">
      <span class="muted" aria-hidden="true">đến</span>
      <label class="sr-only" for="to">Đến ngày</label><input class="input w" id="to" name="to" type="date" value="${ui.to}" max="${isoDate(new Date())}">
      <button class="btn btn-primary" type="submit">Xem báo cáo</button>
    </form>
    <div id="report">${loadingBlock}</div>`);

  const form = main.querySelector("#range-form");
  async function run() {
    mount(main.querySelector("#report"), loadingBlock);
    try {
      const data = await api(`/admin/reports/summary?from=${ui.from}&to=${ui.to}`);
      if (!isCurrent()) return;
      report = data;
      main.querySelector("[data-export]").disabled = false;
      paint();
    } catch (error) { if (isCurrent()) mount(main.querySelector("#report"), errorBlock(error.message)); }
  }
  function paint() {
    const r = report;
    const days = r.revenueSeries.map((d) => ({ label: d.label, value: d.revenue, tipTitle: `Ngày ${d.label}`, tipBody: `${vnd(d.revenue)} · ${d.orders} đơn` }));
    mount(main.querySelector("#report"), html`
      <div class="kpis">
        <div class="kpi"><span class="k-label">${icon("money")}Doanh thu</span><span class="k-value">${vnd(r.revenue)}</span><span class="k-sub">${r.completedCount} đơn hoàn thành · đã giảm ${vnd(r.discountTotal)}</span></div>
        <div class="kpi"><span class="k-label">${icon("receipt")}Số đơn</span><span class="k-value">${number(r.orderCount)}</span><span class="k-sub">Không tính đơn đã hủy</span></div>
        <div class="kpi"><span class="k-label">${icon("tag")}Giá trị đơn trung bình</span><span class="k-value">${vnd(r.averageOrderValue)}</span><span class="k-sub">Trên đơn hoàn thành</span></div>
        <div class="kpi"><span class="k-label">${icon("users")}Khách mua hàng</span><span class="k-value">${r.customerCount}</span><span class="k-sub">${r.productCount} sản phẩm đã bán</span></div>
      </div>
      <div class="grid-2">
        ${chartPanel({ title: "Doanh thu theo ngày", sub: `${vnDate(r.from)} đến ${vnDate(r.to)}`, chart: barChart({ data: days, ariaLabel: "Biểu đồ cột doanh thu theo ngày" }),
          table: dataTable([{ label: "Ngày", render: (d) => d.date }, { label: "Số đơn", num: true, render: (d) => d.orders }, { label: "Doanh thu", num: true, render: (d) => vnd(d.revenue) }], [...r.revenueSeries].reverse()) })}
        <section class="panel-card"><header><div><h2>Trạng thái đơn</h2><p class="sub">Trong khoảng đã chọn</p></div></header>${statusBars(r.statusSeries)}</section>
      </div>
      <div class="grid-2e">
        <section class="panel-card"><header><div><h2>Số lượng bán theo danh mục</h2><p class="sub">Chưa tính đơn đã hủy</p></div></header>
          ${r.categorySeries.length ? rankBars([...r.categorySeries].sort((a, b) => b.value - a.value), { valueText: (c) => `${c.value} sản phẩm · ${vnd(c.revenue)} đã ghi nhận` }) : html`<p class="muted">Chưa có dữ liệu.</p>`}</section>
        <section class="panel-card"><header><h2>Khách chi tiêu nhiều nhất</h2></header>
          ${r.topCustomers.length ? html`<div class="list-plain">${r.topCustomers.map((c, i) => html`<div class="link-row"><div class="row" style="flex-wrap:nowrap"><span class="rank">${i + 1}</span><b style="color:var(--ink);font-weight:600">${c.name}</b></div><b class="num">${vnd(c.spent)}</b></div>`)}</div>` : html`<p class="muted">Chưa có dữ liệu.</p>`}</section>
      </div>
      <section class="panel-card"><header><h2>Sản phẩm bán chạy</h2></header>
        ${r.topProducts.length ? dataTable([
          { label: "#", render: (p) => p._i + 1 },
          { label: "Sản phẩm", render: (p) => html`<div class="cell-prod" style="min-width:0"><img class="thumb" src="${safeSrc(p.image)}" alt="" width="52" height="52"><b>${p.name}</b></div>` },
          { label: "Đã bán", num: true, render: (p) => p.quantity }, { label: "Doanh thu", num: true, render: (p) => vnd(p.revenue) },
        ], r.topProducts.map((p, i) => ({ ...p, _i: i }))) : html`<p class="muted">Chưa có dữ liệu.</p>`}</section>`);
  }

  form.addEventListener("click", (event) => {
    const preset = event.target.closest("[data-preset]");
    if (!preset) return;
    ui.preset = preset.dataset.preset;
    Object.assign(ui, rangeFor(ui.preset));
    form.from.value = ui.from; form.to.value = ui.to;
    form.querySelectorAll("[data-preset]").forEach((b) => b.setAttribute("aria-selected", String(b === preset)));
    run();
  });
  form.addEventListener("submit", (event) => {
    event.preventDefault();
    if (!form.from.value || !form.to.value) { toast("Hãy chọn đủ ngày bắt đầu và kết thúc.", "error"); return; }
    if (form.from.value > form.to.value) { toast("Ngày bắt đầu phải trước hoặc bằng ngày kết thúc.", "error"); return; }
    ui.from = form.from.value; ui.to = form.to.value; ui.preset = "";
    form.querySelectorAll("[data-preset]").forEach((b) => b.setAttribute("aria-selected", "false"));
    run();
  });
  main.querySelector("[data-export]").addEventListener("click", () => { if (report) downloadCsv(report); });
  await run();
}
