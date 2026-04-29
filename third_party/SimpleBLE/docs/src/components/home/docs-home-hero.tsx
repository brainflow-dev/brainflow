"use client";
import Link from "next/link";
import type { ReactElement } from "react";

import { HoverEffectWrapper } from "@/components/ui/hover-effect-wrapper";
import { HeroBackground } from "@/components/ui/hero-background";


export const DocsHomeHero = (): ReactElement => {
  return (
    <HoverEffectWrapper
      variant="hero"
      radius={280}
      maskTransparency={72}
      className="overflow-hidden rounded-[28px] border border-fd-border bg-fd-background"
    >
      <HeroBackground />

      <div className="relative px-5 sm:px-8 lg:px-10 py-10 sm:py-14 lg:py-16">
        <div className="flex flex-col gap-8 sm:gap-10">
          <div className="flex flex-col gap-4">
            <p className="font-sora text-xs sm:text-sm tracking-[0.16em] uppercase text-fd-muted-foreground">
              SimpleBLE Docs
            </p>

            <h1 className="font-sora text-[30px] sm:text-[38px] lg:text-[48px] leading-[1.06] tracking-[-0.035em] text-fd-foreground">
              Bluetooth integration in minutes, not months.
            </h1>

            <p className="text-base sm:text-lg leading-relaxed text-fd-muted-foreground max-w-[70ch]">
              Learn the core API, pick the right bindings, and follow practical
              guides for building reliable Bluetooth experiences across
              platforms.
            </p>
          </div>

          <div className="flex flex-col sm:flex-row gap-3 sm:items-center">
            <Link
              href="/docs/simpleble/tutorial"
              className="inline-flex h-11 items-center justify-center rounded-2xl bg-primary px-5 text-sm font-sora font-semibold text-primary-foreground transition-colors hover:bg-primary/90"
            >
              Quickstart
            </Link>
            <Link
              href="/docs/simpleble/api"
              className="inline-flex h-11 items-center justify-center rounded-2xl border border-fd-border bg-fd-background/70 px-5 text-sm font-sora font-semibold text-fd-foreground backdrop-blur transition-colors hover:bg-fd-muted"
            >
              Explore API
            </Link>
            <Link
              href="/docs"
              className="inline-flex h-11 items-center justify-center rounded-2xl px-4 text-sm font-sora font-semibold text-primary transition-colors hover:bg-primary/10"
            >
              Browse all docs →
            </Link>
          </div>
        </div>
      </div>
    </HoverEffectWrapper>
  );
};
