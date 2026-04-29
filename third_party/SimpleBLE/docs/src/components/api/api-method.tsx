"use client";
import { useState } from "react";
import { Callout } from "fumadocs-ui/components/callout";

import { CollapsibleToggle } from "@/components/ui/collapsible-toggle";
import { InlineCode } from "@/components/ui/inline-code";
import { MarkdownContent } from "@/components/ui/markdown-content";
import { cn } from "@/lib/cn";

interface ApiMethodProps {
  signature: string;
  brief?: string;
  detailed?: string;
  notes?: Array<{
    content: string;
    type: "info" | "warn" | "error";
  }>;
  parameters?: Array<{
    name: string;
    type: string;
    description?: string;
  }>;
  children?: React.ReactNode;
}

export function ApiMethod({
  signature,
  brief,
  detailed,
  notes,
  parameters,
  children,
}: ApiMethodProps) {
  const [isParamsOpen, setIsParamsOpen] = useState(false);
  const [isDetailsOpen, setIsDetailsOpen] = useState(false);

  const hasNotes = notes && notes.length > 0;
  const hasParameters = parameters && parameters.length > 0;
  const hasDetails = brief || detailed || hasNotes;
  const hasAnyContent = hasDetails || hasParameters || children;

  return (
    <div
      className={cn(
        "my-4 rounded-lg border border-fd-border/40 bg-fd-card/30 transition-all hover:border-fd-border/70 hover:shadow-md hover:bg-fd-card/50",
      )}
    >
      {/* Method Signature */}
      <div
        className={cn(
          "w-full flex items-center gap-3 bg-linear-to-r from-fd-muted/30 to-transparent px-4 py-3 text-left transition-colors",
        )}
      >
        <div className="min-w-0 flex-1 py-0.5">
          <InlineCode code={signature} />
        </div>
      </div>

      {/* Method Details */}
      {hasAnyContent && (
        <div className="px-4 pb-4 space-y-4">
          {/* Brief and Detailed descriptions - Collapsible */}
          {hasDetails && (
            <div className="space-y-4">
              <CollapsibleToggle
                isOpen={isDetailsOpen}
                onToggle={() => setIsDetailsOpen(!isDetailsOpen)}
                labelClosed="Show details"
                labelOpen="Hide details"
              />

              {isDetailsOpen && (
                <div className="space-y-3 animate-in fade-in slide-in-from-top-1 duration-500 ease-in-out">
                  {brief && (
                    <MarkdownContent
                      content={brief}
                      className="text-sm text-fd-muted-foreground leading-relaxed [&_p]:inline"
                    />
                  )}

                  {detailed && (
                    <MarkdownContent
                      content={detailed}
                      className="text-sm text-fd-muted-foreground/90"
                    />
                  )}

                  {notes &&
                    notes.map((note, i) => (
                      <Callout key={i} type={note.type} title="Note">
                        <MarkdownContent
                          content={note.content}
                          className="text-sm"
                        />
                      </Callout>
                    ))}

                  {children && <div>{children}</div>}
                </div>
              )}
            </div>
          )}

          {/* Parameters - Collapsible */}
          {hasParameters && (
            <div className="space-y-4">
              <CollapsibleToggle
                isOpen={isParamsOpen}
                onToggle={() => setIsParamsOpen(!isParamsOpen)}
                labelClosed="Show parameters"
                labelOpen="Hide parameters"
              />

              {isParamsOpen && (
                <div className="animate-in fade-in slide-in-from-top-1 duration-700 ease-in-out">
                  <h5 className="text-xs font-semibold text-fd-muted-foreground uppercase tracking-wider mb-3">
                    Parameters
                  </h5>
                  <div className="space-y-2.5">
                    {parameters.map((param, i) => (
                      <div
                        key={i}
                        className="flex items-center gap-2 pl-3 border-l-2 border-fd-primary/20"
                      >
                        <code className="text-xs font-mono text-fd-primary bg-fd-primary/5 px-2 py-1 rounded border border-fd-primary/10">
                          {param.name}
                        </code>
                        <div className="text-xs text-fd-muted-foreground bg-fd-muted/30 px-2 py-1 rounded break-all flex-1">
                          <InlineCode
                            code={param.type}
                            className="[&_code]:text-xs!"
                          />
                        </div>
                        {param.description && (
                          <span className="text-xs text-fd-muted-foreground/80 italic">
                            {param.description}
                          </span>
                        )}
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </div>
          )}
        </div>
      )}
    </div>
  );
}
