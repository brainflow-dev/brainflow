import type { ReactNode } from "react";
import { Layers } from "lucide-react";

interface ApiSectionProps {
  title: string;
  id?: string;
  children: ReactNode;
}

export function ApiSection({ title, id, children }: ApiSectionProps) {
  return (
    <section className="my-12 first:mt-8" id={id}>
      <div className="flex items-center gap-3 mb-8 pb-3 border-b-2 border-fd-border">
        <div className="p-2 rounded-lg bg-fd-primary/10 ring-1 ring-fd-primary/20 flex items-center justify-center">
          <Layers className="h-5 w-5 text-fd-primary" />
        </div>
        <h2 className="text-3xl font-bold text-fd-foreground !m-0">{title}</h2>
      </div>
      <div className="space-y-8">{children}</div>
    </section>
  );
}
