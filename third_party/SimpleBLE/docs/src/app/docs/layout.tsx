import { DocsLayout } from "fumadocs-ui/layouts/docs";

import { SidebarHoverEffect } from "@/components/ui/sidebar-hover-effect";
import { baseOptions } from "@/components/layout/layout.shared";
import { source } from "@/lib/source";

export default function Layout({ children }: LayoutProps<"/docs">) {
  return (
    <DocsLayout
      tree={source.getPageTree()}
      themeSwitch={{ enabled: true }}
      githubUrl="https://github.com/OpenBluetoothToolbox/SimpleBLE"
      {...baseOptions()}
    >
      <SidebarHoverEffect />
      {children}
    </DocsLayout>
  );
}
