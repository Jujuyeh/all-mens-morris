const state = {
  boards: [],
  selected: null,
  tab: "graph",
};

const el = {
  status: document.querySelector("#status"),
  boardSelect: document.querySelector("#boardSelect"),
  newBoardName: document.querySelector("#newBoardName"),
  duplicateBoard: document.querySelector("#duplicateBoard"),
  saveBoard: document.querySelector("#saveBoard"),
  tabs: Array.from(document.querySelectorAll("[data-tab]")),
  summary: document.querySelector("#summary"),
  validation: document.querySelector("#validation"),
  canvas: document.querySelector("#boardCanvas"),
  graphTab: document.querySelector("#graphTab"),
  rulesTab: document.querySelector("#rulesTab"),
  rawTab: document.querySelector("#rawTab"),
  rulesForm: document.querySelector("#rulesForm"),
  rawJson: document.querySelector("#rawJson"),
};

const ctx = el.canvas.getContext("2d");

function setStatus(text) {
  el.status.textContent = text;
}

function clone(value) {
  return JSON.parse(JSON.stringify(value));
}

function selectedBoard() {
  return state.selected ? state.selected.data : null;
}

function boardScale(board) {
  const canvas = board.canvas || { width: 64, height: 64 };
  return {
    x: el.canvas.width / canvas.width,
    y: el.canvas.height / canvas.height,
  };
}

function pointAt(board, index) {
  const point = board.points[index];
  const scale = boardScale(board);
  return {
    x: point.x * scale.x,
    y: point.y * scale.y,
  };
}

function drawLine(board, pair, color, width) {
  if (!Array.isArray(pair) || pair.length < 2) return;
  if (!board.points[pair[0]] || !board.points[pair[1]]) return;
  const a = pointAt(board, pair[0]);
  const b = pointAt(board, pair[1]);
  ctx.strokeStyle = color;
  ctx.lineWidth = width;
  ctx.beginPath();
  ctx.moveTo(a.x, a.y);
  ctx.lineTo(b.x, b.y);
  ctx.stroke();
}

function drawBoard() {
  const board = selectedBoard();
  ctx.fillStyle = "#080808";
  ctx.fillRect(0, 0, el.canvas.width, el.canvas.height);
  if (!board) return;

  (board.adjacency || []).forEach((edges, index) => {
    (edges || []).forEach((to) => {
      if (to > index) drawLine(board, [index, to], "#69b7ff", 2);
    });
  });

  (board.mills || []).forEach((mill) => {
    drawLine(board, [mill[0], mill[1]], "#ffcf5a", 1);
    drawLine(board, [mill[1], mill[2]], "#ffcf5a", 1);
  });

  (board.points || []).forEach((point, index) => {
    const p = pointAt(board, index);
    ctx.fillStyle = "#f7f7f7";
    ctx.beginPath();
    ctx.arc(p.x, p.y, 8, 0, Math.PI * 2);
    ctx.fill();
    ctx.fillStyle = "#050505";
    ctx.font = "14px ui-monospace, monospace";
    ctx.textAlign = "center";
    ctx.textBaseline = "middle";
    ctx.fillText(String(index), p.x, p.y);
  });
}

function validateBoard(board) {
  const issues = [];
  const points = board.points || [];
  const pointCount = points.length;

  if (!pointCount) issues.push("Board needs at least one point.");
  points.forEach((point, index) => {
    if (!Number.isFinite(point.x) || !Number.isFinite(point.y)) {
      issues.push(`Point ${index} needs numeric x/y.`);
    }
  });

  (board.adjacency || []).forEach((edges, from) => {
    if (!Array.isArray(edges)) {
      issues.push(`Adjacency ${from} must be an array.`);
      return;
    }
    const seen = new Set();
    edges.forEach((to) => {
      if (to < 0 || to >= pointCount) issues.push(`Edge ${from}-${to} references a missing point.`);
      if (to === from) issues.push(`Point ${from} cannot be adjacent to itself.`);
      if (seen.has(to)) issues.push(`Point ${from} has duplicate edge ${to}.`);
      seen.add(to);
      const reverse = board.adjacency?.[to] || [];
      if (!reverse.includes(from)) issues.push(`Edge ${from}-${to} is not bidirectional.`);
    });
  });

  (board.mills || []).forEach((mill, index) => {
    if (!Array.isArray(mill) || mill.length !== 3) {
      issues.push(`Mill ${index} must contain exactly 3 points.`);
      return;
    }
    if (new Set(mill).size !== 3) issues.push(`Mill ${index} repeats a point.`);
    mill.forEach((point) => {
      if (point < 0 || point >= pointCount) issues.push(`Mill ${index} references missing point ${point}.`);
    });
  });

  const rules = board.rules || {};
  ["piecesPerPlayer", "minPiecesToContinue", "flyPieceCount"].forEach((key) => {
    if (!Number.isInteger(rules[key]) || rules[key] < 0) issues.push(`Rule ${key} must be a positive integer.`);
  });

  return issues;
}

function renderSummary(board) {
  el.summary.innerHTML = "";
  if (!board) return;
  [
    `id: ${board.id}`,
    `label: ${board.label}`,
    `points: ${(board.points || []).length}`,
    `mills: ${(board.mills || []).length}`,
    `pieces: ${board.rules?.piecesPerPlayer ?? "?"}`,
  ].forEach((text) => {
    const item = document.createElement("div");
    item.textContent = text;
    el.summary.appendChild(item);
  });
}

function renderValidation(board) {
  el.validation.innerHTML = "";
  if (!board) return;
  const issues = validateBoard(board);
  if (!issues.length) {
    const item = document.createElement("div");
    item.className = "ok";
    item.textContent = "Valid board profile.";
    el.validation.appendChild(item);
    return;
  }
  issues.forEach((issue) => {
    const item = document.createElement("div");
    item.className = "bad";
    item.textContent = issue;
    el.validation.appendChild(item);
  });
}

function renderRules(board) {
  el.rulesForm.innerHTML = "";
  if (!board) return;
  const rules = board.rules || {};
  Object.keys(rules).forEach((key) => {
    const label = document.createElement("label");
    label.textContent = key;
    const input = document.createElement("input");
    input.type = typeof rules[key] === "boolean" ? "checkbox" : "number";
    if (input.type === "checkbox") {
      input.checked = Boolean(rules[key]);
    } else {
      input.value = rules[key];
    }
    input.addEventListener("input", () => {
      rules[key] = input.type === "checkbox" ? input.checked : Number(input.value);
      syncRawFromState();
      render();
    });
    label.appendChild(input);
    el.rulesForm.appendChild(label);
  });
}

function syncRawFromState() {
  const board = selectedBoard();
  el.rawJson.value = board ? JSON.stringify(board, null, 2) : "";
}

function syncStateFromRaw() {
  const data = JSON.parse(el.rawJson.value);
  state.selected.data = data;
}

function renderTabs() {
  el.graphTab.classList.toggle("hidden", state.tab !== "graph");
  el.rulesTab.classList.toggle("hidden", state.tab !== "rules");
  el.rawTab.classList.toggle("hidden", state.tab !== "raw");
  el.tabs.forEach((tab) => tab.classList.toggle("active", tab.dataset.tab === state.tab));
}

function render() {
  const board = selectedBoard();
  renderTabs();
  renderSummary(board);
  renderValidation(board);
  renderRules(board);
  drawBoard();
}

function selectBoard(slug) {
  state.selected = state.boards.find((board) => board.slug === slug) || state.boards[0] || null;
  if (state.selected) el.boardSelect.value = state.selected.slug;
  syncRawFromState();
  render();
}

function populateSelect() {
  el.boardSelect.innerHTML = "";
  state.boards.forEach((board) => {
    const option = document.createElement("option");
    option.value = board.slug;
    option.textContent = board.data?.name || board.name || board.slug;
    el.boardSelect.appendChild(option);
  });
}

async function loadBoards() {
  setStatus("Loading boards");
  const response = await fetch("/api/boards");
  const payload = await response.json();
  state.boards = payload.boards || [];
  populateSelect();
  selectBoard(state.boards[0]?.slug);
  setStatus(`${payload.count} board profile(s)`);
}

async function saveBoard() {
  if (!state.selected) return;
  if (state.tab === "raw") syncStateFromRaw();
  const response = await fetch("/api/save-board", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ slug: state.selected.slug, data: state.selected.data }),
  });
  const payload = await response.json();
  if (!payload.ok) throw new Error(payload.error || "Save failed");
  state.selected.slug = payload.slug;
  state.selected.path = payload.path;
  state.selected.data = payload.data;
  populateSelect();
  selectBoard(payload.slug);
  setStatus(`Saved ${payload.path}`);
}

function duplicateBoard() {
  if (!state.selected) return;
  const name = el.newBoardName.value.trim() || `${state.selected.data.name} Copy`;
  const data = clone(state.selected.data);
  data.name = name;
  data.id = name.toLowerCase().replace(/[^a-z0-9_-]+/g, "-").replace(/^-|-$/g, "") || "board";
  data.label = name.toUpperCase().slice(0, 12);
  const board = { slug: data.id, path: `boards/${data.id}.json`, data };
  state.boards.push(board);
  populateSelect();
  selectBoard(board.slug);
  setStatus("Duplicated board draft");
}

el.boardSelect.addEventListener("change", () => selectBoard(el.boardSelect.value));
el.duplicateBoard.addEventListener("click", duplicateBoard);
el.saveBoard.addEventListener("click", () => saveBoard().catch((error) => setStatus(error.message)));
el.rawJson.addEventListener("input", () => {
  try {
    syncStateFromRaw();
    render();
    setStatus("Raw JSON parsed");
  } catch (error) {
    setStatus(error.message);
  }
});
el.tabs.forEach((tab) => {
  tab.addEventListener("click", () => {
    state.tab = tab.dataset.tab;
    if (state.tab === "raw") syncRawFromState();
    render();
  });
});

loadBoards().catch((error) => setStatus(error.message));
