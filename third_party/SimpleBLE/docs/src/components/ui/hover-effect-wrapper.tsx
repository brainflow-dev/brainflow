"use client";
import type { ReactNode } from "react";
import { motion, useMotionTemplate, useMotionValue } from "framer-motion";

type HoverEffectWrapperProps = {
  children: ReactNode;
  className?: string;
  radius?: number;
  variant?: "default" | "button" | "hero";
  maskTransparency?: number; // Percentage for mask transparency (default: 70)
};

const variants = {
  default: {
    gradient:
      "linear-gradient(to right, rgba(33,127,241,0.28), rgba(255,255,255,0.08))",
    sheen: "rgba(255,255,255,0.08)",
  },
  button: {
    gradient:
      "linear-gradient(to right, rgba(33,127,241,0.48), rgba(33,127,241,0.30))",
    sheen: "rgba(255,255,255,0.15)",
  },
  hero: {
    gradient:
      "linear-gradient(to right, rgba(33,127,241,0.28), rgba(255,255,255,0.08))",
    sheen: "rgba(255,255,255,0.10)",
  },
};

export const HoverEffectWrapper = ({
  children,
  className = "",
  radius = 120,
  variant = "default",
  maskTransparency = 70,
}: HoverEffectWrapperProps) => {
  const mouseX = useMotionValue(0);
  const mouseY = useMotionValue(0);

  const maskImage = useMotionTemplate`radial-gradient(${radius}px at ${mouseX}px ${mouseY}px, white, transparent ${maskTransparency}%)`;
  const maskStyle = { maskImage, WebkitMaskImage: maskImage } as const;

  const colors = variants[variant];

  return (
    <div
      className={`group relative ${className}`}
      onMouseMove={(e) => {
        const rect = e.currentTarget.getBoundingClientRect();
        mouseX.set(e.clientX - rect.left);
        mouseY.set(e.clientY - rect.top);
      }}
    >
      <motion.div
        aria-hidden="true"
        className="pointer-events-none absolute inset-0 rounded-[inherit] opacity-0 transition-opacity duration-300 group-hover:opacity-100"
        style={{
          background: colors.gradient,
          ...maskStyle,
        }}
      />

      <motion.div
        aria-hidden="true"
        className="pointer-events-none absolute inset-0 rounded-[inherit] opacity-0 mix-blend-screen transition-opacity duration-300 group-hover:opacity-100"
        style={{
          background: colors.sheen,
          ...maskStyle,
        }}
      />

      {children}
    </div>
  );
};
