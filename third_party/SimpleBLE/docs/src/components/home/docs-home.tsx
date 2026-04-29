"use client";

import Link from "next/link";
import { useState, type ReactElement } from "react";

import { DocsHomeHero } from "@/components/home/docs-home-hero";
import { DocsHomeSearch } from "@/components/home/docs-home-search";
import { HoverIlluminationLink } from "@/components/ui/hover-illumination-link";

type CardLink = Readonly<{
  title: string;
  description: string;
  href: string;
  eyebrow?: string;
}>;

const RESOURCES: ReadonlyArray<CardLink> = [
  {
    title: "Licensing & Support",
    description:
      "Understand the licensing options, commercial readiness, and where to get help when you need it.",
    href: "/docs/licensing",
  },
  {
    title: "Changelog",
    description:
      "Track releases, fixes, and improvements across the core library and language bindings.",
    href: "/docs/changelog",
  },
  {
    title: "Bluetooth LE Basics",
    description: "A brief introduction to Bluetooth Low Energy concepts.",
    href: "/docs/fundamentals/ble_basics",
  },
  {
    title: "BluetoothPermissions",
    description: "Understanding platform-specific permissions for Bluetooth.",
    href: "/docs/fundamentals/permissions",
  },
  {
    title: "Concurrency within SimpleBLE",
    description:
      "Understanding how SimpleBLE handles threading and concurrency.",
    href: "/docs/fundamentals/concurrency",
  },
  {
    title: "CMake Primer",
    description: "A quick primer on CMake for SimpleBLE.",
    href: "/docs/fundamentals/cmake_primer",
  },
];

const PRODUCTS: ReadonlyArray<CardLink> = [
  {
    eyebrow: "C++",
    title: "SimpleBLE",
    description:
      "The core C++ library providing a consistent BLE API across platforms.",
    href: "/docs/simpleble/usage",
  },
  {
    eyebrow: "Python",
    title: "SimplePyBLE",
    description:
      "Python bindings for fast prototyping and automation workflows.",
    href: "/docs/simplepyble/usage",
  },
  {
    eyebrow: "AI / MCP",
    title: "SimpleAIBLE",
    description:
      "AI-friendly BLE toolkit with MCP server, REST API, and agent skills.",
    href: "/docs/simpleaible/mcp",
  },
  {
    eyebrow: "C",
    title: "SimpleCBLE",
    description: "A C-compatible wrapper around the core SimpleBLE library.",
    href: "/docs/simplecble/usage",
  },
  {
    eyebrow: "Java",
    title: "SimpleJavaBLE",
    description:
      "Java bindings for Android and cross-platform Java applications.",
    href: "/docs/simplejavable/examples",
  },
  {
    eyebrow: "Rust",
    title: "SimpleRsBLE",
    description:
      "Rust bindings with a modern API for safe BLE development.",
    href: "/docs/simplersble/examples",
  },
  {
    eyebrow: "Android",
    title: "SimpledroidBLE",
    description:
      "Android-only bindings for native Android BLE development.",
    href: "/docs/simpledroidble/usage",
  },
  {
    eyebrow: "Linux",
    title: "SimpleBluez",
    description:
      "A BlueZ abstraction layer for Linux-centric BLE integrations.",
    href: "/docs/simplebluez/usage",
  },
  {
    eyebrow: "Linux",
    title: "SimpleDBus",
    description: "A DBus object hierarchy handler used by the Linux stack.",
    href: "/docs/simpledbus/usage",
  },
];

type DocsHomeProps = Readonly<{
  className?: string;
}>;

export const DocsHome = ({ className }: DocsHomeProps): ReactElement => {
  const [isExpanded, setIsExpanded] = useState(false);
  const displayedResources = isExpanded ? RESOURCES : RESOURCES.slice(0, 3);

  return (
    <main className={className}>
      <div className="mx-auto w-full max-w-[1100px] px-4 sm:px-6 lg:px-8 pt-8 sm:pt-12 pb-16 sm:pb-20">
        <DocsHomeHero />
        <div className="mt-6 sm:mt-8">
          <DocsHomeSearch />
        </div>

        <section className="mt-12 sm:mt-16">
          <div className="flex items-center justify-between gap-6">
            <h2 className="font-sora text-xl sm:text-2xl tracking-[-0.02em] text-fd-foreground">
              Resources
            </h2>
            {!isExpanded && RESOURCES.length > 3 && (
              <button
                onClick={() => setIsExpanded(true)}
                className="text-sm font-sora font-semibold text-primary hover:underline decoration-primary underline-offset-4"
              >
                View all →
              </button>
            )}
          </div>

          <div className="mt-6 grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
            {displayedResources.map((item) => (
              <HoverIlluminationLink
                key={item.href}
                href={item.href}
                className="group relative overflow-hidden rounded-2xl border border-fd-border bg-fd-background p-5 transition-colors hover:bg-fd-muted/60"
              >
                <div className="flex flex-col gap-3">
                  <p className="font-sora font-semibold text-fd-foreground">
                    {item.title}
                  </p>
                  <p className="text-sm leading-relaxed text-fd-muted-foreground">
                    {item.description}
                  </p>
                  <p className="text-sm font-sora font-semibold text-primary">
                    Read more →
                  </p>
                </div>
              </HoverIlluminationLink>
            ))}
          </div>
        </section>

        <section className="mt-14 sm:mt-20">
          <h2 className="font-sora text-xl sm:text-2xl tracking-[-0.02em] text-fd-foreground">
            Products
          </h2>

          <div className="mt-6 grid gap-4 sm:grid-cols-2 lg:grid-cols-4">
            {PRODUCTS.map((item) => (
              <HoverIlluminationLink
                key={item.href}
                href={item.href}
                className="group relative overflow-hidden rounded-2xl border border-fd-border bg-fd-background p-5 transition-colors hover:bg-fd-muted/60"
                radius={150}
              >
                <div className="flex flex-col gap-4">
                  <div className="flex items-center justify-between gap-3">
                    <p className="text-xs font-sora font-semibold tracking-[0.16em] uppercase text-fd-muted-foreground">
                      {item.eyebrow ?? "Docs"}
                    </p>
                  </div>

                  <div className="flex flex-col gap-2">
                    <p className="font-sora font-semibold text-fd-foreground">
                      {item.title}
                    </p>
                    <p className="text-sm leading-relaxed text-fd-muted-foreground">
                      {item.description}
                    </p>
                  </div>

                  <p className="text-sm font-sora font-semibold text-primary">
                    Open →
                  </p>
                </div>
              </HoverIlluminationLink>
            ))}
          </div>
        </section>
      </div>
    </main>
  );
};
