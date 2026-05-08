"use client";
import Link from "next/link";
import type { ReactNode } from "react";
import type { ReactElement } from "react";
import { motion, useMotionTemplate, useMotionValue } from "framer-motion";

type HoverIlluminationLinkProps = Readonly<{
  href: string;
  className: string;
  children: ReactNode;
  radius?: number;
}>;

export const HoverIlluminationLink = ({
  href,
  className,
  children,
  radius = 170,
}: HoverIlluminationLinkProps): ReactElement => {
  const mouseX = useMotionValue(0);
  const mouseY = useMotionValue(0);

  const maskImage = useMotionTemplate`radial-gradient(${radius}px at ${mouseX}px ${mouseY}px, white, transparent 70%)`;
  const maskStyle = { maskImage, WebkitMaskImage: maskImage } as const;

  return (
    <Link
      href={href}
      className={className}
      onMouseMove={(e) => {
        const rect = e.currentTarget.getBoundingClientRect();
        mouseX.set(e.clientX - rect.left);
        mouseY.set(e.clientY - rect.top);
      }}
    >
      <motion.div
        aria-hidden="true"
        className="pointer-events-none absolute inset-0 opacity-0 transition-opacity duration-300 group-hover:opacity-100"
        style={{
          background:
            "linear-gradient(to right, rgba(33,127,241,0.2), rgba(255,255,255,0.09))",
          ...maskStyle,
        }}
      />
      <motion.div
        aria-hidden="true"
        className="pointer-events-none absolute inset-0 opacity-0 mix-blend-screen transition-opacity duration-300 group-hover:opacity-100"
        style={{
          background: "rgba(255,255,255,0.08)",
          ...maskStyle,
        }}
      />

      <div className="relative">{children}</div>
    </Link>
  );
};
