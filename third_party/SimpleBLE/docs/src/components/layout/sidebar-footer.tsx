"use client";

import Link from "next/link";
import { useTheme } from "next-themes";
import { Moon, Sun, Github } from "lucide-react";
import type { ReactElement } from "react";

export const SidebarFooter = (): ReactElement => {
  const { theme, setTheme } = useTheme();

  return (
    <div className="flex items-center justify-between gap-2 px-4 py-3 border-t border-fd-border">
      <Link
        href="https://github.com/OpenBluetoothToolbox/SimpleBLE"
        target="_blank"
        rel="noopener noreferrer"
        className="flex items-center justify-center size-8 rounded-lg text-fd-muted-foreground hover:bg-fd-accent hover:text-fd-accent-foreground transition-colors"
        aria-label="GitHub repository"
      >
        <Github className="size-4" />
      </Link>

      <button
        onClick={() => setTheme(theme === "dark" ? "light" : "dark")}
        className="flex items-center justify-center size-8 rounded-lg text-fd-muted-foreground hover:bg-fd-accent hover:text-fd-accent-foreground transition-colors"
        aria-label="Toggle theme"
      >
        <Sun className="size-4 rotate-0 scale-100 transition-all dark:-rotate-90 dark:scale-0" />
        <Moon className="absolute size-4 rotate-90 scale-0 transition-all dark:rotate-0 dark:scale-100" />
      </button>
    </div>
  );
};
