/* MyPF Study Site — 공통 스크립트
   file:// 로 열어도 동작해야 하므로 ES 모듈과 fetch를 쓰지 않는다.
   페이지 목록은 PAGES 한 곳에만 둔다. 헤더·학습 경로·이전/다음은 여기서 만든다. */
(function () {
  "use strict";

  var PAGES = [
    { id: "architecture", href: "architecture.html", title: "Architecture", step: "1" },
    { id: "frame", href: "frame.html", title: "Frame Analysis", step: "2" },
    { id: "pipeline", href: "pipeline.html", title: "Rendering Pipeline", step: "3" }
  ];

  var THEME_KEY = "mypf-study-theme";

  // ---------- 테마: DOM을 기다리지 않고 바로 적용해 깜빡임을 줄인다 ----------

  function readTheme() {
    try {
      return window.localStorage.getItem(THEME_KEY);
    } catch (e) {
      return null;
    }
  }

  function writeTheme(value) {
    try {
      window.localStorage.setItem(THEME_KEY, value);
    } catch (e) {
      /* 저장 불가 환경에서는 이번 세션만 유지 */
    }
  }

  function applyTheme(value) {
    if (value === "light" || value === "dark") {
      document.documentElement.setAttribute("data-theme", value);
    } else {
      document.documentElement.removeAttribute("data-theme");
    }
  }

  function effectiveTheme() {
    var set = document.documentElement.getAttribute("data-theme");
    if (set) {
      return set;
    }
    return window.matchMedia && window.matchMedia("(prefers-color-scheme: dark)").matches ? "dark" : "light";
  }

  applyTheme(readTheme());

  // ---------- DOM 헬퍼 ----------

  function el(tag, attrs, children) {
    var node = document.createElement(tag);
    if (attrs) {
      Object.keys(attrs).forEach(function (key) {
        if (key === "text") {
          node.textContent = attrs[key];
        } else if (key === "className") {
          node.className = attrs[key];
        } else {
          node.setAttribute(key, attrs[key]);
        }
      });
    }
    (children || []).forEach(function (child) {
      node.appendChild(child);
    });
    return node;
  }

  function currentIndex(pageId) {
    for (var i = 0; i < PAGES.length; i++) {
      if (PAGES[i].id === pageId) {
        return i;
      }
    }
    return -1;
  }

  // ---------- 헤더 ----------

  function renderHeader(pageId) {
    var mount = document.getElementById("site-header");
    if (!mount) {
      return;
    }

    var items = PAGES.map(function (page) {
      var link = el("a", { href: page.href }, [
        el("span", { className: "nav-step", text: page.step }),
        document.createTextNode(page.title)
      ]);
      if (page.id === pageId) {
        link.setAttribute("aria-current", "page");
      }
      return el("li", null, [link]);
    });

    var toggle = el("button", { type: "button", className: "theme-toggle" });
    function syncLabel() {
      toggle.textContent = effectiveTheme() === "dark" ? "라이트" : "다크";
      toggle.setAttribute("aria-label", toggle.textContent + " 테마로 전환");
    }
    toggle.addEventListener("click", function () {
      var next = effectiveTheme() === "dark" ? "light" : "dark";
      applyTheme(next);
      writeTheme(next);
      syncLabel();
    });
    syncLabel();

    var brand = el("a", { href: "index.html", className: "brand" }, [
      document.createTextNode("MyPF"),
      el("span", { className: "brand__long", text: " Study" })
    ]);

    mount.className = "site-header";
    mount.appendChild(
      el("div", { className: "site-header__inner" }, [
        brand,
        el("nav", { className: "site-nav", "aria-label": "페이지" }, [el("ul", null, items)]),
        toggle
      ])
    );
  }

  // ---------- 학습 경로 띠 ----------

  function renderPath(pageId) {
    var mount = document.getElementById("path");
    if (!mount) {
      return;
    }

    PAGES.forEach(function (page) {
      var node;
      if (page.id === pageId) {
        node = el("span", { "aria-current": "step", text: page.step + ". " + page.title });
      } else {
        node = el("a", { href: page.href, text: page.step + ". " + page.title });
      }
      mount.appendChild(el("li", null, [node]));
    });
  }

  // ---------- 이전 / 다음 ----------

  function renderPager(pageId) {
    var mount = document.getElementById("pager");
    var index = currentIndex(pageId);
    if (!mount || index < 0) {
      return;
    }

    var prev = index > 0 ? PAGES[index - 1] : { href: "index.html", title: "학습 경로", step: "" };
    var next = index < PAGES.length - 1 ? PAGES[index + 1] : null;

    mount.appendChild(
      el("a", { href: prev.href, className: "pager__prev" }, [
        el("small", { text: "← 이전" }),
        el("span", { text: (prev.step ? prev.step + ". " : "") + prev.title })
      ])
    );

    if (next) {
      mount.appendChild(
        el("a", { href: next.href, className: "pager__next" }, [
          el("small", { text: "다음 →" }),
          el("span", { text: next.step + ". " + next.title })
        ])
      );
    }
  }

  // ---------- 페이지 내 목차 + 현재 위치 표시 ----------

  function renderToc() {
    var mount = document.getElementById("toc");
    if (!mount) {
      return;
    }

    var sections = Array.prototype.slice.call(document.querySelectorAll(".content > section[id]"));
    if (sections.length === 0) {
      return;
    }

    var links = {};
    var list = el("ol");

    sections.forEach(function (section) {
      var heading = section.querySelector("h2");
      if (!heading) {
        return;
      }
      var link = el("a", { href: "#" + section.id, text: heading.textContent });
      links[section.id] = link;
      list.appendChild(el("li", null, [link]));
    });

    var details = el("details", { open: "" }, [el("summary", { text: "이 페이지 목차" }), list]);
    mount.appendChild(el("div", { className: "toc__title", text: "이 페이지" }));
    mount.appendChild(details);

    // 좁은 화면에서는 목차를 접어 둔다. 넓은 화면은 접기 버튼이 숨겨지므로 항상 펼친다
    if (window.matchMedia) {
      var narrow = window.matchMedia("(max-width: 900px)");
      details.open = !narrow.matches;
      var onChange = function (event) {
        details.open = !event.matches;
      };
      if (narrow.addEventListener) {
        narrow.addEventListener("change", onChange);
      } else if (narrow.addListener) {
        narrow.addListener(onChange);
      }
    }

    if (!("IntersectionObserver" in window)) {
      return;
    }

    var active = null;
    var observer = new IntersectionObserver(
      function (entries) {
        entries.forEach(function (entry) {
          if (!entry.isIntersecting) {
            return;
          }
          if (active) {
            active.classList.remove("is-active");
          }
          active = links[entry.target.id];
          if (active) {
            active.classList.add("is-active");
          }
        });
      },
      { rootMargin: "-20% 0px -70% 0px" }
    );

    sections.forEach(function (section) {
      observer.observe(section);
    });
  }

  // ---------- 학습 질문: 정답 모두 펼치기 / 접기 ----------

  function bindAnswerToggles() {
    var buttons = document.querySelectorAll("[data-action='toggle-answers']");
    Array.prototype.forEach.call(buttons, function (button) {
      var scope = document.getElementById(button.getAttribute("data-target"));
      if (!scope) {
        return;
      }
      button.addEventListener("click", function () {
        var all = scope.querySelectorAll(".question details");
        var anyClosed = Array.prototype.some.call(all, function (d) {
          return !d.open;
        });
        Array.prototype.forEach.call(all, function (d) {
          d.open = anyClosed;
        });
        button.textContent = anyClosed ? "정답 모두 접기" : "정답 모두 펼치기";
      });
    });
  }

  document.addEventListener("DOMContentLoaded", function () {
    var pageId = document.body.getAttribute("data-page");
    renderHeader(pageId);
    renderPath(pageId);
    renderPager(pageId);
    renderToc();
    bindAnswerToggles();
  });
})();
