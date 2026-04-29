"use client";
import { useEffect } from "react";

/**
 * Adds a hover with illumination effect to the sidebar links.
 */
export const SidebarHoverEffect = () => {
  useEffect(() => {
    const addHoverEffect = (element: HTMLElement) => {
      element.style.position = "relative";
      element.style.overflow = "hidden";

      let hoverLayer1 = element.querySelector(
        ".sidebar-hover-effect-1"
      ) as HTMLElement;
      let hoverLayer2 = element.querySelector(
        ".sidebar-hover-effect-2"
      ) as HTMLElement;

      if (!hoverLayer1) {
        hoverLayer1 = document.createElement("div");
        hoverLayer1.className = "sidebar-hover-effect-1";
        hoverLayer1.style.cssText = `
          position: absolute;
          inset: 0;
          pointer-events: none;
          opacity: 0;
          transition: opacity 300ms;
          background: linear-gradient(to right, rgba(33,127,241,0.18), rgba(255,255,255,0.08));
          border-radius: inherit;
        `;
        element.prepend(hoverLayer1);
      }

      if (!hoverLayer2) {
        hoverLayer2 = document.createElement("div");
        hoverLayer2.className = "sidebar-hover-effect-2";
        hoverLayer2.style.cssText = `
          position: absolute;
          inset: 0;
          pointer-events: none;
          opacity: 0;
          transition: opacity 300ms;
          background: rgba(255,255,255,0.08);
          mix-blend-mode: screen;
          border-radius: inherit;
        `;
        element.prepend(hoverLayer2);
      }

      const handleMouseMove = (e: MouseEvent) => {
        const rect = element.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;

        const maskImage = `radial-gradient(140px at ${x}px ${y}px, white, transparent 70%)`;

        hoverLayer1.style.maskImage = maskImage;
        hoverLayer1.style.webkitMaskImage = maskImage;
        hoverLayer2.style.maskImage = maskImage;
        hoverLayer2.style.webkitMaskImage = maskImage;
      };

      const handleMouseEnter = () => {
        hoverLayer1.style.opacity = "1";
        hoverLayer2.style.opacity = "1";
      };

      const handleMouseLeave = () => {
        hoverLayer1.style.opacity = "0";
        hoverLayer2.style.opacity = "0";
      };

      element.addEventListener("mousemove", handleMouseMove);
      element.addEventListener("mouseenter", handleMouseEnter);
      element.addEventListener("mouseleave", handleMouseLeave);

      return () => {
        element.removeEventListener("mousemove", handleMouseMove);
        element.removeEventListener("mouseenter", handleMouseEnter);
        element.removeEventListener("mouseleave", handleMouseLeave);
      };
    };

    const sidebarLinks = document.querySelectorAll(
      '#nd-sidebar a[href^="/docs"]'
    );

    const cleanupFunctions: Array<() => void> = [];

    sidebarLinks.forEach((link) => {
      const cleanup = addHoverEffect(link as HTMLElement);
      if (cleanup) cleanupFunctions.push(cleanup);
    });

    const observer = new MutationObserver(() => {
      const newLinks = document.querySelectorAll(
        '#nd-sidebar a[href^="/docs"]'
      );
      newLinks.forEach((link) => {
        if (
          !link.querySelector(".sidebar-hover-effect-1") &&
          !link.querySelector(".sidebar-hover-effect-2")
        ) {
          const cleanup = addHoverEffect(link as HTMLElement);
          if (cleanup) cleanupFunctions.push(cleanup);
        }
      });
    });

    const sidebar = document.querySelector("#nd-sidebar");
    if (sidebar) {
      observer.observe(sidebar, {
        childList: true,
        subtree: true,
      });
    }

    return () => {
      cleanupFunctions.forEach((cleanup) => cleanup());
      observer.disconnect();
    };
  }, []);

  return null;
};
