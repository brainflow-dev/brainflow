import type { BaseLayoutProps } from "fumadocs-ui/layouts/shared";

import { LogoWithTheme } from "@/components/ui/logo-with-theme";

export function baseOptions(): BaseLayoutProps {
  return {
    nav: {
      title: (
        <LogoWithTheme
          width={152}
          height={50}
          className="h-8 md:h-9 w-auto"
          priority
        />
      ),
    },
  };
}

