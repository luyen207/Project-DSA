// Biểu đồ SVG viết tay (không thư viện). Quy tắc: một trục, cột bo góc 4px sát đường nền,
// không ghi số lên mọi cột, có tooltip khi rê chuột/focus, luôn có dạng bảng để thay thế.
import { html, raw, esc, vnd, compactVnd, number, STATUS_LABEL } from "/assets/shared.js";

export const STATUS_COLOR = { pending: "#eda100", confirmed: "#2a78d6", shipping: "#1baf7a", completed: "#008300", cancelled: "#e34948" };
const BAR = "#2a78d6";

function niceMax(maxValue) {
  if (maxValue <= 0) return 4;
  const rough = maxValue / 4;
  const power = 10 ** Math.floor(Math.log10(rough));
  const step = [1, 2, 2.5, 5, 10].map((m) => m * power).find((s) => s >= rough);
  return step * 4;
}

export function barChart({ data, formatValue = number, formatAxis = compactVnd, ariaLabel }) {
  const W = 640, H = 290, L = 56, R = 10, T = 26, B = 38;
  const innerW = W - L - R, innerH = H - T - B;
  const max = niceMax(Math.max(0, ...data.map((d) => d.value)));
  const slot = innerW / Math.max(1, data.length);
  const bw = Math.max(8, Math.min(46, slot * 0.62));
  const every = Math.ceil(data.length / 8);
  const maxIndex = data.reduce((best, d, i) => (d.value > data[best].value ? i : best), 0);
  const ticks = [0, 1, 2, 3, 4].map((i) => (max / 4) * i);
  const y = (value) => T + innerH - (value / max) * innerH;

  const grid = ticks.map((tick) => `<line class="${tick === 0 ? "baseline" : "grid"}" x1="${L}" x2="${W - R}" y1="${y(tick)}" y2="${y(tick)}"/>
    <text class="axis-label" x="${L - 10}" y="${y(tick) + 4}" text-anchor="end">${esc(formatAxis(tick))}</text>`).join("");
  const columns = data.map((d, i) => {
    const x = L + slot * i + (slot - bw) / 2;
    const h = Math.max(0, (d.value / max) * innerH);
    const top = T + innerH - h;
    const r = Math.min(4, h / 2, bw / 2);
    const path = h > 0 ? `<path class="bar" d="M${x},${top + h}V${top + r}Q${x},${top} ${x + r},${top}H${x + bw - r}Q${x + bw},${top} ${x + bw},${top + r}V${top + h}Z"/>` : "";
    const label = i % every === 0 ? `<text class="axis-label" x="${x + bw / 2}" y="${H - 14}" text-anchor="middle">${esc(d.label)}</text>` : "";
    const value = i === maxIndex && d.value > 0 ? `<text class="val-label" x="${x + bw / 2}" y="${top - 7}" text-anchor="middle">${esc(compactVnd(d.value))}</text>` : "";
    return `<g class="col" tabindex="0" role="img" aria-label="${esc(d.label)}: ${esc(formatValue(d.value))}" data-tip-title="${esc(d.tipTitle || d.label)}" data-tip-body="${esc(d.tipBody || formatValue(d.value))}">
      <rect class="bar-hit" x="${L + slot * i}" y="${T}" width="${slot}" height="${innerH + B - 6}"/>${path}${label}${value}</g>`;
  }).join("");
  return raw(`<svg viewBox="0 0 ${W} ${H}" role="group" aria-label="${esc(ariaLabel)}">${grid}${columns}</svg>`);
}

export function statusBars(series) {
  const total = series.reduce((sum, s) => sum + s.value, 0);
  const max = Math.max(1, ...series.map((s) => s.value));
  return html`<div class="h-rows" role="list">${series.map((s) => html`
    <div class="h-row" role="listitem" data-tip-title="${STATUS_LABEL[s.status]}" data-tip-body="${s.value} đơn (${total ? Math.round((s.value / total) * 100) : 0}%)">
      <span class="h-name"><span class="sw" style="background:${STATUS_COLOR[s.status]}"></span>${STATUS_LABEL[s.status]}</span>
      <span class="track" aria-hidden="true"><span class="fill" style="display:block;width:${(s.value / max) * 100}%;background:${STATUS_COLOR[s.status]}"></span></span>
      <span class="h-val">${s.value}</span>
    </div>`)}</div>`;
}

export function rankBars(rows, { valueText }) {
  const max = Math.max(1, ...rows.map((r) => r.value));
  return html`<div class="h-rows" role="list">${rows.map((r) => html`
    <div class="h-row" role="listitem" data-tip-title="${r.name}" data-tip-body="${valueText(r)}">
      <span class="h-name">${r.name}</span>
      <span class="track" aria-hidden="true"><span class="fill" style="display:block;width:${(r.value / max) * 100}%;background:${BAR}"></span></span>
      <span class="h-val">${r.value}</span>
    </div>`)}</div>`;
}

export function dataTable(columns, rows) {
  return html`<div class="table-wrap"><table class="table"><thead><tr>${columns.map((c) => html`<th class="${c.num ? "num" : ""}">${c.label}</th>`)}</tr></thead>
    <tbody>${rows.map((row) => html`<tr>${columns.map((c) => html`<td class="${c.num ? "num" : ""}">${c.render(row)}</td>`)}</tr>`)}</tbody></table></div>`;
}

// Khung có nút chuyển Biểu đồ <-> Bảng
export function chartPanel({ title, sub, chart, table }) {
  return html`<section class="panel-card" data-chart-panel>
    <header><div><h2>${title}</h2>${sub ? html`<p class="sub">${sub}</p>` : ""}</div>
      <button class="btn btn-ghost btn-sm view-toggle" type="button" data-view-toggle aria-pressed="false">Xem dạng bảng</button></header>
    <div class="chart" data-view="chart">${chart}</div>
    <div data-view="table" hidden>${table}</div>
  </section>`;
}

let wired = false;
export function wireChartInteractions() {
  if (wired) return;
  wired = true;
  const tip = document.getElementById("tooltip");
  const show = (target, x, y) => {
    tip.innerHTML = `<b>${esc(target.dataset.tipTitle)}</b>${esc(target.dataset.tipBody)}`;
    tip.hidden = false;
    const box = tip.getBoundingClientRect();
    tip.style.left = `${Math.min(window.innerWidth - box.width - 8, Math.max(8, x + 14))}px`;
    tip.style.top = `${Math.min(window.innerHeight - box.height - 8, Math.max(8, y - box.height - 10))}px`;
  };
  document.addEventListener("mousemove", (event) => {
    const target = event.target.closest?.("[data-tip-title]");
    if (target) show(target, event.clientX, event.clientY); else tip.hidden = true;
  });
  document.addEventListener("focusin", (event) => {
    const target = event.target.closest?.("[data-tip-title]");
    if (!target) return;
    const box = target.getBoundingClientRect();
    show(target, box.left + box.width / 2, box.top);
  });
  document.addEventListener("focusout", () => { tip.hidden = true; });
  document.addEventListener("scroll", () => { tip.hidden = true; }, true);
  document.addEventListener("click", (event) => {
    const button = event.target.closest("[data-view-toggle]");
    if (!button) return;
    const panel = button.closest("[data-chart-panel]");
    const showTable = button.getAttribute("aria-pressed") !== "true";
    button.setAttribute("aria-pressed", String(showTable));
    button.textContent = showTable ? "Xem dạng biểu đồ" : "Xem dạng bảng";
    panel.querySelector('[data-view="chart"]').hidden = showTable;
    panel.querySelector('[data-view="table"]').hidden = !showTable;
  });
}
