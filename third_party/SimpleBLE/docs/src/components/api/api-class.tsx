import type { ReactNode } from "react";
import { Box } from "lucide-react";
import ReactMarkdown from "react-markdown";

interface ApiClassProps {
  name: string;
  brief?: string;
  detailed?: string;
  children?: ReactNode;
}

export function ApiClass({ name, brief, detailed, children }: ApiClassProps) {
  return (
    <div className="not-prose my-8 rounded-xl border border-fd-border bg-linear-to-br from-fd-card to-fd-card/50 shadow-lg overflow-hidden">
      {/* Class Header */}
      <div className="border-b border-fd-border bg-linear-to-r from-fd-primary/10 via-fd-primary/5 to-transparent px-6 py-5">
        <div className="flex items-center gap-3">
          <div className="p-2 rounded-lg bg-fd-primary/10 ring-1 ring-fd-primary/20">
            <Box className="h-5 w-5 text-fd-primary" />
          </div>
          <div className="flex-1">
            <h3 className="text-2xl font-bold text-fd-foreground font-mono tracking-tight">
              {name}
            </h3>
            {brief && (
              <p className="mt-1 text-sm text-fd-muted-foreground font-medium">
                {brief}
              </p>
            )}
          </div>
        </div>
      </div>

      {/* Class Body */}
      <div className="px-6 py-5 space-y-6">
        {detailed && (
          <div className="rounded-lg bg-fd-muted/30 py-3">
            <div className="text-sm prose prose-sm max-w-none leading-relaxed">
              <ReactMarkdown
                components={{
                  a: ({ href, children }) => (
                    <a
                      href={href}
                      className="text-fd-primary hover:underline decoration-fd-primary font-medium"
                    >
                      {children}
                    </a>
                  ),
                  p: ({ children }) => (
                    <p className="mb-2 last:mb-0">{children}</p>
                  ),
                }}
              >
                {detailed}
              </ReactMarkdown>
            </div>
          </div>
        )}

        <div className="space-y-4">{children}</div>
      </div>
    </div>
  );
}
