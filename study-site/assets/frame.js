/* Frame Analysis 페이지 전용 — 타임라인 필터와 "현재 파이프라인 상태" 패널
   상태 데이터는 아래 INITIAL과 CHANGES가 원본이다. 각 값의 근거는 frame.html의 단계 카드에 있다.
   단계 id(s01 …)는 frame.html의 li.tl-step id와 같아야 한다. */
(function () {
  "use strict";

  var SLOTS = [
    { id: "ia.layout", stage: "IA", label: "InputLayout" },
    { id: "ia.vb", stage: "IA", label: "VB 0" },
    { id: "ia.ib", stage: "IA", label: "IB" },
    { id: "ia.topo", stage: "IA", label: "Topology" },
    { id: "vs.shader", stage: "VS", label: "Shader" },
    { id: "vs.b0", stage: "VS", label: "b0" },
    { id: "vs.b1", stage: "VS", label: "b1" },
    { id: "rs.state", stage: "RS", label: "State" },
    { id: "rs.viewport", stage: "RS", label: "Viewport" },
    { id: "ps.shader", stage: "PS", label: "Shader" },
    { id: "ps.b0", stage: "PS", label: "b0" },
    { id: "ps.b1", stage: "PS", label: "b1" },
    { id: "ps.b2", stage: "PS", label: "b2" },
    { id: "ps.t0", stage: "PS", label: "t0" },
    { id: "ps.t1", stage: "PS", label: "t1" },
    { id: "ps.s0", stage: "PS", label: "s0" },
    { id: "om.rtv", stage: "OM", label: "RTV" },
    { id: "om.dsv", stage: "OM", label: "DSV" },
    { id: "om.dss", stage: "OM", label: "DepthStencil" },
    { id: "om.blend", stage: "OM", label: "Blend" }
  ];

  // 두 번째 프레임부터, 프레임이 시작될 때 파이프라인에 남아 있는 상태.
  // 직전 프레임의 톤 매핑 패스가 마지막으로 건 것 + ImGui가 백업·복원한 것.
  var INITIAL = {
    "ia.layout": "null",
    "ia.vb": "greybox VB (직전 프레임)",
    "ia.ib": "greybox IB (직전 프레임)",
    "ia.topo": "TRIANGLELIST",
    "vs.shader": "fullscreenVertexShader",
    "vs.b0": "Object CB",
    "vs.b1": "Camera CB",
    "rs.state": "CULL_NONE · SOLID",
    "rs.viewport": "화면 전체",
    "ps.shader": "toneMappingPixelShader (Final 기준)",
    "ps.b0": "PostProcess CB",
    "ps.b1": "Camera CB",
    "ps.b2": "Material CB",
    "ps.t0": "null",
    "ps.t1": "null",
    "ps.s0": "WRAP",
    "om.rtv": "백버퍼 (코드가 마지막으로 건 값)",
    "om.dsv": "null",
    "om.dss": "LESS · write ALL",
    "om.blend": "기본값 (설정 안 함)"
  };

  // set: 이 단계가 바인딩을 바꾼 슬롯 / touch: 바인딩은 그대로이고 내용만 갱신되거나 기록된 슬롯
  var CHANGES = {
    s04: { set: { "rs.viewport": "화면 전체 (W × H)" } },
    s08: {
      set: { "om.rtv": "HDR 씬 타깃", "om.dsv": "깊이 D24S8", "om.dss": "LESS · write ALL" },
      note: "HDR 타깃과 깊이 버퍼가 Clear된다."
    },
    s09: {
      set: { "ps.b0": "Light CB" },
      touch: ["vs.b1", "ps.b1"],
      note: "Camera · Light · PostProcess 버퍼 내용이 이번 프레임 값으로 바뀐다."
    },
    s11: { touch: ["vs.b0", "ps.b2"], note: "Object · Material 버퍼 내용이 이 오브젝트 값으로 바뀐다." },
    s12: {
      set: {
        "ia.layout": "씬 InputLayout",
        "ia.vb": "greybox VB · stride 44",
        "ia.ib": "greybox IB · R32_UINT",
        "ia.topo": "TRIANGLELIST"
      }
    },
    s13: { set: { "vs.shader": "simpleVertexShader", "vs.b0": "Object CB", "vs.b1": "Camera CB" } },
    s14: {
      set: {
        "ps.shader": "simplePixelShader",
        "ps.t0": "Albedo SRV (재질)",
        "ps.b1": "Camera CB",
        "ps.b2": "Material CB",
        "ps.s0": "WRAP"
      }
    },
    s15: { touch: ["om.rtv", "om.dsv"], note: "상태를 바꾸지 않고 소비한다. HDR 타깃과 깊이 버퍼에 기록된다." },
    s16: { set: { "rs.viewport": "Bloom (화면 ½)" } },
    s17: {
      set: {
        "om.rtv": "Bright",
        "om.dsv": "null",
        "ia.layout": "null",
        "vs.shader": "fullscreenVertexShader",
        "ps.shader": "brightPassPixelShader",
        "ps.b0": "PostProcess CB",
        "ps.t0": "null (Draw 뒤 해제)",
        "ps.t1": "null",
        "ps.s0": "WRAP"
      },
      note: "Draw 동안 t0 = HDR SRV였다가 해제된다. VB · IB · VS b0/b1은 씬 패스 것이 남아 있지만 쓰이지 않는다."
    },
    s18: { set: { "om.rtv": "BlurY (마지막 회차)", "ps.shader": "blurYPixelShader", "ps.s0": "CLAMP" } },
    s19: { set: { "rs.viewport": "화면 전체 (복원)" } },
    s20: {
      set: { "om.rtv": "백버퍼", "ps.shader": "toneMappingPixelShader", "ps.s0": "WRAP" },
      note: "Draw 동안 t0 = HDR, t1 = BlurY였다가 해제된다."
    },
    s21: { touch: ["om.rtv"], note: "ImGui가 상태를 바꿨다가 백업한 값으로 복원한다. 순변화 없음. 백버퍼에 UI가 기록된다." },
    s22: { note: "상태를 바꾸지 않는다. 다음 프레임은 이 상태에서 시작한다." }
  };

  function isEmpty(value) {
    return !value || value.indexOf("null") === 0;
  }

  document.addEventListener("DOMContentLoaded", function () {
    var panel = document.getElementById("tl-state");
    var steps = Array.prototype.slice.call(document.querySelectorAll(".tl-step"));
    if (!panel || steps.length === 0) {
      return;
    }

    // ---------- 패널 뼈대 ----------
    var head = document.createElement("div");
    head.className = "tl-state__head";
    var title = document.createElement("div");
    title.className = "tl-state__title";
    title.textContent = "파이프라인 상태";
    var sub = document.createElement("div");
    sub.className = "tl-state__sub";
    head.appendChild(title);
    head.appendChild(sub);
    panel.appendChild(head);

    var legend = document.createElement("div");
    legend.className = "tl-state__legend";
    legend.innerHTML =
      '<span><i style="background:var(--k-state-soft);border:1px solid var(--k-state)"></i>이 단계에서 바인딩 변경</span>' +
      '<span><i style="background:var(--k-upload-soft);border:1px solid var(--k-upload)"></i>내용 갱신 · 기록</span>';
    panel.appendChild(legend);

    var table = document.createElement("table");
    var tbody = document.createElement("tbody");
    var rows = {};
    var lastStage = null;

    SLOTS.forEach(function (slot) {
      if (slot.stage !== lastStage) {
        var stageRow = document.createElement("tr");
        stageRow.className = "st-stage";
        var th = document.createElement("th");
        th.colSpan = 2;
        th.textContent = slot.stage;
        stageRow.appendChild(th);
        tbody.appendChild(stageRow);
        lastStage = slot.stage;
      }
      var tr = document.createElement("tr");
      var tdLabel = document.createElement("td");
      tdLabel.textContent = slot.label;
      var tdValue = document.createElement("td");
      tr.appendChild(tdLabel);
      tr.appendChild(tdValue);
      tbody.appendChild(tr);
      rows[slot.id] = { tr: tr, value: tdValue };
    });

    table.appendChild(tbody);
    panel.appendChild(table);

    var note = document.createElement("div");
    note.className = "tl-state__note";
    panel.appendChild(note);

    // ---------- 상태 계산과 표시 ----------
    var order = steps.map(function (li) {
      return li.id;
    });

    function render(activeId) {
      var state = {};
      Object.keys(INITIAL).forEach(function (k) {
        state[k] = INITIAL[k];
      });

      var activeIndex = order.indexOf(activeId);
      for (var i = 0; i <= activeIndex; i++) {
        var change = CHANGES[order[i]];
        if (change && change.set) {
          Object.keys(change.set).forEach(function (k) {
            state[k] = change.set[k];
          });
        }
      }

      var current = CHANGES[activeId] || {};
      var setKeys = current.set ? Object.keys(current.set) : [];
      var touchKeys = current.touch || [];

      SLOTS.forEach(function (slot) {
        var row = rows[slot.id];
        row.value.textContent = state[slot.id];
        row.tr.classList.toggle("is-set", setKeys.indexOf(slot.id) >= 0);
        row.tr.classList.toggle("is-touch", touchKeys.indexOf(slot.id) >= 0 && setKeys.indexOf(slot.id) < 0);
        row.tr.classList.toggle("is-empty", isEmpty(state[slot.id]));
      });

      if (activeIndex < 0) {
        sub.textContent = "프레임 시작 시 — 직전 프레임에서 물려받은 상태";
        note.textContent =
          "직전 프레임의 톤 매핑 패스가 건 값에, ImGui가 백업 · 복원한 값을 더한 것이다. 첫 프레임은 Initialize가 건 RS 상태와 뷰포트 외에는 비어 있다. OM은 08단계가 다시 걸므로 Present 뒤의 바인딩 상태는 결과에 영향이 없다.";
      } else {
        var heading = document.querySelector("#" + activeId + " h3");
        sub.textContent = activeId.replace("s", "") + " " + (heading ? heading.textContent : "") + " 이후";
        note.textContent = current.note || (setKeys.length ? "" : "이 단계는 파이프라인 바인딩을 바꾸지 않는다.");
      }

      steps.forEach(function (li) {
        li.classList.toggle("is-active", li.id === activeId);
      });
    }

    render(null);

    // ---------- 스크롤에 따라 현재 단계 갱신 ----------
    if ("IntersectionObserver" in window) {
      var observer = new IntersectionObserver(
        function (entries) {
          entries.forEach(function (entry) {
            if (entry.isIntersecting) {
              render(entry.target.id);
            }
          });
        },
        { rootMargin: "-35% 0px -60% 0px" }
      );
      steps.forEach(function (li) {
        observer.observe(li);
      });
    }

    steps.forEach(function (li) {
      var num = li.querySelector(".tl-num");
      if (num) {
        num.addEventListener("click", function () {
          render(li.id);
        });
      }
    });

    // ---------- 필터 ----------
    var GPU_KINDS = ["state", "upload", "clear", "draw", "present"];
    var filterButtons = Array.prototype.slice.call(document.querySelectorAll("[data-filter]"));

    filterButtons.forEach(function (button) {
      button.addEventListener("click", function () {
        var filter = button.getAttribute("data-filter");
        filterButtons.forEach(function (b) {
          b.setAttribute("aria-pressed", b === button ? "true" : "false");
        });
        steps.forEach(function (li) {
          var kind = li.getAttribute("data-kind");
          var show =
            filter === "all" ||
            (filter === "gpu" && GPU_KINDS.indexOf(kind) >= 0) ||
            (filter === "cpu" && kind === "cpu");
          li.hidden = !show;
        });
      });
    });
  });
})();
