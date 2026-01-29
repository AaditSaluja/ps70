const animated = document.querySelectorAll("[data-animate]");

const observer = new IntersectionObserver(
  (entries) => {
    entries.forEach((entry) => {
      if (entry.isIntersecting) {
        entry.target.classList.add("in-view");
        observer.unobserve(entry.target);
      }
    });
  },
  { threshold: 0.15 }
);

animated.forEach((el) => observer.observe(el));

const buttons = document.querySelectorAll(".btn");

buttons.forEach((button) => {
  button.addEventListener("click", (event) => {
    const ripple = document.createElement("span");
    const diameter = Math.max(button.clientWidth, button.clientHeight);
    const rect = button.getBoundingClientRect();
    ripple.classList.add("ripple");
    ripple.style.width = ripple.style.height = `${diameter}px`;
    ripple.style.left = `${event.clientX - rect.left - diameter / 2}px`;
    ripple.style.top = `${event.clientY - rect.top - diameter / 2}px`;
    const existing = button.querySelector(".ripple");
    if (existing) {
      existing.remove();
    }
    button.appendChild(ripple);
  });
});

document.querySelectorAll("[data-scroll]").forEach((button) => {
  button.addEventListener("click", () => {
    const target = button.getAttribute("data-scroll");
    if (!target) return;
    const el = document.querySelector(target);
    if (el) {
      el.scrollIntoView({ behavior: "smooth" });
    }
  });
});
