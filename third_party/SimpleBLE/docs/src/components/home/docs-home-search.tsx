"use client";
import Link from "next/link";
import { useDocsSearch } from "fumadocs-core/search/client";
import type { ReactElement } from "react";

import { toSearchItems } from "@/lib/utils";



export const DocsHomeSearch = (): ReactElement => {
  const { search, setSearch, query } = useDocsSearch({ type: "fetch" });
  const items = query.data === "empty" ? [] : toSearchItems(query.data);

  return (
    <div className="relative">
      <div className="flex items-center gap-3 rounded-2xl border border-fd-border bg-fd-background/70 px-4 py-3 backdrop-blur">
        <svg
          className="size-5 text-fd-muted-foreground"
          viewBox="0 0 24 24"
          fill="none"
          stroke="currentColor"
          strokeWidth="2"
          aria-hidden="true"
        >
          <path d="M21 21l-4.3-4.3" strokeLinecap="round" />
          <circle cx="11" cy="11" r="7" />
        </svg>

        <input
          value={search}
          onChange={(e) => setSearch(e.currentTarget.value)}
          placeholder="Search the docs…"
          aria-label="Search documentation"
          className="h-10 w-full bg-transparent text-sm sm:text-base text-fd-foreground placeholder:text-fd-muted-foreground outline-none"
        />

        <div className="hidden sm:flex items-center gap-1 rounded-xl border border-fd-border bg-fd-background/80 px-2 py-1 text-xs text-fd-muted-foreground">
          <span>⌘</span>
          <span>K</span>
        </div>
      </div>

      {(query.isLoading || items.length > 0) && search.trim().length > 0 && (
        <div className="absolute left-0 right-0 top-[calc(100%+10px)] z-10 overflow-hidden rounded-2xl border border-fd-border bg-fd-background shadow-xl">
          <div className="max-h-[340px] overflow-auto">
            {query.isLoading && (
              <div className="px-4 py-3 text-sm text-fd-muted-foreground">
                Searching…
              </div>
            )}

            {!query.isLoading && items.length === 0 && (
              <div className="px-4 py-3 text-sm text-fd-muted-foreground">
                No results.
              </div>
            )}

            {!query.isLoading &&
              items.slice(0, 8).map((item) => (
                <Link
                  key={item.url}
                  href={item.url}
                  className="block px-4 py-3 transition-colors hover:bg-fd-muted"
                  onClick={() => setSearch("")}
                >
                  <div className="flex flex-col gap-0.5">
                    <p className="text-sm font-sora font-semibold text-fd-foreground">
                      {item.title}
                    </p>
                    {item.description && (
                      <p className="text-sm leading-relaxed text-fd-muted-foreground line-clamp-2">
                        {item.description}
                      </p>
                    )}
                  </div>
                </Link>
              ))}
          </div>

          <div className="border-t border-fd-border px-4 py-3">
            <Link
              href="/docs"
              className="text-sm font-sora font-semibold text-primary hover:underline decoration-primary underline-offset-4"
              onClick={() => setSearch("")}
            >
              Open full docs →
            </Link>
          </div>
        </div>
      )}
    </div>
  );
};
