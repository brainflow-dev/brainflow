"use client";
import Image from "next/image";
import type { ReactElement } from "react";
import { motion } from "framer-motion";

export const HeroBackground = (): ReactElement => {
  return (
    <div className="pointer-events-none absolute inset-0">
      <div
        className="absolute inset-0 opacity-[0.9]"
        style={{
          background:
            "radial-gradient(900px 500px at 20% 10%, rgba(33,127,241,0.35) 0%, rgba(33,127,241,0) 65%), radial-gradient(700px 420px at 85% 0%, rgba(33,127,241,0.28) 0%, rgba(33,127,241,0) 70%)",
        }}
      />

      <div
        className="absolute inset-0 opacity-[0.35] dark:opacity-[0.25]"
        style={{
          backgroundImage:
            "linear-gradient(to right, rgba(23,23,23,0.06) 1px, transparent 1px), linear-gradient(to bottom, rgba(23,23,23,0.06) 1px, transparent 1px)",
          backgroundSize: "48px 48px",
        }}
      />

      <div className="absolute inset-0 mix-blend-soft-light opacity-[0.40] lg:opacity-[0.35] dark:opacity-[0.30] dark:lg:opacity-[0.25] overflow-hidden">
        <motion.div
          className="absolute"
          style={{
            top: "-5%",
            left: "-5%",
            width: "110%",
            height: "110%",
          }}
          initial={{
            x: "0%",
            y: "0%",
          }}
          animate={{
            x: ["-2%", "2%", "-2%"],
            y: ["-1.5%", "1.5%", "-1.5%"],
          }}
          transition={{
            duration: 50,
            repeat: Infinity,
            ease: [0.25, 0.1, 0.25, 1],
            times: [0, 0.5, 1],
          }}
        >
          <Image
            src="/footer-bg.png"
            alt=""
            fill
            className="object-cover"
            style={{
              objectPosition: "left top",
            }}
            priority
          />
        </motion.div>
      </div>

      <div
        className="absolute -bottom-12 left-0 right-0 h-40 opacity-80 dark:opacity-90"
        style={{
          background:
            "linear-gradient(to top, rgba(33,127,241,0.20) 0%, rgba(33,127,241,0) 85%)",
        }}
      />
    </div>
  );
};
