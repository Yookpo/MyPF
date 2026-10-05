/* Architecture 페이지 전용 — 시스템 지도 선택과 상세 패널
   관계 데이터는 아래 세 표가 원본이다. 페이지의 의존 매트릭스·시스템 카드와 같은 내용이어야 한다.
   근거는 각 헤더의 멤버 선언과 #include 목록 (architecture.html 시스템 카드 참고). */
(function () {
  "use strict";

  var NAMES = {
    appbase: "AppBase",
    graphicsdevice: "GraphicsDevice",
    grm: "GraphicsResourceManager",
    assetmanager: "AssetManager",
    renderer: "Renderer",
    scene: "Scene",
    gameobject: "GameObject",
    assets: "에셋 객체(Mesh·Material·Texture·Model)",
    components: "컴포넌트(값 멤버)"
  };

  // 소유: 소유자 → 소유 대상. AppBase는 다섯 시스템을 값 멤버로 가진다 (AppBase.h:78-85)
  var OWNS = {
    appbase: ["graphicsdevice", "grm", "assetmanager", "renderer", "scene"],
    assetmanager: ["assets"],
    scene: ["gameobject"],
    gameobject: ["components"]
  };

  // 비소유 참조: 포인터 또는 핸들로 빌려 쓰는 대상
  var REFS = {
    renderer: ["grm", "graphicsdevice", "assets"],
    grm: ["graphicsdevice"],
    assetmanager: ["grm"],
    gameobject: ["assets"],
    assets: ["grm"]
  };

  // 알면 안 되는(실제로 include하지 않는) 시스템
  var NOT_KNOW = {
    appbase: [],
    graphicsdevice: ["grm", "assetmanager", "renderer", "scene", "gameobject", "appbase"],
    grm: ["assetmanager", "renderer", "scene", "gameobject", "appbase"],
    assetmanager: ["renderer", "scene", "gameobject", "graphicsdevice", "appbase"],
    renderer: ["assetmanager", "scene", "gameobject", "appbase"],
    scene: ["graphicsdevice", "grm", "assetmanager", "renderer", "appbase"],
    gameobject: ["scene", "graphicsdevice", "grm", "assetmanager", "renderer", "appbase"]
  };

  function addRole(roles, id, role) {
    if (!roles[id]) {
      roles[id] = [];
    }
    if (roles[id].indexOf(role) < 0) {
      roles[id].push(role);
    }
  }

  function computeRoles(selected) {
    var roles = {};
    addRole(roles, selected, "self");

    (OWNS[selected] || []).forEach(function (id) {
      addRole(roles, id, "owned");
    });
    Object.keys(OWNS).forEach(function (owner) {
      if (OWNS[owner].indexOf(selected) >= 0) {
        addRole(roles, owner, "owner");
      }
    });
    (REFS[selected] || []).forEach(function (id) {
      addRole(roles, id, "ref");
    });
    Object.keys(REFS).forEach(function (source) {
      if (REFS[source].indexOf(selected) >= 0) {
        addRole(roles, source, "refby");
      }
    });
    (NOT_KNOW[selected] || []).forEach(function (id) {
      addRole(roles, id, "nope");
    });

    return roles;
  }

  function collect(roles, role) {
    return Object.keys(roles)
      .filter(function (id) {
        return roles[id].indexOf(role) >= 0;
      })
      .map(function (id) {
        return NAMES[id];
      });
  }

  function roleItem(tagClass, label, names) {
    var li = document.createElement("li");
    var tag = document.createElement("span");
    tag.className = "role-tag " + tagClass;
    tag.textContent = label;
    li.appendChild(tag);
    li.appendChild(document.createTextNode(names.length ? names.join(", ") : "없음"));
    return li;
  }

  document.addEventListener("DOMContentLoaded", function () {
    var svg = document.getElementById("arch-map");
    var panel = document.getElementById("arch-panel");
    if (!svg || !panel) {
      return;
    }

    var chips = Array.prototype.slice.call(document.querySelectorAll(".sys-chip"));
    var nodeLinks = Array.prototype.slice.call(svg.querySelectorAll("a.node"));

    function select(id) {
      if (!NAMES[id] || !NOT_KNOW.hasOwnProperty(id)) {
        return;
      }

      var roles = computeRoles(id);
      svg.classList.add("has-selection");

      Array.prototype.forEach.call(svg.querySelectorAll("[data-node]"), function (el) {
        var r = roles[el.getAttribute("data-node")];
        if (r) {
          el.setAttribute("data-roles", r.join(" "));
        } else {
          el.removeAttribute("data-roles");
        }
      });

      Array.prototype.forEach.call(svg.querySelectorAll(".edge-group"), function (g) {
        var related = g.getAttribute("data-from") === id || g.getAttribute("data-to") === id;
        g.classList.toggle("is-related", related);
      });

      chips.forEach(function (chip) {
        chip.setAttribute("aria-pressed", chip.getAttribute("data-select") === id ? "true" : "false");
      });

      // 패널: 역할 요약 + 해당 시스템 카드의 6개 항목 복제
      var card = document.getElementById("sys-" + id);
      panel.textContent = "";

      var head = document.createElement("div");
      head.className = "arch-panel__head";
      var name = document.createElement("span");
      name.className = "arch-panel__name";
      name.textContent = NAMES[id];
      var link = document.createElement("a");
      link.href = "#sys-" + id;
      link.textContent = "카드 목록에서 보기 ↓";
      head.appendChild(name);
      head.appendChild(link);
      panel.appendChild(head);

      var list = document.createElement("ul");
      list.className = "arch-panel__roles";
      list.appendChild(roleItem("role-tag--own", "소유자", collect(roles, "owner")));
      list.appendChild(roleItem("role-tag--own", "소유", collect(roles, "owned")));
      list.appendChild(roleItem("role-tag--ref", "참조 →", collect(roles, "ref")));
      list.appendChild(roleItem("role-tag--ref", "← 참조받음", collect(roles, "refby")));
      list.appendChild(roleItem("role-tag--nope", "✕ 모름", collect(roles, "nope")));
      panel.appendChild(list);

      if (card) {
        var fields = card.querySelector(".sys-fields");
        if (fields) {
          panel.appendChild(fields.cloneNode(true));
        }
      }
    }

    nodeLinks.forEach(function (a) {
      a.addEventListener("click", function (event) {
        event.preventDefault();
        select(a.getAttribute("data-node"));
      });
    });

    chips.forEach(function (chip) {
      chip.addEventListener("click", function () {
        select(chip.getAttribute("data-select"));
      });
    });

    var fromHash = (window.location.hash || "").replace("#sys-", "");
    select(NOT_KNOW.hasOwnProperty(fromHash) ? fromHash : "renderer");
  });
})();
