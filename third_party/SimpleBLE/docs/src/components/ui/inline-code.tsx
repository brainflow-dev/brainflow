import { DynamicCodeBlock } from "fumadocs-ui/components/dynamic-codeblock";

import { cn } from "@/lib/cn";

interface InlineCodeProps {
  code: string;
  className?: string;
}

export function InlineCode({ code, className }: InlineCodeProps) {
  return (
    <div
      className={cn(
        "min-w-0 flex-1 [&_pre]:m-0! [&_pre]:p-0! [&_pre]:bg-transparent! [&_code]:text-sm! [&_code]:font-mono! [&_pre]:whitespace-pre-wrap! [&_pre]:wrap-break-words! [&_code]:whitespace-pre-wrap! [&_code]:wrap-break-words! [&_pre]:overflow-visible! [&_code]:overflow-visible!",
        className,
      )}
    >
      <DynamicCodeBlock
        lang="cpp"
        code={code}
        codeblock={{
          allowCopy: false,
        }}
      />
    </div>
  );
}
