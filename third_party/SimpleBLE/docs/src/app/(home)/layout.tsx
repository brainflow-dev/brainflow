import { HomeLayout } from "fumadocs-ui/layouts/home";

import { baseOptions } from "@/components/layout/layout.shared";

export default function Layout({ children }: LayoutProps<"/">) {
  return <HomeLayout {...baseOptions()}>{children}</HomeLayout>;
}
