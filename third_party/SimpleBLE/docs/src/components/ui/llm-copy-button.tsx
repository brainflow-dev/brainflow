"use client";
import { useState } from "react";
import { Check, Copy } from "lucide-react";
import { useCopyButton } from "fumadocs-ui/utils/use-copy-button";
import { buttonVariants } from "fumadocs-ui/components/ui/button";

import { HoverEffectWrapper } from "@/components/ui/hover-effect-wrapper";
import { cn } from "@/lib/cn";

const cache = new Map<string, string>();

export function LLMCopyButton({
  /**
   * A URL to fetch the raw Markdown/MDX content of page
   */
  markdownUrl,
}: {
  markdownUrl: string;
}) {
  const [isLoading, setLoading] = useState(false);
  const [checked, onClick] = useCopyButton(async () => {
    const cached = cache.get(markdownUrl);
    if (cached) return navigator.clipboard.writeText(cached);

    setLoading(true);

    try {
      await navigator.clipboard.write([
        new ClipboardItem({
          "text/plain": fetch(markdownUrl).then(async (res) => {
            const content = await res.text();
            cache.set(markdownUrl, content);

            return content;
          }),
        }),
      ]);
    } finally {
      setLoading(false);
    }
  });

  return (
    <HoverEffectWrapper
      variant="button"
      className="inline-block overflow-hidden rounded-lg"
    >
      <button
        disabled={isLoading}
        className={cn(
          buttonVariants({
            color: "secondary",
            size: "sm",
            className:
              "gap-2 [&_svg]:size-3.5 [&_svg]:text-fd-muted-foreground relative",
          }),
        )}
        onClick={onClick}
      >
        <span className="relative z-10 md:z-10 z-[1] flex items-center gap-2">
          {checked ? <Check /> : <Copy />}
          Copy Markdown
        </span>
      </button>
    </HoverEffectWrapper>
  );
}
