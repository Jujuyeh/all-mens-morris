const state = {
  boards: [],
  selected: null,
  tab: "graph",
  tool: "move",
  selectedPoint: null,
  dragPoint: null,
  edgeStart: null,
  millDraft: [],
};

const el = {
  status: document.querySelector("#status"),
  boardSelect: document.querySelector("#boardSelect"),
  newBoardName: document.querySelector("#newBoardName"),
  createBoard: document.querySelector("#createBoard"),
  duplicateBoard: document.querySelector("#duplicateBoard"),
  saveBoard: document.querySelector("#saveBoard"),
  tabs: Array.from(document.querySelectorAll("[data-tab]")),
  tools: Array.from(document.querySelectorAll("[data-tool]")),
  showAdjacency: document.querySelector("#showAdjacency"),
  showMills: document.querySelector("#showMills"),
  cursorInfo: document.querySelector("#cursorInfo"),
  selectionInfo: document.querySelector("#selectionInfo"),
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

function canvasToBoard(event) {
  const board = selectedBoard();
  const rect = el.canvas.getBoundingClientRect();
  const canvas = board?.canvas || { width: 64, height: 64 };
  return {
    x: Math.round(((event.clientX - rect.left) / rect.width) * canvas.width),
    y: Math.round(((event.clientY - rect.top) / rect.height) * canvas.height),
  };
}

function hitPoint(event) {
  const board = selectedBoard();
  if (!board) return null;
  const rect = el.canvas.getBoundingClientRect();
  const x = ((event.clientX - rect.left) / rect.width) * el.canvas.width;
  const y = ((event.clientY - rect.top) / rect.height) * el.canvas.height;
  let best = null;
  let bestDistance = 18;
  (board.points || []).forEach((point, index) => {
    const p = pointAt(board, index);
    const distance = Math.hypot(p.x - x, p.y - y);
    if (distance < bestDistance) {
      best = index;
      bestDistance = distance;
    }
  });
  return best;
}

function drawLine(board, pair, color, width, dash = []) {
  if (!Array.isArray(pair) || pair.length < 2) return;
  if (!board.points[pair[0]] || !board.points[pair[1]]) return;
  const a = pointAt(board, pair[0]);
  const b = pointAt(board, pair[1]);
  ctx.strokeStyle = color;
  ctx.lineWidth = width;
  ctx.setLineDash(dash);
  ctx.beginPath();
  ctx.moveTo(a.x, a.y);
  ctx.lineTo(b.x, b.y);
  ctx.stroke();
  ctx.setLineDash([]);
}

function drawBoardConnection(board, pair) {
  drawLine(board, pair, "#050505", 3, [8, 5]);
  drawLine(board, pair, "#f7f7f7", 1.5, [8, 5]);
}

function millColor(index) {
  const hue = (index * 137.508) % 360;
  const lightness = index % 2 === 0 ? 62 : 72;
  return `hsl(${hue.toFixed(1)} 92% ${lightness}%)`;
}

function millSegments(mill) {
  return [
    [mill[0], mill[1]],
    [mill[1], mill[2]],
  ];
}

function sameSegment(left, right) {
  return left.length >= 2
    && right.length >= 2
    && ((left[0] === right[0] && left[1] === right[1])
      || (left[0] === right[1] && left[1] === right[0]));
}

function millSegmentOverlap(board, pair, millIndex) {
  const overlaps = [];
  (board.mills || []).forEach((mill, index) => {
    if (millSegments(mill).some((segment) => sameSegment(segment, pair))) {
      overlaps.push(index);
    }
  });
  return {
    count: overlaps.length,
    rank: overlaps.indexOf(millIndex),
  };
}

function drawMillSegment(board, pair, millIndex) {
  const overlap = millSegmentOverlap(board, pair, millIndex);
  const width = overlap.count > 1 ? 10 + (overlap.count - overlap.rank) * 4 : 10;
  drawLine(board, pair, millColor(millIndex), width);
}

function ensureGraphArrays(board) {
  board.points ||= [];
  board.adjacency ||= [];
  board.mills ||= [];
  while (board.adjacency.length < board.points.length) {
    board.adjacency.push([]);
  }
}

function edgeExists(board, a, b) {
  return (board.adjacency?.[a] || []).includes(b);
}

function toggleEdge(board, a, b) {
  ensureGraphArrays(board);
  if (a === b) return;
  [a, b].forEach((index) => {
    board.adjacency[index] ||= [];
  });
  if (edgeExists(board, a, b)) {
    board.adjacency[a] = board.adjacency[a].filter((value) => value !== b);
    board.adjacency[b] = board.adjacency[b].filter((value) => value !== a);
  } else {
    board.adjacency[a].push(b);
    board.adjacency[b].push(a);
    board.adjacency[a].sort((left, right) => left - right);
    board.adjacency[b].sort((left, right) => left - right);
  }
}

function normalizeMill(points) {
  return [...points].sort((a, b) => a - b);
}

function sameMill(left, right) {
  const a = normalizeMill(left);
  const b = normalizeMill(right);
  return a.length === 3 && b.length === 3 && a.every((value, index) => value === b[index]);
}

function toggleMill(board, points) {
  ensureGraphArrays(board);
  if (new Set(points).size !== 3) return;
  const existing = board.mills.findIndex((mill) => sameMill(mill, points));
  if (existing >= 0) {
    board.mills.splice(existing, 1);
  } else {
    board.mills.push([...points]);
  }
}

function removePoint(board, index) {
  ensureGraphArrays(board);
  board.points.splice(index, 1);
  board.adjacency.splice(index, 1);
  board.adjacency = board.adjacency.map((edges) =>
    (edges || [])
      .filter((to) => to !== index)
      .map((to) => (to > index ? to - 1 : to))
  );
  board.mills = (board.mills || [])
    .filter((mill) => !mill.includes(index))
    .map((mill) => mill.map((point) => (point > index ? point - 1 : point)));
}

function drawBoard() {
  const board = selectedBoard();
  ctx.fillStyle = "#080808";
  ctx.fillRect(0, 0, el.canvas.width, el.canvas.height);
  if (!board) return;

  if (el.showMills.checked) {
    ctx.globalAlpha = 0.72;
    (board.mills || []).forEach((mill, index) => {
      drawMillSegment(board, [mill[0], mill[1]], index);
      drawMillSegment(board, [mill[1], mill[2]], index);
    });
    ctx.globalAlpha = 1;
  }

  if (el.showAdjacency.checked) {
    (board.adjacency || []).forEach((edges, index) => {
      (edges || []).forEach((to) => {
        if (to > index) drawBoardConnection(board, [index, to]);
      });
    });
  }

  (board.points || []).forEach((point, index) => {
    const p = pointAt(board, index);
    const selected = index === state.selectedPoint
      || state.millDraft.includes(index)
      || index === state.edgeStart;
    ctx.fillStyle = selected ? "#7ee787" : "#f7f7f7";
    ctx.beginPath();
    ctx.arc(p.x, p.y, selected ? 11 : 8, 0, Math.PI * 2);
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

function renderSelection() {
  if (state.tool === "edge" && state.edgeStart !== null) {
    el.selectionInfo.textContent = `Connection start: ${state.edgeStart}`;
  } else if (state.tool === "mill" && state.millDraft.length) {
    el.selectionInfo.textContent = `Mill: ${state.millDraft.join(", ")}`;
  } else if (state.selectedPoint !== null) {
    el.selectionInfo.textContent = `Point ${state.selectedPoint}`;
  } else {
    el.selectionInfo.textContent = `${state.tool} mode`;
  }
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
  renderSelection();
  drawBoard();
}

function makeBlankBoard(name) {
  const id = name.toLowerCase().replace(/[^a-z0-9_-]+/g, "-").replace(/^-|-$/g, "") || "board";
  return {
    schemaVersion: 1,
    id,
    name,
    label: name.toUpperCase().slice(0, 12),
    family: "custom-morris",
    canvas: { width: 64, height: 64 },
    rules: {
      piecesPerPlayer: 6,
      minPiecesToContinue: 3,
      flyPieceCount: 3,
      noCaptureDrawTurnLimit: 50,
      flyingEnabled: true,
      protectPiecesInMills: true,
      blockWinEnabled: true,
      materialWinEnabled: true,
    },
    points: [],
    mills: [],
    adjacency: [],
    assets: {
      boardSprite: null,
      banner: "assets/fx/banner.png",
    },
  };
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

function createBoard() {
  const name = el.newBoardName.value.trim() || "New Board";
  const data = makeBlankBoard(name);
  const board = { slug: data.id, path: `boards/${data.id}.json`, data };
  state.boards.push(board);
  populateSelect();
  selectBoard(board.slug);
  setStatus("Created blank board draft");
}

function syncEditedBoard() {
  syncRawFromState();
  render();
}

function handleCanvasDown(event) {
  const board = selectedBoard();
  if (!board) return;
  ensureGraphArrays(board);
  const hit = hitPoint(event);
  if (state.tool === "point") {
    const point = canvasToBoard(event);
    board.points.push(point);
    board.adjacency.push([]);
    state.selectedPoint = board.points.length - 1;
    setStatus(`Added point ${state.selectedPoint}`);
    syncEditedBoard();
    return;
  }
  if (hit === null) return;
  state.selectedPoint = hit;
  if (state.tool === "move") {
    state.dragPoint = hit;
  } else if (state.tool === "edge") {
    if (state.edgeStart === null) {
      state.edgeStart = hit;
      setStatus(`Selected connection start ${hit}`);
    } else {
      toggleEdge(board, state.edgeStart, hit);
      setStatus(`Toggled connection ${state.edgeStart}-${hit}`);
      state.edgeStart = null;
      syncEditedBoard();
    }
  } else if (state.tool === "mill") {
    if (!state.millDraft.includes(hit)) state.millDraft.push(hit);
    if (state.millDraft.length === 3) {
      toggleMill(board, state.millDraft);
      setStatus(`Toggled mill ${state.millDraft.join("-")}`);
      state.millDraft = [];
      syncEditedBoard();
    } else {
      render();
    }
  } else if (state.tool === "erase") {
    removePoint(board, hit);
    state.selectedPoint = null;
    setStatus(`Removed point ${hit}`);
    syncEditedBoard();
  }
}

function handleCanvasMove(event) {
  const board = selectedBoard();
  if (!board) return;
  const point = canvasToBoard(event);
  el.cursorInfo.textContent = `x ${point.x} y ${point.y}`;
  if (state.dragPoint === null) return;
  board.points[state.dragPoint] = point;
  syncRawFromState();
  render();
}

function handleCanvasLeave() {
  el.cursorInfo.textContent = "x -- y --";
  handleCanvasUp();
}

function handleCanvasUp() {
  if (state.dragPoint !== null) {
    setStatus(`Moved point ${state.dragPoint}`);
  }
  state.dragPoint = null;
}

el.boardSelect.addEventListener("change", () => selectBoard(el.boardSelect.value));
el.createBoard.addEventListener("click", createBoard);
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
el.tools.forEach((tool) => {
  tool.addEventListener("click", () => {
    state.tool = tool.dataset.tool;
    state.edgeStart = null;
    state.millDraft = [];
    el.tools.forEach((button) => button.classList.toggle("active", button === tool));
    render();
  });
});
["change", "input"].forEach((eventName) => {
  [el.showAdjacency, el.showMills].forEach((input) => {
    input.addEventListener(eventName, render);
  });
});
el.canvas.addEventListener("mousedown", handleCanvasDown);
el.canvas.addEventListener("mousemove", handleCanvasMove);
el.canvas.addEventListener("mouseleave", handleCanvasLeave);
window.addEventListener("mouseup", handleCanvasUp);

loadBoards().catch((error) => setStatus(error.message));
