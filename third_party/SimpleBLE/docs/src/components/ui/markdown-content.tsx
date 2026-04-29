import ReactMarkdown from "react-markdown";

import { cn } from "@/lib/cn";

interface MarkdownContentProps {
  content: string;
  className?: string;
}

export function MarkdownContent({
  content,
  className,
}: MarkdownContentProps) {
  return (
    <div className={cn("prose prose-sm max-w-none", className)}>
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
            <p className="leading-relaxed mb-2 last:mb-0">{children}</p>
          ),
        }}
      >
        {content}
      </ReactMarkdown>
    </div>
  );
}
