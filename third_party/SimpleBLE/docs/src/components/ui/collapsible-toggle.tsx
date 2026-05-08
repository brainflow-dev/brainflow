import { cn } from "@/lib/cn";

interface CollapsibleToggleProps {
  isOpen: boolean;
  onToggle: () => void;
  labelClosed: string;
  labelOpen: string;
}

export function CollapsibleToggle({
  isOpen,
  onToggle,
  labelClosed,
  labelOpen,
}: CollapsibleToggleProps) {
  return (
    <button
      onClick={onToggle}
      className={cn(
        "flex items-center gap-2 px-3 py-1.5 text-xs font-medium transition-all cursor-pointer group rounded-md border",
        "text-fd-muted-foreground border-fd-border/50 ",
      )}
    >
      <div className="relative w-3 h-3 flex items-center justify-center">
        <span
          className={cn(
            "absolute w-full h-0.5 bg-current rounded-full transition-all duration-500 ease-in-out",
            isOpen ? "rotate-180" : "rotate-0",
          )}
        />
        <span
          className={cn(
            "absolute w-full h-0.5 bg-current rounded-full transition-all duration-500 ease-in-out",
            isOpen ? "rotate-180" : "rotate-90",
          )}
        />
      </div>
      {isOpen ? labelOpen : labelClosed}
    </button>
  );
}
